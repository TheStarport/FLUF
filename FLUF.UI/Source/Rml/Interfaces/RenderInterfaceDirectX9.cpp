#include "PCH.hpp"

#include "Rml/Interfaces/RenderInterfaceDirectX9.hpp"

#pragma pack(1)
struct TgaHeader
{
        char idLength;
        char colourMapType;
        char dataType;
        short int colourMapOrigin;
        short int colourMapLength;
        char colourMapDepth;
        short int xOrigin;
        short int yOrigin;
        short int width;
        short int height;
        char bitsPerPixel;
        char imageDescriptor;
};
#pragma pack()

constexpr DWORD vertexFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

void RenderInterfaceDirectX9::EnableScissorRegion(const bool enable) { d3d9Device->SetRenderState(D3DRS_SCISSORTESTENABLE, enable); }

Rml::TextureHandle RenderInterfaceDirectX9::GenerateTexture(const Rml::Span<const unsigned char> source, const Rml::Vector2i sourceDimensions)
{
    LPDIRECT3DTEXTURE9 d3d9Texture;
    if (d3d9Device->CreateTexture(sourceDimensions.x, sourceDimensions.y, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &d3d9Texture, nullptr) != D3D_OK)
    {
        return reinterpret_cast<Rml::TextureHandle>(nullptr);
    }

    D3DLOCKED_RECT locked;
    d3d9Texture->LockRect(0, &locked, nullptr, 0);
    for (auto y = 0; y < sourceDimensions.y; ++y)
    {
        for (auto x = 0; x < sourceDimensions.x; ++x)
        {
            const auto* sourcePixel = source.data() + sourceDimensions.x * 4 * y + x * 4;
            auto destPixel = static_cast<byte*>(locked.pBits) + locked.Pitch * y + x * 4;

            destPixel[0] = sourcePixel[2];
            destPixel[1] = sourcePixel[1];
            destPixel[2] = sourcePixel[0];
            destPixel[3] = sourcePixel[3];
        }
    }

    d3d9Texture->UnlockRect(0);
    return reinterpret_cast<Rml::TextureHandle>(d3d9Texture);
}

Rml::CompiledGeometryHandle RenderInterfaceDirectX9::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
    auto geometry = new RmlD3D9CompiledGeometry();
    d3d9Device->CreateVertexBuffer(vertices.size() * sizeof(RmlD3D9Vertex), D3DUSAGE_WRITEONLY, vertexFvf, D3DPOOL_DEFAULT, &geometry->verticies, nullptr);
    d3d9Device->CreateIndexBuffer(indices.size() * sizeof(uint), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &geometry->indicies, nullptr);

    RmlD3D9Vertex* d3d9Vertex;
    geometry->verticies->Lock(0, 0, reinterpret_cast<void**>(&d3d9Vertex), 0);

    for (int i = 0; i < vertices.size(); ++i)
    {
        auto& newVertex = d3d9Vertex[i];
        auto& oldVertex = vertices[i];
        newVertex.x = oldVertex.position.x;
        newVertex.y = oldVertex.position.y;
        newVertex.z = 0;

        newVertex.color = D3DCOLOR_RGBA(oldVertex.colour.red, oldVertex.colour.green, oldVertex.colour.blue, oldVertex.colour.alpha);
        newVertex.u = oldVertex.tex_coord[0];
        newVertex.v = oldVertex.tex_coord[1];
    }

    geometry->verticies->Unlock();

    uint* d3d9Index;
    geometry->indicies->Lock(0, 0, reinterpret_cast<void**>(&d3d9Index), 0);
    memcpy(d3d9Index, indices.data(), indices.size() * sizeof(uint));
    geometry->indicies->Unlock();

    geometry->verticiesCount = vertices.size();
    geometry->primitivesCount = indices.size() / 3;

    return reinterpret_cast<Rml::CompiledGeometryHandle>(geometry);
}

