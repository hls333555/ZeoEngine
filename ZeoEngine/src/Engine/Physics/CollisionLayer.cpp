#include "ZEpch.h"
#include "Engine/Physics/CollisionLayer.h"

namespace ZeoEngine {

	std::vector<CollisionLayer> CollisionLayerManager::s_Layers;
	std::vector<CollisionGroup> CollisionLayerManager::s_Groups;

	U32 CollisionLayerManager::AddLayer(const std::string& name)
	{
		const U32 layerID = static_cast<U32>(s_Layers.size());
		CollisionLayer layer{ layerID, name };
		s_Layers.emplace_back(std::move(layer));
		const U32 allGroup = GetGroup("All");
		AddLayerToGroup(allGroup, layerID);
		return layerID;
	}

	void CollisionLayerManager::RemoveLayer(U32 layerID)
	{
		if (!IsValidLayer(layerID)) return;

		for (auto& group : s_Groups)
		{
			group.RemoveLayer(layerID);
		}
		s_Layers.erase(s_Layers.begin() + layerID);
	}

	I32 CollisionLayerManager::GetLayer(const std::string& layerName)
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

	std::string CollisionLayerManager::GetLayerName(U32 layerID)
	{
		if (!IsValidLayer(layerID)) return "";

		return s_Layers[layerID].Name;
	}

	bool CollisionLayerManager::IsValidLayer(U32 layerID)
	{
		return layerID < s_Layers.size();
	}

	void CollisionLayerManager::SetLayerName(U32 layerID, const std::string& name)
	{
		if (!IsValidLayer(layerID)) return;

		s_Layers[layerID].Name = name;
	}

	void CollisionLayerManager::ClearLayers()
	{
		s_Layers.clear();
		AddDefaultLayers();
	}

	U32 CollisionLayerManager::AddGroup(const std::string& name)
	{
		const U32 groupID = static_cast<U32>(s_Groups.size());
		CollisionGroup group{ groupID, name };
		s_Groups.emplace_back(std::move(group));
		return groupID;
	}

	void CollisionLayerManager::RemoveGroup(U32 groupID)
	{
		if (!IsValidGroup(groupID)) return;
		s_Groups.erase(s_Groups.begin() + groupID);
	}

	I32 CollisionLayerManager::GetGroup(const std::string& groupName)
	{
		for (const auto& group : s_Groups)
		{
			if (group.Name == groupName)
			{
				return group.GroupID;
			}
		}

		return -1;
	}

	std::string CollisionLayerManager::GetGroupName(U32 groupID)
	{
		if (!IsValidGroup(groupID)) return "";

		return s_Groups[groupID].Name;
	}

	U32 CollisionLayerManager::GetGroupLayerMask(U32 groupID)
	{
		if (!IsValidGroup(groupID)) return 0;

		return s_Groups[groupID].LayerMask;
	}

	bool CollisionLayerManager::IsValidGroup(U32 groupID)
	{
		return groupID < s_Groups.size();
	}

	void CollisionLayerManager::SetGroupName(U32 groupID, const std::string& name)
	{
		if (!IsValidGroup(groupID)) return;

		s_Groups[groupID].Name = name;
	}

	void CollisionLayerManager::AddLayerToGroup(U32 groupID, U32 layerID)
	{
		if (!IsValidGroup(groupID) || !IsValidLayer(layerID)) return;

		s_Groups[groupID].AddLayer(layerID);
	}

	void CollisionLayerManager::RemoveLayerFromGroup(U32 groupID, U32 layerID)
	{
		if (!IsValidGroup(groupID) || !IsValidLayer(layerID)) return;

		s_Groups[groupID].RemoveLayer(layerID);
	}

	void CollisionLayerManager::ClearGroups()
	{
		s_Groups.clear();
		AddDefaultGroups();
	}

	void CollisionLayerManager::AddDefaultLayers()
	{
		AddLayer("Default");
	}

	void CollisionLayerManager::AddDefaultGroups()
	{
		const U32 allGroup = AddGroup("All");
		for (SizeT i = 0; i < s_Layers.size(); ++i)
		{
			s_Groups[allGroup].AddLayer(static_cast<U32>(i));
		}
		AddGroup("None");
	}

}
