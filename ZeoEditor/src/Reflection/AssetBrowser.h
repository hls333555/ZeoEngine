#pragma once
#include <imgui.h>

#include "Engine/Core/EngineTypes.h"
#include "Engine/ImGui/TextFilter.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Renderer/Texture.h"
#include "Core/AssetManager.h"

namespace ZeoEngine {

	struct AssetBrowser
	{
		AssetTypeId TypeId;
		TextFilter Filter;

		/**
		 * Draw the complete asset browing widget.
		 *
		 * @param assetPath - Path of the selected asset
		 * @param comboBoxWidth - Width of combo box, may vary in different context
		 * @param rightClickFunc - Function to execute when right clicking on the asset preview (should be a series of ImGui::MenuItem() calls)
		 * @return A pair, the key indicates if the selection changed, the value is the specification of the selected asset
		 */
		template<typename AssetRightClickFunc>
		std::pair<bool, Ref<AssetSpec>> Draw(const std::string& assetPath, float comboBoxWidth, AssetRightClickFunc rightClickFunc)
		{
			bool bIsValueChanged = false;
			auto retSpec = AssetRegistry::Get().GetPathSpec<AssetSpec>(assetPath);

			ImGui::PushID(TypeId);
			{
				// Asset preview
				{
					static constexpr float assetPreviewWidth = 75.0f;
					auto thumbnailTexture = retSpec && retSpec->ThumbnailTexture ? retSpec->ThumbnailTexture : Ref<Texture2D>{};
					// For transparent texture
					if (thumbnailTexture && thumbnailTexture->HasAlpha())
					{
						// Draw background first
						ImGui::GetWindowDrawList()->AddImage(Texture2D::s_DefaultBackgroundTexture->GetTextureID(),
							{ ImGui::GetCursorScreenPos().x + 1.0f, ImGui::GetCursorScreenPos().y + 1.0f },
							{ ImGui::GetCursorScreenPos().x + assetPreviewWidth + 1.0f, ImGui::GetCursorScreenPos().y + assetPreviewWidth + 1.0f },
							{ 0.0f, 1.0f }, { 1.0f, 0.0f });
					}
					
					// Draw preview thumbnail or default texture
					ImGui::Image(thumbnailTexture ? thumbnailTexture->GetTextureID() : Texture2D::s_DefaultBackgroundTexture->GetTextureID(),
						{ assetPreviewWidth, assetPreviewWidth },
						{ 0.0f, 1.0f }, { 1.0f, 0.0f },
						{ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.2f, 0.2f, 0.2f, 1.0f });

					// If asset is set...
					if (retSpec)
					{
						if (ImGui::IsItemHovered())
						{
							// Double-click on the preview thumbnail to open the asset editor
							if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							{
								AssetManager::Get().OpenAsset(retSpec->Path);
							}
						}

						// Right-click on the preview thumbnail to open the popup menu
						if (ImGui::BeginPopupContextItemWithPadding("AssetOptions"))
						{
							rightClickFunc();

							ImGui::EndPopup();
						}
					}

				}

				ImGui::SameLine();

				ImGui::SetNextItemWidth(comboBoxWidth);

				if (ImGui::BeginCombo("##AssetBrowser", retSpec ? retSpec->PathName.c_str() : nullptr))
				{
					bool bIsBufferChanged = false;

					// Clear current selection
					if (ImGui::Selectable("Clear"))
					{
						bIsValueChanged = static_cast<bool>(retSpec);
						retSpec = {};
					}

					ImGui::Separator();

					Filter.Draw("##AssetFilter", "Search assets");

					bool bIsListEmpty = true;
					// List all registered assets from AssetRegistry
					AssetRegistry::Get().ForEachAssetByTypeId(TypeId, [&](const Ref<AssetSpec>& spec)
					{
						if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(spec->PathName.c_str()))
						{
							bIsListEmpty = false;

							// Push asset path as id
							ImGui::PushID(spec->Path.c_str());
							{
								static constexpr float assetThumbnailWidth = 30.0f;
								bool bIsSelected = ImGui::Selectable("##AssetThumbnail", false, 0, ImVec2(0.0f, assetThumbnailWidth));
								// Display asset path tooltip for drop-down asset
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltipWithPadding("%s", spec->Path.c_str());
								}

								ImGui::SameLine();

								if (spec->ThumbnailTexture)
								{
									// Draw asset thumbnail
									ImGui::Image(spec->ThumbnailTexture->GetTextureID(),
										ImVec2(assetThumbnailWidth, assetThumbnailWidth),
										ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
								}

								ImGui::SameLine();

								// Display asset name
								ImGui::Text(spec->PathName.c_str());
								if (bIsSelected)
								{
									bIsValueChanged = spec != retSpec;
									retSpec = spec;
								}
							}
							ImGui::PopID();
						}
					});

					if (bIsListEmpty)
					{
						Filter.DrawEmptyText();
					}

					ImGui::EndCombo();
				}
			}
			ImGui::PopID();

			// Display path tooltip for current selection
			if (retSpec && ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("%s", retSpec->Path.c_str());
			}

			return std::make_pair(bIsValueChanged, retSpec);
		}
	};

}
