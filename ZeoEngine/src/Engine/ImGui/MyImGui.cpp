#include "ZEpch.h"
#include "Engine/ImGui/MyImGui.h"

#include <IconsFontAwesome5.h>

namespace ImGui {

	void TextCentered(const char* fmt, ...)
	{
		ImVec2 textSize = CalcTextSize(fmt);
		float indent = (GetContentRegionAvail().x - textSize.x) * 0.5f;
		Indent(indent);
		va_list args;
		va_start(args, fmt);
		TextV(fmt, args);
		va_end(args);
		Unindent(indent);
	}

	void SetTooltipWithPadding(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		SetTooltipV(fmt, args);
		PopStyleVar();
		va_end(args);
	}

	void BeginTooltipWithPadding()
	{
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		BeginTooltip();
	}

	void EndTooltipWithPadding()
	{
		EndTooltip();
		PopStyleVar();
	}

	bool BeginPopupWithPadding(const char* str_id, ImGuiWindowFlags flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		bool is_open = BeginPopup(str_id, flags);
		ImGui::PopStyleVar();
		return is_open;
	}

	bool BeginPopupContextWindowWithPadding(const char* str_id, ImGuiPopupFlags popup_flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		bool is_open = BeginPopupContextWindow(str_id, popup_flags);
		ImGui::PopStyleVar();
		return is_open;
	}

	bool BeginPopupContextItemWithPadding(const char* str_id, ImGuiPopupFlags popup_flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		bool is_open = BeginPopupContextItem(str_id, popup_flags);
		ImGui::PopStyleVar();
		return is_open;
	}

	static const ImGuiDataTypeInfo GDataTypeInfo[] =
	{
		{ sizeof(char),             "S8",   "%d",   "%d"    },  // ImGuiDataType_S8
		{ sizeof(unsigned char),    "U8",   "%u",   "%u"    },
		{ sizeof(short),            "S16",  "%d",   "%d"    },  // ImGuiDataType_S16
		{ sizeof(unsigned short),   "U16",  "%u",   "%u"    },
		{ sizeof(int),              "S32",  "%d",   "%d"    },  // ImGuiDataType_S32
		{ sizeof(unsigned int),     "U32",  "%u",   "%u"    },
	#ifdef _MSC_VER
		{ sizeof(ImS64),            "S64",  "%I64d","%I64d" },  // ImGuiDataType_S64
		{ sizeof(ImU64),            "U64",  "%I64u","%I64u" },
	#else
		{ sizeof(ImS64),            "S64",  "%lld", "%lld"  },  // ImGuiDataType_S64
		{ sizeof(ImU64),            "U64",  "%llu", "%llu"  },
	#endif
		{ sizeof(float),            "float", "%f",  "%f"    },  // ImGuiDataType_Float (float are promoted to double in va_arg)
		{ sizeof(double),           "double","%f",  "%lf"   },  // ImGuiDataType_Double
	};
	IM_STATIC_ASSERT(IM_ARRAYSIZE(GDataTypeInfo) == ImGuiDataType_COUNT);

