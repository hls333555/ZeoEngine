#pragma once

#include "ZeoEngine.h"

#include "Player.h"

class Level
{
public:
	void Init();

	void OnUpdate(ZeoEngine::DeltaTime dt) ;
	void OnRender();
	void OnImGuiRender();

	const glm::vec4& GetLevelBounds() const { return m_LevelBounds; }
	void SetLevelBounds(const glm::vec4& levelBounds) { m_LevelBounds = levelBounds; }

private:
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_CheckerboardTexture;

	/** x: left, y: right, z: bottom, w: top */
	glm::vec4 m_LevelBounds;

	Player m_Player;
};
