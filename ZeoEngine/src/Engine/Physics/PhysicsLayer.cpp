#include "ZEpch.h"
#include "Engine/Physics/PhysicsLayer.h"

namespace ZeoEngine {

	std::vector<PhysicsLayer> PhysicsLayerManager::s_Layers;
	PhysicsLayer PhysicsLayerManager::s_NullLayer{ 0, "NULL", -1 };

	U32 PhysicsLayerManager::AddLayer(const std::string& name, bool bDefaultSetCollisions)
	{
		U32 layerID = static_cast<U32>(s_Layers.size());
		PhysicsLayer layer{ layerID, name, ZE_BIT(layerID) };
		s_Layers.emplace_back(std::move(layer));
		if (bDefaultSetCollisions)
		{
			for (const auto& otherLayer : s_Layers)
			{
				SetLayerCollision(layerID, otherLayer.LayerID, true);
			}
		}

		return layerID;
	}

	void PhysicsLayerManager::RemoveLayer(U32 layerID)
	{
		for (auto& otherLayer : s_Layers)
		{
			if (otherLayer.LayerID == layerID) continue;

			if (otherLayer.ShouldCollide(layerID))
			{
				otherLayer.SetLayerCollision(layerID, false);
			}
		}
		s_Layers.erase(s_Layers.begin() + layerID);
	}

	PhysicsLayer& PhysicsLayerManager::GetLayer(U32 layerID)
	{
		return layerID >= s_Layers.size() ? s_NullLayer : s_Layers[layerID];
	}

	I32 PhysicsLayerManager::GetLayer(const std::string& layerName)
	{
		for (const auto& layer : s_Layers)
		{
			if (layer.Name == layerName)
			{
				return layer.LayerID;
			}
		}

		return -1;
	}

	I32 PhysicsLayerManager::GetLayerMask(const std::vector<std::string>& layerNames)
	{
		I32 mask = 0;
		for (const auto& name : layerNames)
		{
			for (const auto& layer : s_Layers)
			{
				if (layer.Name == name)
				{
					mask |= ZE_BIT(layer.LayerID);
					break;
				}
			}
		}
		return mask;
	}

	void PhysicsLayerManager::SetLayerName(U32 layerID, const std::string& name)
	{
		PhysicsLayer& layer = GetLayer(layerID);
		layer.Name = name;
	}

	void PhysicsLayerManager::SetLayerCollision(U32 layerID, U32 otherLayerID, bool bShouldCollide)
	{
		if (bShouldCollide && ShouldCollide(layerID, otherLayerID)) return;

		PhysicsLayer& layer = GetLayer(layerID);
		PhysicsLayer& otherLayer = GetLayer(otherLayerID);
		layer.SetLayerCollision(otherLayerID, bShouldCollide);
		otherLayer.SetLayerCollision(layerID, bShouldCollide);
	}

	bool PhysicsLayerManager::ShouldCollide(U32 layerID, U32 otherLayerID)
	{
		return GetLayer(layerID).ShouldCollide(otherLayerID);
	}

	void PhysicsLayerManager::ClearLayers()
	{
		s_Layers.clear();
		AddLayer("Default");
	}

}
