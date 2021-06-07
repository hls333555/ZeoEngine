#pragma once
#include <imgui.h>

#include "Engine/Core/EngineTypes.h"
#include "Engine/ImGui/TextFilter.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Core/AssetManager.h"
#include "Engine/Core/AssetFactory.h"

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
					static const float assetPreviewWidth = ImGui::GetStyle().Alpha * 64.0f;

					// If asset is set...
					if (retSpec)
					{
						// Draw background first
						ImGui::GetWindowDrawList()->AddImage(Texture2D::s_DefaultBackgroundTexture->GetTextureID(),
							{ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y },
							{ ImGui::GetCursorScreenPos().x + assetPreviewWidth, ImGui::GetCursorScreenPos().y + assetPreviewWidth },
							{ 0.0f, 1.0f }, { 1.0f, 0.0f });

						auto thumbnailTexture = retSpec->ThumbnailTexture ? retSpec->ThumbnailTexture : AssetManager::Get().GetAssetTypeIcon(TypeId);
						// Draw asset thumbnail or default icon
						ImGui::Image(thumbnailTexture->GetTextureID(),
							{ assetPreviewWidth, assetPreviewWidth },
							{ 0.0f, 1.0f }, { 1.0f, 0.0f });

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
							if (ImGui::MenuItem(ICON_FA_EDIT "  Open"))
							{
								AssetManager::Get().OpenAsset(retSpec->Path);
							}

							rightClickFunc();

							ImGui::EndPopup();
						}
					}
					else
					{
						ImGui::Image(Texture2D::s_DefaultBackgroundTexture->GetTextureID(),
							{ assetPreviewWidth, assetPreviewWidth },
							{ 0.0f, 1.0f }, { 1.0f, 0.0f });
					}

					// Accept asset dragging from the Content Browser
					if (ImGui::BeginDragDropTarget())
					{
						char typeStr[32];
						_itoa_s(TypeId, typeStr, 10);
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(typeStr))
						{
							auto spec = *(const Ref<AssetSpec>*)payload->Data;
							bIsValueChanged = spec != retSpec;
							retSpec = spec;
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
								static const float assetThumbnailWidth = ImGui::GetStyle().Alpha * 32.0f;
								bool bIsSelected = ImGui::Selectable("##AssetSelectable", false, 0, ImVec2(0.0f, assetThumbnailWidth));
								// Display asset path tooltip for drop-down asset
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltipWithPadding("%s", spec->Path.c_str());
								}

								ImGui::SameLine();

								bool bThumbnailExists = static_cast<bool>(spec->ThumbnailTexture);
								if (bThumbnailExists)
								{
									// Draw background first if thumbnail exists
									ImGui::GetWindowDrawList()->AddImage(Texture2D::s_DefaultBackgroundTexture->GetTextureID(),
										{ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y },
										{ ImGui::GetCursorScreenPos().x + assetThumbnailWidth, ImGui::GetCursorScreenPos().y + assetThumbnailWidth },
										{ 0.0f, 1.0f }, { 1.0f, 0.0f });
								}

								auto thumbnailTexture = bThumbnailExists ? spec->ThumbnailTexture : AssetManager::Get().GetAssetTypeIcon(TypeId);
								// Draw asset thumbnail or default icon
								ImGui::Image(thumbnailTexture->GetTextureID(),
									{ assetThumbnailWidth, assetThumbnailWidth },
									{ 0.0f, 1.0f }, { 1.0f, 0.0f });

								ImGui::SameLine();

								// Align text
								ImGui::BeginGroup();
								{
									// Make two lines of text more compact
									ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
									// Display asset name
									ImGui::Text(spec->PathName.c_str());
									// Display asset type name
									ImGui::TextColored({ 0.6f, 0.6f, 0.6f, 1.0f }, AssetManager::Get().GetAssetFactoryByAssetType(TypeId)->GetAssetTypeName());
									ImGui::PopStyleVar();
								}
								ImGui::EndGroup();

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