	void PushMultiItemsWidthsWithLabels(int components, float label_width, float w_full)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const ImGuiStyle& style = g.Style;
		const float w_item_one = ImMax(1.0f, IM_FLOOR((w_full - (style.ItemInnerSpacing.x) * (components - 1 + components)) / (float)components) - label_width);
		const float w_item_last = ImMax(1.0f, IM_FLOOR(w_full - (w_item_one + label_width) * (components - 1) - style.ItemInnerSpacing.x * (components - 1 + components) - label_width));
		window->DC.ItemWidthStack.push_back(window->DC.ItemWidth); // Backup current width
		window->DC.ItemWidthStack.push_back(w_item_last);
		for (int i = 0; i < components - 2; i++)
			window->DC.ItemWidthStack.push_back(w_item_one);
		window->DC.ItemWidth = (components == 1) ? w_item_last : w_item_one;
		g.NextItemData.Flags &= ~ImGuiNextItemDataFlags_HasWidth;
	}

	bool DragScalarNEx(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		PushMultiItemsWidthsWithLabels(components, CalcTextSize("X").x, CalcItemWidth());
		ZeoEngine::SizeT type_size = GDataTypeInfo[data_type].Size;
		for (int i = 0; i < components; ++i)
		{
			PushID(i);
			if (i > 0)
			{
				SameLine(0, g.Style.ItemInnerSpacing.x);
			}
			if (components > 1)
			{
				static const char* labels[] =
				{
					"X", "Y", "Z"
				};
				static const ImVec4 colors[] =
				{
					{ 1.0f, 0.0f, 0.0f, 1.0f},
					{ 0.0f, 1.0f, 0.0f, 1.0f},
					{ 0.0f, 0.0f, 1.0f, 1.0f}
				};
				ImGui::TextColored(colors[i], labels[i]);
				ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
			}
			value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);
			PopID();
			PopItemWidth();
			p_data = (void*)((char*)p_data + type_size);
		}
		PopID();

		const char* label_end = FindRenderedTextEnd(label);
		if (label != label_end)
		{
			SameLine(0, g.Style.ItemInnerSpacing.x);
			TextEx(label, label_end);
		}

		EndGroup();
		return value_changed;
	}

	bool TransparentSmallButton(const char* label)
	{
		ImGuiContext& g = *GImGui;
		float backup_padding_y = g.Style.FramePadding.y;
		g.Style.FramePadding.y = 0.0f;
		PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		bool pressed = ButtonEx(label, ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine);
		PopStyleColor();
		g.Style.FramePadding.y = backup_padding_y;
		return pressed;
	}

	bool TransparentButton(const char* label, const ImVec2& size_arg)
	{
		PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		bool pressed = ButtonEx(label, size_arg, ImGuiButtonFlags_None);
		PopStyleColor();
		return pressed;
	}

	bool TileImageButton(ImTextureID user_texture_id, bool bDrawImageBackground, bool bIsDisabled, bool bIsSelected, const ImVec2& size, float rounding, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& tint_col)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		// Default to using texture ID as ID. User can still push string/integer prefixes.
		PushID((void*)(intptr_t)user_texture_id);
		const ImGuiID id = window->GetID("#image");
		PopID();

		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : g.Style.FramePadding;
		if (!bIsSelected)
		{
			PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		}
		bool pressed = TileImageButtonEx(id, user_texture_id, bDrawImageBackground, bIsDisabled, bIsSelected, size, rounding, uv0, uv1, padding, tint_col);
		if (!bIsSelected)
		{
			PopStyleColor();
		}
		return pressed;
	}

	bool TileImageButtonEx(ImGuiID id, ImTextureID texture_id, bool bDrawImageBackground, bool bIsDisabled, bool bIsSelected, const ImVec2& size, float rounding, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& padding, const ImVec4& tint_col)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		// Limit the wrapped text up to 2 lines
		const ZeoEngine::I32 maxTextLine = 2;
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + ImVec2{ padding.x * 2, padding.y * (4 + maxTextLine - 1) } + ImVec2{ 0, g.FontSize * maxTextLine });
		ItemSize(bb);

		// Some of the "disabled" code are copied from ImGui::Selectable
		bool item_add;
		if (bIsDisabled)
		{
			ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
			g.CurrentItemFlags |= ImGuiItemFlags_Disabled;
			item_add = ItemAdd(bb, id);
			g.CurrentItemFlags = backup_item_flags;
		}
		else
		{
			item_add = ItemAdd(bb, id);
		}

		if (!item_add)
			return false;

		const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
		if (bIsDisabled && !disabled_global) // Only testing this as an optimization
			BeginDisabled(true);

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		// Render
		RenderNavHighlight(bb, id);
		ImU32 col;
		if (bDrawImageBackground)
		{
			col = GetColorU32(bIsSelected || held ? ImVec4{ 0.0f, 0.6f, 1.0f, 1.0f } : hovered ? ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
			float shadowSize = 2.0f;
			float borderSize = bIsSelected ? 2.5f : 1.5f;
			// Draw shadow
			window->DrawList->AddRect(bb.Min + ImVec2(1, 1), bb.Max + ImVec2(1, 1), GetColorU32({ 0.05f, 0.05f, 0.05f, 1.0f }), rounding, 0, shadowSize);
			// Draw outline (hovered/selected)
			window->DrawList->AddRect(bb.Min, bb.Max, col, rounding, 0, borderSize);
			// Draw up black background
			window->DrawList->AddRectFilled(bb.Min, bb.Min + padding * 2 + size, GetColorU32({ 0.1f, 0.1f, 0.1f, 1.0f }), rounding, ImDrawFlags_RoundCornersTop);
			// Draw down grey background
			window->DrawList->AddRectFilled(bb.Min + ImVec2{ 0, size.y + padding.y * 2 }, bb.Max, GetColorU32({ 0.19f, 0.19f, 0.19f, 1.0f }), rounding, ImDrawFlags_RoundCornersBottom);
		}
		else
		{
			col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
		}
		// Draw icon image
		window->DrawList->AddImageRounded(texture_id, bb.Min + padding, bb.Max - ImVec2{ padding.x, padding.y * (3 + maxTextLine - 1) + g.FontSize * 2 }, uv0, uv1, GetColorU32(tint_col), rounding);
		SetCursorScreenPos(bb.Min + padding + ImVec2{ 0, size.y + padding.y * 2 });

		if (bIsDisabled && !disabled_global)
			EndDisabled();

		return pressed;
	}

	void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled(ICON_FA_QUESTION_CIRCLE);
		if (ImGui::IsItemHovered())
		{
			PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
			PopStyleVar();
		}
	}

	ImVec2 GetFramePadding()
	{
		return GImGui->Style.FramePadding;
	}

	void VSplitter(const char* str_id, ImVec2* size)
	{
		ImVec2 screen_pos = GetCursorScreenPos();
		InvisibleButton(str_id, ImVec2(3, size->y == 0 ? -1 : size->y));
		ImVec2 end_pos = screen_pos + GetItemRectSize();
		ImGuiWindow* window = GetCurrentWindow();
		ImVec4* colors = GetStyle().Colors;
		ImU32 color = GetColorU32(IsItemActive() ? colors[ImGuiCol_SeparatorActive] : (IsItemHovered() ? colors[ImGuiCol_SeparatorHovered] : colors[ImGuiCol_Separator]));
		window->DrawList->AddRectFilled(screen_pos, end_pos, color);
		if (IsItemHovered())
		{
			SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}
		if (IsItemActive())
		{
			size->x = ImMax(1.0f, GetIO().MouseDelta.x + size->x);
			SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}
	}

	void ImageRounded(ImTextureID user_texture_id, const ImVec2& size, float rounding, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col, ImDrawFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		if (border_col.w > 0.0f)
			bb.Max += ImVec2(2, 2);
		ItemSize(bb);
		if (!ItemAdd(bb, 0))
			return;

		if (border_col.w > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), rounding, flags);
			window->DrawList->AddImageRounded(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col), rounding, flags);
		}
		else
		{
			window->DrawList->AddImageRounded(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col), rounding, flags);
		}
	}

	void AssetThumbnail(ImTextureID thumbnailTextureID, float thumbnailSize, float rounding, bool bShouldDrawBackground, ImTextureID backgroundTextureID)
	{
		if (bShouldDrawBackground)
		{
			// Draw background first if needed
			ImGui::GetWindowDrawList()->AddImageRounded(backgroundTextureID,
				{ ImGui::GetCursorScreenPos().x + 1.0f, ImGui::GetCursorScreenPos().y + 1.0f },
				{ ImGui::GetCursorScreenPos().x + 1.0f + thumbnailSize, ImGui::GetCursorScreenPos().y + 1.0f + thumbnailSize },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f },
				ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }), rounding);
		}

		// Draw asset thumbnail or default icon
		ImGui::ImageRounded(thumbnailTextureID,
			{ thumbnailSize, thumbnailSize }, rounding,
			{ 0.0f, 1.0f }, { 1.0f, 0.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.2039f, 0.2039f, 0.2039f, bShouldDrawBackground ? 1.0f : 0.0f});
	}

	const ImGuiPayload* MyAcceptDragDropPayload(const char* type, float highlightRounding, ImGuiDragDropFlags flags)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiPayload& payload = g.DragDropPayload;
		IM_ASSERT(g.DragDropActive);                        // Not called between BeginDragDropTarget() and EndDragDropTarget() ?
		IM_ASSERT(payload.DataFrameCount != -1);            // Forgot to call EndDragDropTarget() ?
		if (type != NULL && !payload.IsDataType(type))
			return NULL;

		// Accept smallest drag target bounding box, this allows us to nest drag targets conveniently without ordering constraints.
		// NB: We currently accept NULL id as target. However, overlapping targets requires a unique ID to function!
		const bool was_accepted_previously = (g.DragDropAcceptIdPrev == g.DragDropTargetId);
		ImRect r = g.DragDropTargetRect;
		float r_surface = r.GetWidth() * r.GetHeight();
		if (r_surface <= g.DragDropAcceptIdCurrRectSurface)
		{
			g.DragDropAcceptFlags = flags;
			g.DragDropAcceptIdCurr = g.DragDropTargetId;
			g.DragDropAcceptIdCurrRectSurface = r_surface;
		}

		// Render default drop visuals
		// FIXME-DRAGDROP: Settle on a proper default visuals for drop target.
		payload.Preview = was_accepted_previously;
		flags |= (g.DragDropSourceFlags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect); // Source can also inhibit the preview (useful for external sources that lives for 1 frame)
		if (!(flags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect) && payload.Preview)
			window->DrawList->AddRect(r.Min - ImVec2(2.f, 2.f), r.Max + ImVec2(2.f, 2.f), GetColorU32(ImGuiCol_DragDropTarget), highlightRounding, 0, 1.0f);

		g.DragDropAcceptFrameCount = g.FrameCount;
		payload.Delivery = was_accepted_previously && !IsMouseDown(g.DragDropMouseButton); // For extern drag sources affecting os window focus, it's easier to just test !IsMouseDown() instead of IsMouseReleased()
		if (!payload.Delivery && !(flags & ImGuiDragDropFlags_AcceptBeforeDelivery))
			return NULL;

		return &payload;
	}

}
