#pragma once

#include "Engine/Core/Core.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class EditorCamera;

	class EditorSceneUtils
	{
	public:
		static bool IsRuntime();
		static EditorCamera& GetEditorCamera();
		static Entity CreateAndPlaceEntity(const Ref<Scene>& scene, const std::string& name = "New Entity");
		static Entity CreateAndPlaceCube(const Ref<Scene>& scene);
		static Entity CreateAndPlaceSphere(const Ref<Scene>& scene);
		static Entity CreateAndPlacePlane(const Ref<Scene>& scene);
		static Entity CreateAndPlaceDirectionalLight(const Ref<Scene>& scene);
		static Entity CreateAndPlacePointLight(const Ref<Scene>& scene);
		static Entity CreateAndPlaceSpotLight(const Ref<Scene>& scene);
	};

}
