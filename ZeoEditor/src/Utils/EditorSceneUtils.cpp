#include "Utils/EditorSceneUtils.h"

#include "Engine/Utils/SceneUtils.h"
#include "Core/EditorManager.h"
#include "Core/EditorTypes.h"
#include "Editors/EditorBase.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	Ref<Scene> SceneUtils::GetActiveGameScene()
	{
		return EditorManager::Get().GetEditor(LEVEL_EDITOR)->GetScene();
	}

	void SceneUtils::OpenLevel(const std::filesystem::path& path)
	{
		EditorManager::Get().GetEditor(LEVEL_EDITOR)->LoadScene(path);
	}

	EditorCamera* EditorSceneUtils::GetEditorCamera()
	{
		return EditorManager::Get().GetEditor(LEVEL_EDITOR)->GetEditorCamera();
	}

	Entity EditorSceneUtils::CreateAndPlaceEntity(const Ref<Scene>& scene, const std::string& name)
	{
		EditorCamera* editorCamera = GetEditorCamera();
		const Vec3 position = editorCamera->GetPosition() + editorCamera->GetForwardVector() * 2.0f;
		return scene->CreateEntity(name, position);
	}

	Entity EditorSceneUtils::CreateAndPlaceCube(const Ref<Scene>& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Cube");
		entity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultCubeMesh());
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlaceSphere(const Ref<Scene>& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Sphere");
		entity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultSphereMesh());
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlacePlane(const Ref<Scene>& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Plane");
		entity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultPlaneMesh());
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlaceDirectionalLight(const Ref<Scene>& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Directional Light");
		entity.AddComponent<LightComponent>(LightComponent::LightType::DirectionalLight);
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlacePointLight(const Ref<Scene>& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Point Light");
		entity.AddComponent<LightComponent>(LightComponent::LightType::PointLight);
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlaceSpotLight(const Ref<Scene>& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Spot Light");
		entity.AddComponent<LightComponent>(LightComponent::LightType::SpotLight);
		return entity;
	}

}
