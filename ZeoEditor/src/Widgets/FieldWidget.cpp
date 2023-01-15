#include "Widgets/FieldWidget.h"

namespace ZeoEngine {

	std::unordered_map<UUID, std::string> s_FieldStringBuffer;

	namespace Utils {

		ImGuiDataType FieldTypeToImGuiDataType(FieldType type)
		{
			switch (type)
			{
				case FieldType::I8: return ImGuiDataType_S8;
				case FieldType::U8: return ImGuiDataType_U8;
				case FieldType::I16: return ImGuiDataType_S16;
				case FieldType::U16: return ImGuiDataType_U16;
				case FieldType::I32: return ImGuiDataType_S32;
				case FieldType::U32: return ImGuiDataType_U32;
				case FieldType::I64: return ImGuiDataType_S64;
				case FieldType::U64: return ImGuiDataType_U64;
				case FieldType::Float:
				case FieldType::Vec2:
				case FieldType::Vec3: return ImGuiDataType_Float;
				case FieldType::Double: return ImGuiDataType_Double;
			}

			ZE_CORE_ASSERT(false);
			return ImGuiDataType_S8;
		}

		const char* GetOutputFormatByFieldType(FieldType type)
		{
			switch (type)
			{
				case FieldType::I8: return "%hhd";
				case FieldType::U8: return "%hhu";
				case FieldType::I16: return "%hd";
				case FieldType::U16: return "%hu";
				case FieldType::I32: return "%d";
				case FieldType::U32: return "%u";
				case FieldType::I64: return "%lld";
				case FieldType::U64: return "%llu";
				case FieldType::Float:
				case FieldType::Vec2:
				case FieldType::Vec3: return "%.3f";
				case FieldType::Double: return "%.4lf";
			}

			return "";
		}

		bool IsFieldTypeBufferBased(FieldType type)
		{
			switch (type)
			{
				case FieldType::None: ZE_CORE_ASSERT(false);
				case FieldType::Bool:
				case FieldType::Enum:
				case FieldType::SeqCon:
				case FieldType::AssCon:
				case FieldType::Asset:
				case FieldType::Entity: return false;
			}
			return true;
		}

		float GetContainerDropdownWidth()
		{
			return ImGui::GetFontSize() + ImGui::GetFramePadding().y * 5.0f;
		}
	}
	
}
