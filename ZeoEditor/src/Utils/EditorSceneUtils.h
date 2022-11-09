#pragma once

#include "Engine/Core/Core.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class EditorCamera;

	class EditorSceneUtils
	{
	public:
		static EditorCamera& GetEditorCamera();
		static Entity CreateAndPlaceEntity(Scene& scene, const std::string& name = "New Entity");
		static Entity CreateAndPlaceCube(Scene& scene);
		static Entity CreateAndPlaceSphere(Scene& scene);
		static Entity CreateAndPlacePlane(Scene& scene);
		static Entity CreateAndPlaceDirectionalLight(Scene& scene);
		static Entity CreateAndPlacePointLight(Scene& scene);
		static Entity CreateAndPlaceSpotLight(Scene& scene);
	};

}
