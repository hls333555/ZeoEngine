#pragma once

namespace ZeoEngine {

	struct PhysicsLayer
	{
		U32 LayerID;
		std::string Name;
		I32 CollidesWith = 0; // Bitfield that contains a set of layer bits with which this layer collides

		bool IsValid() const
		{
			return !Name.empty() && ZE_BIT(LayerID) > 0;
		}

		bool ShouldCollide(U32 otherLayerID) const
		{
			return CollidesWith & ZE_BIT(otherLayerID);
		}

		void SetLayerCollision(U32 otherLayerID, bool bShouldCollide)
		{
			if (bShouldCollide)
			{
				CollidesWith |= ZE_BIT(otherLayerID);
			}
			else
			{
				CollidesWith &= ~ZE_BIT(otherLayerID);
			}
		}
	};

	class PhysicsLayerManager
	{
	public:
		static U32 AddLayer(const std::string& name, bool bDefaultSetCollisions = true);
		static void RemoveLayer(U32 layerID);
		static PhysicsLayer& GetLayer(U32 layerID);
		static PhysicsLayer& GetLayer(const std::string& layerName);

		static void SetLayerName(U32 layerID, const std::string& name);
		static void SetLayerCollision(U32 layerID, U32 otherLayerID, bool bShouldCollide);

		static bool ShouldCollide(U32 layerID, U32 otherLayerID);

		static void ClearLayers();

	private:
		static std::vector<PhysicsLayer> s_Layers;
		static PhysicsLayer s_NullLayer;
	};
	
}
