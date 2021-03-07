#include "ZEpch.h"
#include "Engine/ImGui/MyImGui.h"

#include <imgui_internal.h>

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

	bool DragScalarNEx(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
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
		for (int i = 0; i < components; ++i)
		{
			PushID(i);
			if (i > 0)
			{
				SameLine(0, g.Style.ItemInnerSpacing.x);
			}
			value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);

			if (components > 1)
			{
				const ImVec2 min = GetItemRectMin();
				const ImVec2 max = GetItemRectMax();
				const float spacing = g.Style.FrameRounding;
				const float halfSpacing = spacing * 0.5f;
				static const ImU32 colors[] =
				{
					0xBB0000FF, // Red
					0xBB00FF00, // Green
					0xBBFF0000, // Blue
					0xBBFFFFFF, // White for alpha?
				};
				window->DrawList->AddLine({ min.x + spacing, max.y - halfSpacing }, { max.x - spacing, max.y - halfSpacing }, colors[i], 3);
			}

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
