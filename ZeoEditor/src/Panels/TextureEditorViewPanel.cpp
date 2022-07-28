#include "Panels/TextureEditorViewPanel.h"

#include <magic_enum.hpp>

#include "Editors/EditorBase.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	void TextureEditorViewPanel::ProcessRender()
	{
		// Get default available region before drawing any widgets
		const Vec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
		Vec2 displaySize;
		if (contentRegionAvailable.x < contentRegionAvailable.y)
		{
			displaySize = { contentRegionAvailable.x, contentRegionAvailable.x };
			ImGui::SetCursorPos(ImGui::GetCursorPos() + Vec2{ 0.0f, (contentRegionAvailable.y - displaySize.y) / 2.0f });
		}
		else
		{
			displaySize = { contentRegionAvailable.y,contentRegionAvailable.y };
			ImGui::SetCursorPos(ImGui::GetCursorPos() + Vec2{ (contentRegionAvailable.x - displaySize.x) / 2.0f, 0.0f });
		}

		// Checkerboard background
		ImGui::GetWindowDrawList()->AddImage(Texture2D::GetCheckerboardTexture()->GetTextureID(),
		ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + displaySize,
		{ 0.0f, 1.0f }, { 1.0f, 0.0f },
		ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 0.1f }));

		// Texture
		const auto texture = std::dynamic_pointer_cast<Texture2D>(GetContextEditor()->GetAsset());
		ImGui::Image(texture->GetTextureID(), displaySize, { 0.0f, 1.0f }, { 1.0f, 0.0f });

		// Display texture info at the bottom center
		const U32 textureWidth = texture->GetWidth();
		const U32 textureHeight = texture->GetHeight();
		const auto textureFormat = magic_enum::enum_name(texture->GetFormat());
		// 1024 X 1024 SRGB8 Mipmaps: 10
		const std::string infoStr = std::to_string(textureWidth) + " X " + std::to_string(textureHeight) + " " + std::string(textureFormat) + " Mipmaps: " + std::to_string(texture->GetMipmapLevels());
		const float infoWidth = ImGui::CalcTextSize(infoStr.c_str()).x;
		const auto textPos = ImGui::GetWindowPos() + ImVec2{ (contentRegionAvailable.x - infoWidth) * 0.5f + ImGui::GetFramePadding().x, contentRegionAvailable.y };
		ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }), infoStr.c_str());

	}

}
