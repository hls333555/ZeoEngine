#include "ZEpch.h"
#include "Engine/ImGui/MyImGui.h"

#include <imgui_internal.h>

namespace ImGui {

	void TextCentered(const char* fmt, ...)
	{
		ImVec2 textSize = CalcTextSize(fmt);
		float indent = (GetWindowSize().x - textSize.x) / 2.0f;
		Indent(indent);
		va_list args;
		va_start(args, fmt);
		TextV(fmt, args);
		va_end(args);
		Unindent(indent);
	}

	void AddProgressBar(ImDrawList* drawList, float fraction, const ImVec2& a, const ImVec2& b, ImU32 foregroundCol, ImU32 backgroundCol)
	{
		if (!drawList)
			return;

		drawList->AddRectFilled(a, b, backgroundCol);
		drawList->AddRectFilled(a, { a.x + fraction * (b.x - a.x), b.y }, foregroundCol);
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

	bool DragScalarNEx(const char* label, const char* component_labels[], const ImVec4 component_label_colors[], ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		PushMultiItemsWidths(components, CalcItemWidth());
		size_t type_size = GDataTypeInfo[data_type].Size;
		for (int i = 0; i < components; i++)
		{
			PushID(component_labels[i]);
			PushID(i);
			if (i > 0)
				SameLine(0, g.Style.ItemInnerSpacing.x);
			if (components > 1)
			{
				PushStyleColor(ImGuiCol_Text, component_label_colors[i]);
				TextUnformatted(component_labels[i], FindRenderedTextEnd(component_labels[i]));
				PopStyleColor();
				SameLine();
			}
			value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);
			PopID();
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

}

namespace ZeoEngine {

	ImVec2Data ImVec2Data::DefaultPos{ { -1.0f, -1.0f } };
	ImVec2Data ImVec2Data::DefaultSize{ { 800.0f, 600.0f } };

}
