#include <PCH.hpp>

#include "imgui.h"
#include "Internal/ImGuiD3D8.hpp"

#include "vendor/DXSDK/include/d3d8.h"

static LPDIRECT3DDEVICE8 d3dDevice = nullptr;
static LPDIRECT3DVERTEXBUFFER8 vertexBuffer = nullptr;
static LPDIRECT3DINDEXBUFFER8 indexBuffer = nullptr;
static LPDIRECT3DVERTEXBUFFER8 maskVertexBuffer = nullptr;
static LPDIRECT3DINDEXBUFFER8 maskIndexBuffer = nullptr;
static LPDIRECT3DTEXTURE8 fontTexture = nullptr;
static int vertexBufferSize = 5000, indexBufferSize = 10000;
static IDirect3DSurface8* depthBuffer = nullptr;
IDirect3DSurface8* realDepthStencilBuffer;

struct CUSTOMVERTEX
{
        float pos[3];
        D3DCOLOR col;
        float uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

static void ImGui_ImplDX8_SetupRenderState(ImDrawData* draw_data)
{
    if (realDepthStencilBuffer)
    {
        realDepthStencilBuffer->Release();
    }

    // Setup viewport
    D3DVIEWPORT8 vp;
    vp.X = vp.Y = 0;
    vp.Width = static_cast<DWORD>(draw_data->DisplaySize.x);
    vp.Height = static_cast<DWORD>(draw_data->DisplaySize.y);
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    d3dDevice->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient)
    d3dDevice->GetDepthStencilSurface(&realDepthStencilBuffer);
    d3dDevice->SetRenderTarget(nullptr, depthBuffer);
    d3dDevice->SetPixelShader(NULL);
    d3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);
    d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    d3dDevice->SetRenderState(D3DRS_LIGHTING, false);
    d3dDevice->SetRenderState(D3DRS_ZENABLE, false);
    d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    //g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD); //new
    d3dDevice->SetRenderState(D3DRS_FOGENABLE, false);            //new
    d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    //g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    //g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Setup orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    // Being agnostic of whether <d3DX8.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        const float L = draw_data->DisplayPos.x + 0.5f;
        const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
        const float T = draw_data->DisplayPos.y + 0.5f;
        const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
        const D3DMATRIX matIdentity = { { { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } } };
        const D3DMATRIX matProjection = {
            { { 2.0f / (R - L), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (T - B), 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, (L + R) / (L - R), (T + B) / (B - T), 0.5f, 1.0f } }
        };
        d3dDevice->SetTransform(D3DTS_WORLD, &matIdentity);
        d3dDevice->SetTransform(D3DTS_VIEW, &matIdentity);
        d3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
    }
}

static void build_mask_vbuffer(const RECT* rect)
{
    CUSTOMVERTEX* vtx_dst;
    maskVertexBuffer->Lock(0, (UINT)(6 * sizeof(CUSTOMVERTEX)), (BYTE**)&vtx_dst, 0);
    vtx_dst[0].pos[0] = static_cast<float>(rect->left);
    vtx_dst[0].pos[1] = static_cast<float>(rect->bottom);
    vtx_dst[0].pos[2] = 0;
    vtx_dst[1].pos[0] = static_cast<float>(rect->left);
    vtx_dst[1].pos[1] = static_cast<float>(rect->top);
    vtx_dst[1].pos[2] = 0;
    vtx_dst[2].pos[0] = static_cast<float>(rect->right);
    vtx_dst[2].pos[1] = static_cast<float>(rect->top);
    vtx_dst[2].pos[2] = 0;
    vtx_dst[3].pos[0] = static_cast<float>(rect->left);
    vtx_dst[3].pos[1] = static_cast<float>(rect->bottom);
    vtx_dst[3].pos[2] = 0;
    vtx_dst[4].pos[0] = static_cast<float>(rect->right);
    vtx_dst[4].pos[1] = static_cast<float>(rect->top);
    vtx_dst[4].pos[2] = 0;
    vtx_dst[5].pos[0] = static_cast<float>(rect->right);
    vtx_dst[5].pos[1] = static_cast<float>(rect->bottom);
    vtx_dst[5].pos[2] = 0;
    vtx_dst[0].col = 0xFFFFFFFF;
    vtx_dst[1].col = 0xFFFFFFFF;
    vtx_dst[2].col = 0xFFFFFFFF;
    vtx_dst[3].col = 0xFFFFFFFF;
    vtx_dst[4].col = 0xFFFFFFFF;
    vtx_dst[5].col = 0xFFFFFFFF;
    vtx_dst[0].uv[0] = 0;
    vtx_dst[0].uv[1] = 0;
    vtx_dst[1].uv[0] = 0;
    vtx_dst[1].uv[1] = 0;
    vtx_dst[2].uv[0] = 0;
    vtx_dst[2].uv[1] = 0;
    vtx_dst[3].uv[0] = 0;
    vtx_dst[3].uv[1] = 0;
    vtx_dst[4].uv[0] = 0;
    vtx_dst[4].uv[1] = 0;
    vtx_dst[5].uv[0] = 0;
    vtx_dst[5].uv[1] = 0;
    maskVertexBuffer->Unlock();
}

