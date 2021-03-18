#include "ZEpch.h"
#include "Engine/ImGui/MyImGui.h"

#include <imgui_internal.h>

namespace ImGui {

	static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
	static ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

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

	bool BeginPopupWithPadding(const char* str_id, ImGuiWindowFlags flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
		bool is_open = BeginPopup(str_id, flags);
		ImGui::PopStyleVar();
		return is_open;
	}

	bool BeginPopupContextWindowWithPadding(const char* str_id, ImGuiPopupFlags popup_flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
		bool is_open = BeginPopupContextWindow(str_id, popup_flags);
		ImGui::PopStyleVar();
		return is_open;
	}

	bool BeginPopupContextItemWithPadding(const char* str_id, ImGuiPopupFlags popup_flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
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
		size_t type_size = GDataTypeInfo[data_type].Size;
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
		bool pressed = TransparentButtonEx(label, ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine);
		g.Style.FramePadding.y = backup_padding_y;
		return pressed;
	}

	bool TransparentButton(const char* label, const ImVec2& size_arg)
	{
		return TransparentButtonEx(label, size_arg, ImGuiButtonFlags_None);
	}

	bool TransparentButtonEx(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);

		ImVec2 pos = window->DC.CursorPos;
		if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
			pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
		ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		const ImRect bb(pos, pos + size);
		ItemSize(size, style.FramePadding.y);
		if (!ItemAdd(bb, id))
			return false;

		if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;
		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button, held || hovered ? 1.0f : 0.0f);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

		if (g.LogEnabled)
			LogSetNextTextDecoration("[", "]");
		RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

		// Automatically close popups
		//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
		//    CloseCurrentPopup();

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
		return pressed;
	}

}
