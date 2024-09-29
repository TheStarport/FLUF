#pragma once

#include "RmlUi/Core/RenderInterfaceCompatibility.h"

#include <RmlUi/Core.h>
#include <d3dx9.h>

// TODO: Update to RenderInterface with shaders instead of textures
class RenderInterfaceDirectX9 final : public Rml::RenderInterfaceCompatibility
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

        void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture,
                            const Rml::Vector2f& translation) override;
        Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
                                                    Rml::TextureHandle texture) override;
        void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) override;
        void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) override;
        void EnableScissorRegion(bool enable) override;
        void SetScissorRegion(int x, int y, int width, int height) override;
        bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
        bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
        void ReleaseTexture(Rml::TextureHandle texture) override;

        static float GetHorizontalTexelOffset();
        static float GetVerticalTexelOffset();

        void* rmlContext = nullptr;
        LPDIRECT3D9 d3d9 = nullptr;
        LPDIRECT3DDEVICE9 d3d9Device = nullptr;

    public:
        RenderInterfaceDirectX9(LPDIRECT3D9 d3d9, LPDIRECT3DDEVICE9 d3d9Device);
};
