#pragma once

#include <RmlUi/Core.h>
#include <RmlUi/Core/RenderInterface.h>
#include <d3dx9.h>

// TODO: Update to RenderInterface with shaders instead of textures
class RenderInterfaceDirectX9 final : public Rml::RenderInterface
{
        struct RmlD3D9Vertex
        {
                float x, y, z;
                unsigned long color;
                float u, v;
        };

        struct RmlD3D9CompiledGeometry
        {
                LPDIRECT3DVERTEXBUFFER9 verticies;
                size_t verticiesCount;

                LPDIRECT3DINDEXBUFFER9 indicies;
                size_t primitivesCount;

                LPDIRECT3DTEXTURE9 texture;
        };

        Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
        void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
        void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
        Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
        Rml::TextureHandle GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i sourceDimensions) override;
        void SetScissorRegion(Rml::Rectanglei region) override;

        void EnableScissorRegion(bool enable) override;
        void ReleaseTexture(Rml::TextureHandle texture) override;

        static float GetHorizontalTexelOffset();
        static float GetVerticalTexelOffset();

        void* rmlContext = nullptr;
        LPDIRECT3D9 d3d9 = nullptr;
        LPDIRECT3DDEVICE9 d3d9Device = nullptr;

    public:
        RenderInterfaceDirectX9(LPDIRECT3D9 d3d9, LPDIRECT3DDEVICE9 d3d9Device);
};
