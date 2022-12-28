#pragma once

#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

namespace ImGui {

	IMGUI_API void TextCentered(const char* fmt, ...);

	IMGUI_API void SetTooltipWithPadding(const char* fmt, ...) IM_FMTARGS(1);
	IMGUI_API void BeginTooltipWithPadding();
	IMGUI_API void EndTooltipWithPadding();
	IMGUI_API bool BeginPopupWithPadding(const char* str_id, ImGuiWindowFlags flags = 0);
	IMGUI_API bool BeginPopupContextWindowWithPadding(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);
	IMGUI_API bool BeginPopupContextItemWithPadding(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);

	IMGUI_API void PushMultiItemsWidthsWithLabels(int components, float label_width, float w_full);
	/** Version that displays (x,y,z) labels before each component (components > 1). */
	IMGUI_API bool DragScalarNEx(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);

	/** Version that only renders text by default. */
	IMGUI_API bool TransparentSmallButton(const char* label);
	IMGUI_API bool TransparentButton(const char* label, const ImVec2& size = ImVec2(0, 0));
	IMGUI_API bool ActiveTransparentButton(const char* label, bool bActive, const ImVec2& size = ImVec2(0, 0));
	/** This button does not contain label text, but it reserves space for 2 lines of wrapped text */
	IMGUI_API bool TileImageButton(ImTextureID user_texture_id, bool bDrawImageBackground, bool bIsDisabled, bool bIsSelected, const ImVec2& size, float rounding, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
	IMGUI_API bool TileImageButtonEx(ImGuiID id, ImTextureID texture_id, bool bDrawImageBackground, bool bIsDisabled, bool bIsSelected, const ImVec2& size, float rounding, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& padding, const ImVec4& tint_col);

	IMGUI_API void HelpMarker(const char* desc);

	IMGUI_API ImVec2 GetFramePadding();

	IMGUI_API void VSplitter(const char* str_id, ImVec2* size);

	IMGUI_API void ImageRounded(ImTextureID user_texture_id, const ImVec2& size, float rounding, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4 & border_col = ImVec4(0, 0, 0, 0), ImDrawFlags flags = 0);

	IMGUI_API void AssetThumbnail(ImTextureID thumbnailTextureID, float thumbnailSize, float rounding, bool bShouldDrawBackground, ImTextureID backgroundTextureID = nullptr);

	/** Version that modifies the hightlight style for drop target. */
	IMGUI_API const ImGuiPayload* MyAcceptDragDropPayload(const char* type, float highlightRounding = 0.0f, ImGuiDragDropFlags flags = 0);

	IMGUI_API void SetMouseEnabled(bool bEnable);
	IMGUI_API void SetKeyboardNavEnabled(bool bEnable);

}