void RenderInterfaceDirectX9::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
{
    D3DXMATRIX worldTransform;
    D3DXMatrixTranslation(&worldTransform, translation.x, translation.y, 0);
    d3d9Device->SetTransform(D3DTS_WORLD, &worldTransform);

    auto* d3d9Geometry = reinterpret_cast<RmlD3D9CompiledGeometry*>(geometry);

    d3d9Device->SetFVF(vertexFvf);
    d3d9Device->SetStreamSource(0, d3d9Geometry->verticies, 0, sizeof(RmlD3D9Vertex));
    d3d9Device->SetIndices(d3d9Geometry->indicies);

    d3d9Device->SetTexture(0, reinterpret_cast<LPDIRECT3DTEXTURE9>(texture));
    d3d9Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, d3d9Geometry->verticiesCount, 0, d3d9Geometry->primitivesCount);

    d3d9Geometry->texture = reinterpret_cast<LPDIRECT3DTEXTURE9>(texture);
}

void RenderInterfaceDirectX9::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
    const auto* d3d9Geometry = reinterpret_cast<RmlD3D9CompiledGeometry*>(geometry);

    d3d9Geometry->verticies->Release();
    d3d9Geometry->indicies->Release();
    delete d3d9Geometry;
}

Rml::TextureHandle RenderInterfaceDirectX9::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
    auto* fileInterface = Rml::GetFileInterface();
    auto fileHandle = fileInterface->Open(source);

    if (fileHandle == NULL)
    {
        return false;
    }

    fileInterface->Seek(fileHandle, 0, SEEK_END);
    const size_t bufferSize = fileInterface->Tell(fileHandle);
    fileInterface->Seek(fileHandle, 0, SEEK_SET);

    std::vector buffer{ bufferSize, std::byte() };
    fileInterface->Read(buffer.data(), bufferSize, fileHandle);
    fileInterface->Close(fileHandle);

    TgaHeader header{};
    memcpy(&header, buffer.data(), sizeof(TgaHeader));

    const int colorMode = header.bitsPerPixel / 8;
    const size_t imageSize = header.width * header.height * 4;

    if (header.dataType != 2)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24bit and 32bit uncompressed TGAs are supported.");
        return false;
    }

    if (colorMode < 3)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24bit and 32bit textures are supported.");
        return false;
    }

    const auto* src = buffer.data() + sizeof(TgaHeader);
    std::vector dest{ imageSize, std::byte() };
    auto* destData = dest.data();

    for (auto y = 0; y < header.height; ++y)
    {
        auto readIndex = y * header.width * colorMode;
        auto writeIndex = ((header.imageDescriptor & 32) != 0) ? readIndex : (header.height - y - 1) * header.width * colorMode;
        for (auto x = 0; x < header.width; ++x)
        {
            destData[writeIndex++] = src[readIndex + 2];
            destData[writeIndex++] = src[readIndex + 1];
            destData[writeIndex++] = src[readIndex];
            destData[writeIndex++] = colorMode == 4 ? src[readIndex + 3] : static_cast<std::byte>(255);
            readIndex += colorMode;
        }
    }

    texture_dimensions.x = header.width;
    texture_dimensions.y = header.height;

    return GenerateTexture(Rml::Span(reinterpret_cast<const unsigned char*>(&destData), imageSize), texture_dimensions);
}

void RenderInterfaceDirectX9::SetScissorRegion(const Rml::Rectanglei region)
{
    // clang-format off
    const RECT scissor{
        .left = region.Left(),
        .top = region.Top(),
        .right = region.Left() + region.Width(),
        .bottom = region.Right() + region.Height()
    };
    // clang-format on

    d3d9Device->SetScissorRect(&scissor);
}

void RenderInterfaceDirectX9::ReleaseTexture(const Rml::TextureHandle texture) { reinterpret_cast<LPDIRECT3DTEXTURE9>(texture)->Release(); }
float RenderInterfaceDirectX9::GetHorizontalTexelOffset() { return -0.5f; }
float RenderInterfaceDirectX9::GetVerticalTexelOffset() { return -0.5f; }

// ReSharper disable twice CppParameterMayBeConst
RenderInterfaceDirectX9::RenderInterfaceDirectX9(LPDIRECT3D9 d3d9, LPDIRECT3DDEVICE9 d3d9Device) : d3d9(d3d9), d3d9Device(d3d9Device)
{
    if (!d3d9 || !d3d9Device)
    {
        throw std::invalid_argument("RenderInterfaceDirectX9 constructed with an invalid pointers.");
    }
}
