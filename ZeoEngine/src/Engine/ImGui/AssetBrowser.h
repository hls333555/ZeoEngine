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
		/** If 0, all assets, without any type filtering, will be displayed */
		AssetTypeID TypeID = 0;
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		AssetBrowser() = default;
		AssetBrowser(AssetTypeID typeID)
			: TypeID(typeID) {}

		/**
		 * Draw the asset browsing widget.
		 *
		 * @param outHandle - Handle of the selected asset
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @param rightClickFunc - Function to execute when right clicking on the asset preview (should be a series of ImGui::MenuItem() calls) // TODO:
		 * @param bAllowClear - If false, clear option is not available
		 * @return True if the selection changed
		 */
		template<typename AssetRightClickFunc>
		bool Draw(AssetHandle& outHandle, float rightPadding, AssetRightClickFunc rightClickFunc, bool bAllowClear = true)
		{
			bool bIsValueChanged = false;
			auto* metadata = AssetRegistry::Get().GetAssetMetadata(outHandle);

			ImGui::PushID(TypeID);
			{
				// Asset preview
				{
					static constexpr float assetPreviewWidth = 64.0f;
					static constexpr float previewRounding = 5.0f;

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
								AssetManager::Get().OpenAsset(metadata->Path, true);
							}
						}

						// Right-click on the preview thumbnail to open the popup menu
						if (ImGui::BeginPopupContextItemWithPadding("AssetOptions"))
						{
							if (ImGui::MenuItem(ICON_FA_EDIT "  Edit"))
							{
								AssetManager::Get().OpenAsset(metadata->Path, true);
							}

							rightClickFunc();

							ImGui::EndPopup();
						}
					}

					// Accept asset dragging from the Content Browser
					if (ImGui::BeginDragDropTarget())
					{
						char assetTypeStr[DRAG_DROP_PAYLOAD_TYPE_SIZE];
						_itoa_s(TypeID, assetTypeStr, 10);
						if (const ImGuiPayload* payload = ImGui::MyAcceptDragDropPayload(assetTypeStr))
						{
							auto* md = *static_cast<AssetMetadata**>(payload->Data);
							bIsValueChanged = metadata != md;
							metadata = md;
							outHandle = metadata->Handle;
						}
					}
				}

				ImGui::SameLine();

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightPadding);

				static constexpr int maxCount = 10;
				const int assetCount = glm::clamp(static_cast<int>(LastFilteredCount), 1, maxCount);
				const float fontSize = ImGui::GetFontSize();
				const float itemHeight = LastFilteredCount > 0 ? 32.0f : fontSize;
				if (ImGui::BeginComboFilterWithPadding("", metadata ? metadata->PathName.c_str() : "", assetCount, itemHeight, fontSize + ImGui::GetFramePadding().y * 4/* Clear button and separator */))
				{
					// Clear current selection
					if (bAllowClear && ImGui::Selectable("Clear"))
					{
						bIsValueChanged = static_cast<bool>(metadata);
						metadata = {};
						outHandle = 0;
					}

					ImGui::Separator();

					// Auto-focus search box at startup
					// https://github.com/ocornut/imgui/issues/455#issuecomment-167440172
					if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						ImGui::SetKeyboardFocusHere();
						ImGui::SetScrollHereY();
					}
					Filter.Draw("##AssetFilter", "Search assets");

					const float spacing = GImGui->Style.ItemSpacing.y;
					const ImVec2 size = { -FLT_MIN, (itemHeight + spacing) * assetCount - spacing + ImGui::GetFramePadding().y * 2 };
					if (ImGui::BeginListBox("##AssetBrowserList", size))
					{
						bool bIsListEmpty = true;
						LastFilteredCount = 0;

						// List all registered assets from AssetRegistry
						AssetRegistry::Get().ForEachAssetByTypeID(TypeID, [&](AssetMetadata* md)
						{
							if (!Filter.IsActive() || Filter.IsActive() && Filter.PassFilter(md->PathName.c_str()))
							{
								bIsListEmpty = false;
								++LastFilteredCount;

								// Push asset handle as id
								ImGui::PushID(static_cast<int>(md->Handle));
								{
									static constexpr float thumbnailRounding = 4.0f;

									bool bIsSelected = ImGui::Selectable("", false, 0, ImVec2(0.0f, itemHeight));
									// Display asset path tooltip for drop-down asset
									if (ImGui::IsItemHovered())
									{
										ImGui::SetTooltipWithPadding("%s", md->Path.c_str());
									}

									ImGui::SameLine();

									ImGui::AssetThumbnail(md->ThumbnailTexture->GetTextureID(), itemHeight, thumbnailRounding, false);

									ImGui::SameLine();

									// Align text
									ImGui::BeginGroup();
									{
										// Make two lines of text more compact
										ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
										// Display asset name
										ImGui::Text(md->PathName.c_str());
										// Display asset type name
										ImGui::TextColored({ 0.6f, 0.6f, 0.6f, 1.0f }, AssetManager::Get().GetAssetFactoryByAssetType(md->TypeID)->GetAssetTypeName());
										ImGui::PopStyleVar();
									}
									ImGui::EndGroup();

									if (bIsSelected)
									{
										bIsValueChanged = md != metadata;
										metadata = md;
										outHandle = metadata->Handle;

										ImGui::CloseCurrentPopup();
									}
								}
								ImGui::PopID();
							}
						});

						if (bIsListEmpty)
						{
							Filter.DrawEmptyText();
						}

						ImGui::EndListBox();
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

			return bIsValueChanged;
		}
	};

}
