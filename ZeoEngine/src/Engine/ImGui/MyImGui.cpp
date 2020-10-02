#include "ZEpch.h"
#include "Engine/ImGui/MyImGui.h"

namespace ImGui {

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
