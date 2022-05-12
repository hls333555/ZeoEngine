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

		explicit AssetBrowser(AssetTypeId typeId)
			: TypeId(typeId) {}

		/**
		 * Draw the complete asset browing widget.
		 *
		 * @param assetPath - Path of the selected asset
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @param rightClickFunc - Function to execute when right clicking on the asset preview (should be a series of ImGui::MenuItem() calls)
		 * @return A pair, the key indicates if the selection changed, the value is the specification of the selected asset
		 */
		template<typename AssetRightClickFunc>
		std::pair<bool, Ref<AssetSpec>> Draw(const std::string& assetPath, float rightPadding, AssetRightClickFunc rightClickFunc)
		{
			bool bIsValueChanged = false;
			auto retSpec = AssetRegistry::Get().GetPathSpec<AssetSpec>(assetPath);

			ImGui::PushID(TypeId);
			{
				// Asset preview
				{
					static const float assetPreviewWidth = ImGui::GetStyle().Alpha * 64.0f;
					static const float previewRounding = 5.0f;

					// Draw asset thumbnail or default background
					auto textureID = Texture2DLibrary::GetAssetBackgroundTexture()->GetTextureID();
					ImGui::AssetThumbnail(retSpec ? retSpec->ThumbnailTexture->GetTextureID() : textureID,
						assetPreviewWidth, previewRounding, true, textureID);

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
							if (ImGui::MenuItem(ICON_FA_EDIT "  Edit"))
							{
								AssetManager::Get().OpenAsset(retSpec->Path);
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
							auto spec = *(const Ref<AssetSpec>*)payload->Data;
							bIsValueChanged = spec != retSpec;
							retSpec = spec;
						}
					}
				}

				ImGui::SameLine();

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

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
								static const float thumbnailRounding = 4.0f;

								bool bIsSelected = ImGui::Selectable("##AssetSelectable", false, 0, ImVec2(0.0f, assetThumbnailWidth));
								// Display asset path tooltip for drop-down asset
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltipWithPadding("%s", spec->Path.c_str());
								}

								ImGui::SameLine();

								ImGui::AssetThumbnail(spec->ThumbnailTexture->GetTextureID(),
									assetThumbnailWidth, thumbnailRounding, false);

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
