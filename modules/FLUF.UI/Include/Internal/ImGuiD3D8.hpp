#pragma once

#include "imgui.h" // IMGUI_IMPL_API

struct IDirect3DDevice8;

IMGUI_IMPL_API bool ImGui_ImplDX8_Init(IDirect3DDevice8* device);
IMGUI_IMPL_API void ImGui_ImplDX8_Shutdown();
IMGUI_IMPL_API void ImGui_ImplDX8_NewFrame();
IMGUI_IMPL_API void ImGui_ImplDX8_RenderDrawData(ImDrawData* drawData);
IMGUI_IMPL_API void ImGui_ImplDX8_UpdateTexture(ImTextureData* tex);

// Use if you want to reset your rendering device without losing Dear ImGui state.
IMGUI_IMPL_API bool ImGui_ImplDX8_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplDX8_InvalidateDeviceObjects();