void ImGui_ImplDX8_RenderDrawData(ImDrawData* drawData)
{
    try
    {
        // Avoid rendering when minimized
        if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
        {
            return;
        }

        // Create and grow buffers if needed
        if (!vertexBuffer || vertexBufferSize < drawData->TotalVtxCount)
        {
            if (vertexBuffer)
            {
                vertexBuffer->Release();
                vertexBuffer = nullptr;
            }

            vertexBufferSize = drawData->TotalVtxCount + 5000;
            if (d3dDevice->CreateVertexBuffer(
                    vertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vertexBuffer) < 0)
            {
                return;
            }
        }

        if (!indexBuffer || indexBufferSize < drawData->TotalIdxCount)
        {
            if (indexBuffer)
            {
                indexBuffer->Release();
                indexBuffer = nullptr;
            }

            indexBufferSize = drawData->TotalIdxCount + 10000;
            if (d3dDevice->CreateIndexBuffer(indexBufferSize * sizeof(ImDrawIdx),
                                             D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                             sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
                                             D3DPOOL_DEFAULT,
                                             &indexBuffer) < 0)
            {
                return;
            }
        }

        if (!maskVertexBuffer && !maskIndexBuffer)
        {
            if (d3dDevice->CreateVertexBuffer(
                    6 * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &maskVertexBuffer) < 0)
            {
                return;
            }

            if (d3dDevice->CreateIndexBuffer(
                    6, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &maskIndexBuffer) < 0)
            {
                return;
            }

            ImDrawIdx* indexDest;
            maskIndexBuffer->Lock(0, 6 * sizeof(ImDrawIdx), (BYTE**)&indexDest, D3DLOCK_DISCARD);
            indexDest[0] = 0;
            indexDest[1] = 1;
            indexDest[2] = 2;
            indexDest[3] = 0;
            indexDest[4] = 2;
            indexDest[5] = 3;
            maskIndexBuffer->Unlock();
        }

        // Backup the DX8 state
        DWORD stateBlockToken = 0;
        if (d3dDevice->CreateStateBlock(D3DSBT_ALL, &stateBlockToken) < 0)
        {
            return;
        }

        // Backup the DX8 transform (DX8 documentation suggests that it is included in the StateBlock but it doesn't appear to)
        D3DMATRIX last_world, last_view, last_projection;            //new
        d3dDevice->GetTransform(D3DTS_WORLD, &last_world);           //new
        d3dDevice->GetTransform(D3DTS_VIEW, &last_view);             //new
        d3dDevice->GetTransform(D3DTS_PROJECTION, &last_projection); //new

        // Copy and convert all vertices into a single contiguous buffer, convert colors to DX8 default format.
        // FIXME-OPT: This is a waste of resource, the ideal is to use imconfig.h and
        //  1) to avoid repacking colors:   #define IMGUI_USE_BGRA_PACKED_COLOR
        //  2) to avoid repacking vertices: #define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct ImDrawVert { ImVec2 pos; float z; ImU32 col; ImVec2 uv; }
        CUSTOMVERTEX* vtx_dst;
        ImDrawIdx* idx_dst;
        if (vertexBuffer->Lock(0, (UINT)(drawData->TotalVtxCount * sizeof(CUSTOMVERTEX)), (BYTE**)&vtx_dst, D3DLOCK_DISCARD) < 0)
        {
            d3dDevice->DeleteStateBlock(stateBlockToken);
            return;
        }

        if (indexBuffer->Lock(0, (UINT)(drawData->TotalIdxCount * sizeof(ImDrawIdx)), (BYTE**)&idx_dst, D3DLOCK_DISCARD) < 0)
        {
            d3dDevice->DeleteStateBlock(stateBlockToken);
            vertexBuffer->Unlock();
            return;
        }

        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = drawData->CmdLists[n];
            const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
            for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
            {
                vtx_dst->pos[0] = vtx_src->pos.x;
                vtx_dst->pos[1] = vtx_src->pos.y;
                vtx_dst->pos[2] = 0.0f;
                vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) | ((vtx_src->col & 0xFF) << 16); // RGBA --> ARGB for DirectX9
                vtx_dst->uv[0] = vtx_src->uv.x;
                vtx_dst->uv[1] = vtx_src->uv.y;
                vtx_dst++;
                vtx_src++;
            }
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            idx_dst += cmd_list->IdxBuffer.Size;
        }

        vertexBuffer->Unlock();
        indexBuffer->Unlock();
        d3dDevice->SetStreamSource(0, vertexBuffer, sizeof(CUSTOMVERTEX));
        d3dDevice->SetIndices(indexBuffer, 0);
        d3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX); //new

        // Setup desired DX state
        ImGui_ImplDX8_SetupRenderState(drawData);

        // Render command lists
        // (Because we merged all buffers into a single one, we maintain our own offset into them)
        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        const ImVec2 clip_off = drawData->DisplayPos;
        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = drawData->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != nullptr)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        ImGui_ImplDX8_SetupRenderState(drawData);
                    }
                    else
                    {
                        pcmd->UserCallback(cmd_list, pcmd);
                    }
                }
                else
                {
                    const RECT r = { static_cast<LONG>(pcmd->ClipRect.x - clip_off.x),
                                     static_cast<LONG>(pcmd->ClipRect.y - clip_off.y),
                                     static_cast<LONG>(pcmd->ClipRect.z - clip_off.x),
                                     static_cast<LONG>(pcmd->ClipRect.w - clip_off.y) };
                    const LPDIRECT3DTEXTURE8 texture = (LPDIRECT3DTEXTURE8)pcmd->TexRef.GetTexID();
                    d3dDevice->SetTexture(0, texture);
                    build_mask_vbuffer(&r);
                    d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
                    d3dDevice->SetRenderState(D3DRS_ZENABLE, true);
                    d3dDevice->SetRenderState(D3DRS_STENCILENABLE, true);
                    d3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFF);
                    d3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
                    d3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
                    d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
                    d3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
                    d3dDevice->SetRenderState(D3DRS_STENCILREF, 0xFF);
                    d3dDevice->Clear(0, nullptr, D3DCLEAR_STENCIL, 0, 1.0f, 0);
                    d3dDevice->SetStreamSource(0, maskVertexBuffer, sizeof(CUSTOMVERTEX));
                    d3dDevice->SetIndices(maskIndexBuffer, 0);
                    d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2); //new
                    d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
                    d3dDevice->SetStreamSource(0, vertexBuffer, sizeof(CUSTOMVERTEX));
                    d3dDevice->SetIndices(indexBuffer, global_vtx_offset);
                    d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
                    d3dDevice->SetRenderState(D3DRS_ZENABLE, false);
                    d3dDevice->SetRenderState(D3DRS_STENCILENABLE, true);
                    d3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0);
                    d3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
                    d3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
                    d3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
                    d3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
                    d3dDevice->SetRenderState(D3DRS_STENCILREF, 0xFF);
                    //g_pd3dDevice->SetScissorRect(&r);
                    d3dDevice->DrawIndexedPrimitive(
                        D3DPT_TRIANGLELIST, 0, static_cast<UINT>(cmd_list->VtxBuffer.Size), pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount / 3);
                }
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }

        // Restore the DX8 transform
        d3dDevice->SetTransform(D3DTS_WORLD, &last_world);           //new
        d3dDevice->SetTransform(D3DTS_VIEW, &last_view);             //new
        d3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection); //new

        // Restore the DX8 state
        d3dDevice->SetRenderTarget(nullptr, realDepthStencilBuffer);
        d3dDevice->ApplyStateBlock(stateBlockToken);
        d3dDevice->DeleteStateBlock(stateBlockToken);
    }
    catch (...)
    {}
}

