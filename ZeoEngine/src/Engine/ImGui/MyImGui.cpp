#include "ZEpch.h"
#include "Engine/ImGui/MyImGui.h"

namespace ImGui {

	IMGUI_API bool DragInt_8(const char* label, int8_t* v, float v_speed, int8_t v_min, int8_t v_max, const char* format)
	{
		return DragScalar(label, ImGuiDataType_S8, v, v_speed, &v_min, &v_max, format);
	}

	IMGUI_API bool DragInt_32(const char* label, int32_t* v, float v_speed, int32_t v_min, int32_t v_max, const char* format)
	{
		return DragScalar(label, ImGuiDataType_S32, v, v_speed, &v_min, &v_max, format);
	}

	IMGUI_API bool DragInt_64(const char* label, int64_t* v, float v_speed, int64_t v_min, int64_t v_max, const char* format)
	{
		return DragScalar(label, ImGuiDataType_S64, v, v_speed, &v_min, &v_max, format);
	}

	IMGUI_API bool DragUInt_8(const char* label, uint8_t* v, float v_speed, uint8_t v_min, uint8_t v_max, const char* format)
	{
		return DragScalar(label, ImGuiDataType_U8, v, v_speed, &v_min, &v_max, format);
	}

	IMGUI_API bool DragUInt_32(const char* label, uint32_t* v, float v_speed, uint32_t v_min, uint32_t v_max, const char* format)
	{
		return DragScalar(label, ImGuiDataType_U32, v, v_speed, &v_min, &v_max, format);
	}

	IMGUI_API bool DragUInt_64(const char* label, uint64_t* v, float v_speed, uint64_t v_min, uint64_t v_max, const char* format)
	{
		return DragScalar(label, ImGuiDataType_U64, v, v_speed, &v_min, &v_max, format);
	}

	IMGUI_API bool DragDouble(const char* label, double* v, float v_speed, double v_min, double v_max, const char* format, float power)
	{
		return DragScalar(label, ImGuiDataType_Double, v, v_speed, &v_min, &v_max, format, power);
	}

	IMGUI_API void TextCentered(const char* fmt, ...)
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

	IMGUI_API void AddProgressBar(ImDrawList* drawList, float fraction, const ImVec2& a, const ImVec2& b, ImU32 foregroundCol, ImU32 backgroundCol)
	{
		if (!drawList)
			return;

		drawList->AddRectFilled(a, b, backgroundCol);
		drawList->AddRectFilled(a, { a.x + fraction * (b.x - a.x), b.y }, foregroundCol);
	}

}

namespace ZeoEngine {

	ImVec2Data ImVec2Data::DefaultPos{ { -1.0f, -1.0f } };
	ImVec2Data ImVec2Data::DefaultSize{ { 800.0f, 600.0f } };

}
