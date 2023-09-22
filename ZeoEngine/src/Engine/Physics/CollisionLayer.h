#pragma once

namespace ZeoEngine {

	struct CollisionLayer
	{
		U32 LayerID;
		std::string Name;
	};

	struct CollisionGroup
	{
		U32 GroupID;
		std::string Name;
		U32 LayerMask = 0; // Bitfield that contains a set of layers

		bool HasLayer(U32 layerID) const
		{
			return LayerMask & ZE_BIT(layerID);
		}

		void AddLayer(U32 layerID)
		{
			LayerMask |= ZE_BIT(layerID);
		}

		void RemoveLayer(U32 layerID)
		{
			LayerMask &= ~ZE_BIT(layerID);
		}
	};

	class CollisionLayerManager
	{
		friend class PhysicsEngine;

	public:
		static U32 AddLayer(const std::string& name);
		static void RemoveLayer(U32 layerID);
		static I32 GetLayer(const std::string& layerName);
		static std::string GetLayerName(U32 layerID);
		static const auto& GetLayers() { return s_Layers; }
		static bool IsValidLayer(U32 layerID);
		static void SetLayerName(U32 layerID, const std::string& name);
		static void ClearLayers();

		static U32 AddGroup(const std::string& name);
		static void RemoveGroup(U32 groupID);
		static I32 GetGroup(const std::string& groupName);
		static std::string GetGroupName(U32 groupID);
		static const auto& GetGroups() { return s_Groups; }
		static U32 GetGroupLayerMask(U32 groupID);
		static bool IsValidGroup(U32 groupID);
		static void SetGroupName(U32 groupID, const std::string& name);
		static void AddLayerToGroup(U32 groupID, U32 layerID);
		static void RemoveLayerFromGroup(U32 groupID, U32 layerID);
		static void ClearGroups();

	private:
		static void AddDefaultLayers();
		static void AddDefaultGroups();

	private:
		static std::vector<CollisionLayer> s_Layers;
		static std::vector<CollisionGroup> s_Groups;
	};
	
}