bool ImGui_ImplDX8_Init(IDirect3DDevice8* device)
{
    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "fld3dx8";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

    d3dDevice = device;
    d3dDevice->AddRef();
    return true;
}

void ImGui_ImplDX8_Shutdown()
{
    ImGui_ImplDX8_InvalidateDeviceObjects();
    if (d3dDevice)
    {
        d3dDevice->Release();
        d3dDevice = nullptr;
    }
}

static bool ImGui_ImplDX8_CreateFontsTexture()
{
    // Build texture atlas
    const ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height, BYTEs_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &BYTEs_per_pixel);

    // Upload texture to graphics system
    fontTexture = nullptr;
    if (d3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &fontTexture) < 0)
    {
        return false;
    }

    D3DLOCKED_RECT tex_locked_rect;
    if (fontTexture->LockRect(0, &tex_locked_rect, nullptr, 0) != D3D_OK)
    {
        return false;
    }

    for (int y = 0; y < height; y++)
    {
        memcpy(
            static_cast<unsigned char*>(tex_locked_rect.pBits) + tex_locked_rect.Pitch * y, pixels + (width * BYTEs_per_pixel) * y, (width * BYTEs_per_pixel));
    }

    fontTexture->UnlockRect(0);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)fontTexture;

    return true;
}

