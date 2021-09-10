#pragma once

#include <imgui.h>
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
	/** This button does not contain label text, but it reserves space for 2 lines of wrapped text */
	IMGUI_API bool TileImageButton(ImTextureID user_texture_id, bool bIsDisabled, const ImVec2& size, float rounding, bool bIsSelected, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
	IMGUI_API bool TileImageButtonEx(ImGuiID id, ImTextureID texture_id, bool bIsDisabled, const ImVec2& size, float rounding, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& padding, const ImVec4& bg_col, const ImVec4& tint_col);

	IMGUI_API void HelpMarker(const char* desc);

	IMGUI_API ImRect GetWindowWorkRect();

	IMGUI_API ImVec2 GetFramePadding();

	IMGUI_API void VSplitter(const char* str_id, ImVec2* size);

	IMGUI_API void ImageRounded(ImTextureID user_texture_id, const ImVec2& size, float rounding, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4 & border_col = ImVec4(0, 0, 0, 0));

	IMGUI_API void AssetThumbnail(ImTextureID thumbnailTextureID, float thumbnailSize, float rounding, bool bShouldDrawBackground, ImTextureID backgroundTextureID = nullptr);
}

namespace ZeoEngine {
	
	struct ImVec2Data
	{
		ImVec2 Data;
		ImGuiCond Condition{ ImGuiCond_FirstUseEver };

		bool operator==(const ImVec2Data& other) const
		{
			return Data.x == other.Data.x && Data.y == other.Data.y && Condition == other.Condition;
		}
	};

}
