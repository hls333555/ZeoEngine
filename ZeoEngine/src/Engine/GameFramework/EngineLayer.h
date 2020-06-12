#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	class EngineLayer : public Layer
	{
	public:
		EngineLayer(const std::string& name = "Layer");

		virtual void OnAttach() override;

		virtual OrthographicCamera* GetGameCamera() = 0;

		Texture2DLibrary* GetTexture2DLibrary() { return &m_Texture2DLibrary; }
		ParticleLibrary* GetParticleLibrary() { return &m_ParticleLibrary; }

	private:
		void LoadFont(const char* fontPath, const char* missingChars);

	private:
		Texture2DLibrary m_Texture2DLibrary;
		ParticleLibrary m_ParticleLibrary;

	};

}