static bool ImGui_ImplD3D8_CreateDepthStencilBuffer()
{
    if (d3dDevice == nullptr)
    {
        return false;
    }

    if (depthBuffer == nullptr)
    {
        IDirect3DSurface8* realDepth;
        D3DSURFACE_DESC sfcDesc;

        d3dDevice->GetDepthStencilSurface(&realDepth);
        if (realDepth->GetDesc(&sfcDesc))
        {
            realDepth->Release();
            return false;
        }

        if (d3dDevice->CreateDepthStencilSurface(sfcDesc.Width, sfcDesc.Height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, &depthBuffer))
        {
            realDepth->Release();
            return false;
        }

        realDepth->Release();
    }

    return true;
}

bool ImGui_ImplDX8_CreateDeviceObjects()
{
    if (!d3dDevice)
    {
        return false;
    }

    if (!ImGui_ImplDX8_CreateFontsTexture())
    {
        return false;
    }

    if (!ImGui_ImplD3D8_CreateDepthStencilBuffer())
    {
        return false;
    }

    return true;
}

void ImGui_ImplDX8_InvalidateDeviceObjects()
{
    if (!d3dDevice)
    {
        return;
    }

    if (vertexBuffer)
    {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }

    if (indexBuffer)
    {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }

    if (maskVertexBuffer)
    {
        maskVertexBuffer->Release();
        maskVertexBuffer = nullptr;
    }

    if (maskIndexBuffer)
    {
        maskIndexBuffer->Release();
        maskIndexBuffer = nullptr;
    }

    if (depthBuffer)
    {
        depthBuffer->Release();
        depthBuffer = nullptr;
    }

    if (realDepthStencilBuffer)
    {
        realDepthStencilBuffer->Release();
        realDepthStencilBuffer = nullptr;
    }

    if (fontTexture)
    {
        fontTexture->Release();
        fontTexture = nullptr;
        ImGui::GetIO().Fonts->TexID._TexData = nullptr;
        ImGui::GetIO().Fonts->TexID._TexID = 0;
    }
}

void ImGui_ImplDX8_NewFrame()
{
    if (!fontTexture || !depthBuffer)
    {
        ImGui_ImplDX8_CreateDeviceObjects();
    }
}
