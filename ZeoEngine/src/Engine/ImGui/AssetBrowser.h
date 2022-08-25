#pragma once
#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "Engine/ImGui/TextFilter.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetFactory.h"

namespace ZeoEngine {

	struct AssetBrowser
	{
		AssetTypeID TypeId;
		TextFilter Filter;

		explicit AssetBrowser(AssetTypeID typeId)
			: TypeId(typeId) {}

		/**
		 * Draw the complete asset browsing widget.
		 *
		 * @param handle - Handle of the selected asset
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @param rightClickFunc - Function to execute when right clicking on the asset preview (should be a series of ImGui::MenuItem() calls)
		 * @return A pair, the key indicates if the selection changed, the value is the asset metadata
		 */
		template<typename AssetRightClickFunc>
		std::pair<bool, Ref<AssetMetadata>> Draw(const AssetHandle handle, float rightPadding, AssetRightClickFunc rightClickFunc)
		{
			bool bIsValueChanged = false;
			auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);

			ImGui::PushID(TypeId);
			{
				// Asset preview
				{
					static const float assetPreviewWidth = ImGui::GetStyle().Alpha * 64.0f;
					static const float previewRounding = 5.0f;

					// Draw asset thumbnail or default background
					const auto textureID = Texture2D::GetAssetBackgroundTexture()->GetTextureID();
					ImGui::AssetThumbnail(metadata ? metadata->ThumbnailTexture->GetTextureID() : textureID,
						assetPreviewWidth, previewRounding, true, textureID);

					// If asset is set...
					if (metadata)
					{
						if (ImGui::IsItemHovered())
						{
							// Double-click on the preview thumbnail to open the asset editor
							if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							{
								AssetManager::Get().OpenAsset(metadata->Path);
							}
						}

						// Right-click on the preview thumbnail to open the popup menu
						if (ImGui::BeginPopupContextItemWithPadding("AssetOptions"))
						{
							if (ImGui::MenuItem(ICON_FA_EDIT "  Edit"))
							{
								AssetManager::Get().OpenAsset(metadata->Path);
							}

							rightClickFunc();

							ImGui::EndPopup();
						}
					}

					// Accept asset dragging from the Content Browser
					if (ImGui::BeginDragDropTarget())
					{
						char typeStr[DRAG_DROP_PAYLOAD_TYPE_SIZE];
						_itoa_s(TypeId, typeStr, 10);
						if (const ImGuiPayload* payload = ImGui::MyAcceptDragDropPayload(typeStr, previewRounding))
						{
							const auto md = *(const Ref<AssetMetadata>*)payload->Data;
							bIsValueChanged = metadata != md;
							metadata = md;
						}
					}
				}

				ImGui::SameLine();

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

				if (ImGui::BeginCombo("##AssetBrowser", metadata ? metadata->PathName.c_str() : nullptr, ImGuiComboFlags_HeightLarge))
				{
					// Clear current selection
					if (ImGui::Selectable("Clear"))
					{
						bIsValueChanged = static_cast<bool>(metadata);
						metadata = {};
					}

					ImGui::Separator();

					// Auto-focus search box at startup
					// https://github.com/ocornut/imgui/issues/455#issuecomment-167440172
					if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
					{
						ImGui::SetKeyboardFocusHere();
						ImGui::SetScrollHereY();
					}
					Filter.Draw("##AssetFilter", "Search assets");

					bool bIsListEmpty = true;
					// List all registered assets from AssetRegistry
					AssetRegistry::Get().ForEachAssetByTypeID(TypeId, [&](const Ref<AssetMetadata>& md)
					{
						if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(md->PathName.c_str()))
						{
							bIsListEmpty = false;

							// Push asset path as id
							ImGui::PushID(md->Path.c_str());
							{
								static const float assetThumbnailWidth = ImGui::GetStyle().Alpha * 32.0f;
								static const float thumbnailRounding = 4.0f;

								bool bIsSelected = ImGui::Selectable("##AssetSelectable", false, 0, ImVec2(0.0f, assetThumbnailWidth));
								// Display asset path tooltip for drop-down asset
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltipWithPadding("%s", md->Path.c_str());
								}

								ImGui::SameLine();

								ImGui::AssetThumbnail(md->ThumbnailTexture->GetTextureID(),
									assetThumbnailWidth, thumbnailRounding, false);

								ImGui::SameLine();

								// Align text
								ImGui::BeginGroup();
								{
									// Make two lines of text more compact
									ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
									// Display asset name
									ImGui::Text(md->PathName.c_str());
									// Display asset type name
									ImGui::TextColored({ 0.6f, 0.6f, 0.6f, 1.0f }, AssetManager::Get().GetAssetFactoryByAssetType(TypeId)->GetAssetTypeName());
									ImGui::PopStyleVar();
								}
								ImGui::EndGroup();

								if (bIsSelected)
								{
									bIsValueChanged = md != metadata;
									metadata = md;
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
			if (metadata && ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("%s", metadata->Path.c_str());
			}

			return std::make_pair(bIsValueChanged, metadata);
		}
	};

}
