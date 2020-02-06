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

}
