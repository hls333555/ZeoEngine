#include "Utils/EditorSceneUtils.h"

#include "Core/Editor.h"
#include "Engine/Utils/SceneUtils.h"
#include "Engine/Renderer/EditorCamera.h"
#include "Engine/GameFramework/Components.h"
#include "Worlds/EditorPreviewWorldBase.h"
#include "Worlds/LevelPreviewWorld.h"

namespace ZeoEngine {

	bool SceneUtils::IsLevelRuntime()
	{
		return g_Editor->GetLevelWorld()->IsRuntime();
	}

	bool SceneUtils::IsLevelPlaying()
	{
		const auto levelWorld = g_Editor->GetLevelWorld();
		return levelWorld->IsRuntime() && !levelWorld->IsSimulation();
	}

	Scene& SceneUtils::GetActiveGameScene()
	{
		return *g_Editor->GetLevelWorld()->GetActiveScene();
	}

	void SceneUtils::OpenLevel(const std::string& path)
	{
		g_Editor->LoadLevel(path);
	}

	EditorCamera& EditorSceneUtils::GetEditorCamera()
	{
		return g_Editor->GetLevelWorld()->GetEditorCamera();
	}

	Entity EditorSceneUtils::CreateAndPlaceEntity(Scene& scene, const std::string& name)
	{
		const EditorCamera& editorCamera = GetEditorCamera();
		const Vec3 position = editorCamera.GetPosition() + editorCamera.GetForwardVector() * 2.0f;
		return scene.CreateEntity(name, position);
	}

	Entity EditorSceneUtils::CreateAndPlaceCube(Scene& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Cube");
		entity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultCubeMesh()->GetHandle());
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlaceSphere(Scene& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Sphere");
		entity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultSphereMesh()->GetHandle());
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlaceCapsule(Scene& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Capsule");
		entity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultCapsuleMesh()->GetHandle());
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlacePlane(Scene& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Plane");
		entity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultPlaneMesh()->GetHandle());
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlaceDirectionalLight(Scene& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Directional Light");
		entity.AddComponent<DirectionalLightComponent>();
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlacePointLight(Scene& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Point Light");
		entity.AddComponent<PointLightComponent>();
		return entity;
	}

	Entity EditorSceneUtils::CreateAndPlaceSpotLight(Scene& scene)
	{
		Entity entity = CreateAndPlaceEntity(scene, "Spot Light");
		entity.AddComponent<SpotLightComponent>();
		return entity;
	}

}
