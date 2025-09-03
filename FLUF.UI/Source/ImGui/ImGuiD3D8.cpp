#include <PCH.hpp>

#include "imgui.h"
#include "Internal/ImGuiD3D8.hpp"

#include "vendor/DXSDK/include/d3d8.h"

// DirectX data
struct ImGui_ImplDX8_Data
{
        LPDIRECT3DDEVICE8 pd3dDevice;
        LPDIRECT3DVERTEXBUFFER8 pVB;
        LPDIRECT3DINDEXBUFFER8 pIB;
        LPDIRECT3DTEXTURE8 FontTexture;
        int VertexBufferSize;
        int IndexBufferSize;

        ImGui_ImplDX8_Data()
        {
            memset(this, 0, sizeof(*this));
            VertexBufferSize = 5000;
            IndexBufferSize = 10000;
        }
};

struct CUSTOMVERTEX
{
        float pos[3];
        D3DCOLOR col;
        float uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#ifdef IMGUI_USE_BGRA_PACKED_COLOR
    #define IMGUI_COL_TO_DX8_ARGB(_COL) (_COL)
#else
    #define IMGUI_COL_TO_DX8_ARGB(_COL) (((_COL) & 0xFF00FF00) | (((_COL) & 0xFF0000) >> 16) | (((_COL) & 0xFF) << 16))
#endif

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplDX8_Data* ImGui_ImplDX8_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplDX8_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

// Functions
static void ImGui_ImplDX8_SetupRenderState(ImDrawData* draw_data)
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();

    // Setup viewport
    D3DVIEWPORT8 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)draw_data->DisplaySize.x;
    vp.Height = (DWORD)draw_data->DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    bd->pd3dDevice->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient)
    bd->pd3dDevice->SetPixelShader(NULL);
    bd->pd3dDevice->SetVertexShader(NULL);
    bd->pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    bd->pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    bd->pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    bd->pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    bd->pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0 | D3DCLIPPLANE1 | D3DCLIPPLANE2 | D3DCLIPPLANE3);
    bd->pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    // Setup orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    // Being agnostic of whether <d3dx8.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        float L = draw_data->DisplayPos.x + 0.5f;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
        float T = draw_data->DisplayPos.y + 0.5f;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
        D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } } };
        D3DMATRIX mat_projection = {
            { { 2.0f / (R - L), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (T - B), 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, (L + R) / (L - R), (T + B) / (B - T), 0.5f, 1.0f } }
        };
        bd->pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }
}

// Render function.
void ImGui_ImplDX8_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
    {
        return;
    }

    // Create and grow buffers if needed
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (bd->pVB)
        {
            bd->pVB->Release();
            bd->pVB = nullptr;
        }
        bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;
        //if (bd->pd3dDevice->CreateVertexBuffer(bd->VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &bd->pVB, NULL) < 0)
        if (bd->pd3dDevice->CreateVertexBuffer(
                bd->VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &bd->pVB) < 0)
        {
            return;
        }
    }
    if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (bd->pIB)
        {
            bd->pIB->Release();
            bd->pIB = nullptr;
        }
        bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
        //if (bd->pd3dDevice->CreateIndexBuffer(bd->IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &bd->pIB, NULL) < 0)
        if (bd->pd3dDevice->CreateIndexBuffer(bd->IndexBufferSize * sizeof(ImDrawIdx),
                                              D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                              sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
                                              D3DPOOL_DEFAULT,
                                              &bd->pIB) < 0)
        {
            return;
        }
    }

    // Backup the DX8 transform (DX8 documentation suggests that it is included in the StateBlock but it doesn't appear to)
    D3DMATRIX last_world, last_view, last_projection;
    bd->pd3dDevice->GetTransform(D3DTS_WORLD, &last_world);
    bd->pd3dDevice->GetTransform(D3DTS_VIEW, &last_view);
    bd->pd3dDevice->GetTransform(D3DTS_PROJECTION, &last_projection);

    // Setup desired DX state
    ImGui_ImplDX8_SetupRenderState(draw_data);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            ImDrawCmd* next_pcmd = nullptr;

            if (cmd_i < cmd_list->CmdBuffer.Size - 1)
            {
                next_pcmd = const_cast<ImDrawCmd*>(&cmd_list->CmdBuffer[cmd_i + 1]);
            }

            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                {
                    ImGui_ImplDX8_SetupRenderState(draw_data);
                }
                else
                {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
                ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                {
                    continue;
                }

                CUSTOMVERTEX* vtx_dst;
                ImDrawIdx* idx_dst;
                if (bd->pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (BYTE**)&vtx_dst, D3DLOCK_DISCARD) < 0)
                {
                    //d3d9_state_block->Release();
                    return;
                }
                if (bd->pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (BYTE**)&idx_dst, D3DLOCK_DISCARD) < 0)
                {
                    bd->pVB->Unlock();
                    //d3d9_state_block->Release();
                    return;
                }

                //we dont need to get it for vtx cause its always 0 ?!?!?!!?
                UINT curr_draw_cmd_idx_count;

                if (!next_pcmd)
                {
                    curr_draw_cmd_idx_count = UINT(cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size - (cmd_list->IdxBuffer.Data + pcmd->IdxOffset));
                }
                else
                {
                    curr_draw_cmd_idx_count = next_pcmd->IdxOffset;
                }

                const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data + pcmd->VtxOffset;
                for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
                {
                    vtx_dst->pos[0] = vtx_src->pos.x;
                    vtx_dst->pos[1] = vtx_src->pos.y;
                    vtx_dst->pos[2] = 0.0f;
                    vtx_dst->col = IMGUI_COL_TO_DX8_ARGB(vtx_src->col);
                    vtx_dst->uv[0] = vtx_src->uv.x;
                    vtx_dst->uv[1] = vtx_src->uv.y;
                    vtx_dst++;
                    vtx_src++;
                }
                memcpy(idx_dst, cmd_list->IdxBuffer.Data + pcmd->IdxOffset, curr_draw_cmd_idx_count * sizeof(ImDrawIdx));

                bd->pVB->Unlock();
                bd->pIB->Unlock();

                // Apply Scissor/clipping rectangle, Bind texture, Draw
                float leftPlane[4] = { 1.0f, 0.0f, 0.0f, clip_min.x };
                float rightPlane[4] = { -1.0f, 0.0f, 0.0f, clip_max.x };
                float topPlane[4] = { 0.0f, 1.0f, 0.0f, clip_min.y };
                float bottomPlane[4] = { 0.0f, -1.0f, 0.0f, clip_max.y };

                bd->pd3dDevice->SetClipPlane(0, leftPlane);
                bd->pd3dDevice->SetClipPlane(1, rightPlane);
                bd->pd3dDevice->SetClipPlane(2, topPlane);
                bd->pd3dDevice->SetClipPlane(3, bottomPlane);

                // Render the vertex buffer contents
                const LPDIRECT3DTEXTURE8 texture = (LPDIRECT3DTEXTURE8)pcmd->GetTexID();
                bd->pd3dDevice->SetTexture(0, texture);
                bd->pd3dDevice->SetStreamSource(0, bd->pVB, sizeof(CUSTOMVERTEX));
                bd->pd3dDevice->SetIndices(bd->pIB, 0); //
                bd->pd3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);
                bd->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0U, cmd_list->VtxBuffer.Size, 0U, pcmd->ElemCount / 3);
            }
        }
    }

    // Restore the DX8 transform
    bd->pd3dDevice->SetTransform(D3DTS_WORLD, &last_world);
    bd->pd3dDevice->SetTransform(D3DTS_VIEW, &last_view);
    bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection);
}

