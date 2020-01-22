#pragma once

#include "ZeoEngine.h"

class Level;

class Player
{
public:
	Player();

	void Init(Level* level);

	void OnUpdate(ZeoEngine::DeltaTime dt);
	void OnRender();
	void OnImGuiRender();

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_ShipTexture;

	glm::vec2 m_Position;
	float m_Velocity;

	Level* m_Level;
};
