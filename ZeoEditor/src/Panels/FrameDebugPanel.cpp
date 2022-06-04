#include "Panels/FrameDebugPanel.h"

#include "Engine/Renderer/RenderPass.h"

namespace ZeoEngine {

	void FrameDebugPanel::OnAttach()
	{
		PanelBase::OnAttach();


	}

	void FrameDebugPanel::ProcessRender()
	{
		if (ImGui::CollapsingHeader("ShadowMappingPass"))
		{
			if (ImGui::TreeNode("DepthMap"))
			{
				// TODO: Retrieve from setting
				static I32 count = 4;
				static const float previewWidth = 300.0f;
				static const float previewHeight = 200.0f;
				float windowWidth = ImGui::GetContentRegionAvail().x;
				float totalPreviewWidth = previewWidth * count + ImGui::GetFramePadding().x * 2 * count;
				float childWidth = glm::min(windowWidth, totalPreviewWidth);
				float childHeight = previewHeight + ImGui::GetFramePadding().y * 2;
				if (ImGui::BeginChild("DepthMapView", { childWidth, childHeight }, true, ImGuiWindowFlags_HorizontalScrollbar))
				{
					for (int i = 0; i < count; ++i)
					{
						ImGui::Image(ShadowMappingPass::GetDepthFrameBuffer()->GetDepthAttachment(i),
							{ previewWidth, previewHeight }, { 0, 1 }, { 1, 0 });

						if (i < count - 1)
						{
							ImGui::SameLine();
						}
					}
				}
				
				ImGui::EndChild();
				ImGui::TreePop();
			}
		}
	}

}