bool ImGui_ImplDX8_Init(IDirect3DDevice8* device)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    ImGui_ImplDX8_Data* bd = IM_NEW(ImGui_ImplDX8_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_dx8";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

    bd->pd3dDevice = device;
    bd->pd3dDevice->AddRef();

    return true;
}

void ImGui_ImplDX8_Shutdown()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    IM_ASSERT(bd != NULL && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplDX8_InvalidateDeviceObjects();
    if (bd->pd3dDevice)
    {
        bd->pd3dDevice->Release();
    }
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    IM_DELETE(bd);
}

static bool ImGui_ImplDX8_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    unsigned char* pixels;
    int width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    // Convert RGBA32 to BGRA32 (because RGBA32 is not well supported by DX8 devices)
#ifndef IMGUI_USE_BGRA_PACKED_COLOR
    if (io.Fonts->TexPixelsUseColors)
    {
        ImU32* dst_start = (ImU32*)ImGui::MemAlloc((size_t)width * height * bytes_per_pixel);
        for (ImU32 *src = (ImU32*)pixels, *dst = dst_start, *dst_end = dst_start + (size_t)width * height; dst < dst_end; src++, dst++)
        {
            *dst = IMGUI_COL_TO_DX8_ARGB(*src);
        }
        pixels = (unsigned char*)dst_start;
    }
#endif

    // Upload texture to graphics system
    bd->FontTexture = nullptr;
    //if (bd->pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &bd->FontTexture, NULL) < 0)
    if (bd->pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &bd->FontTexture) < 0)
    {
        return false;
    }
    D3DLOCKED_RECT tex_locked_rect;
    if (bd->FontTexture->LockRect(0, &tex_locked_rect, nullptr, 0) != D3D_OK)
    {
        return false;
    }
    for (int y = 0; y < height; y++)
    {
        memcpy((unsigned char*)tex_locked_rect.pBits + (size_t)tex_locked_rect.Pitch * y,
               pixels + (size_t)width * bytes_per_pixel * y,
               (size_t)width * bytes_per_pixel);
    }
    bd->FontTexture->UnlockRect(0);

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)bd->FontTexture);

#ifndef IMGUI_USE_BGRA_PACKED_COLOR
    if (io.Fonts->TexPixelsUseColors)
    {
        ImGui::MemFree(pixels);
    }
#endif

    return true;
}

bool ImGui_ImplDX8_CreateDeviceObjects()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd || !bd->pd3dDevice)
    {
        return false;
    }
    if (!ImGui_ImplDX8_CreateFontsTexture())
    {
        return false;
    }
    return true;
}

void ImGui_ImplDX8_InvalidateDeviceObjects()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd || !bd->pd3dDevice)
    {
        return;
    }
    if (bd->pVB)
    {
        bd->pVB->Release();
        bd->pVB = nullptr;
    }
    if (bd->pIB)
    {
        bd->pIB->Release();
        bd->pIB = nullptr;
    }
    if (bd->FontTexture)
    {
        bd->FontTexture->Release();
        bd->FontTexture = nullptr;
        ImGui::GetIO().Fonts->SetTexID(NULL);
    } // We copied bd->pFontTextureView to io.Fonts->TexID so let's clear that as well.
}

void ImGui_ImplDX8_NewFrame()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplDX8_Init()?");

    if (!bd->FontTexture)
    {
        ImGui_ImplDX8_CreateDeviceObjects();
    }
}
