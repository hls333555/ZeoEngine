#pragma once

#include <glm/glm.hpp>

#include <rttr/type>
#include <rttr/registration>
#include <rttr/registration_friend>

#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

#define WORLD_UP_VECTOR glm::vec2({ 0.0f, 1.0f })
#define WORLD_RIGHT_VECTOR glm::vec2({ 1.0f, 0.0f })

	struct Transform
	{
		Transform() = default;
		Transform(const glm::vec3& _position)
			: position(_position)
		{}

		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		float rotation = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		RTTR_ENABLE()
	};

	enum class ObjectCollisionType
	{
		None,
		Box,
		Sphere,
	};

	struct CollisionData
	{
		virtual void UpdateData() = 0;

		ObjectCollisionType collisionType;
		glm::vec2 center = { 0.0f, 0.0f };
		class GameObject* ownerObject;
	};

	struct BoxCollisionData : public CollisionData
	{
	public:
		BoxCollisionData(GameObject* _ownerObject, const glm::vec2& _centerOffset, const glm::vec2& _extentsMultiplier)
			: centerOffset(_centerOffset)
			, extentsMultiplier(_extentsMultiplier)
		{
			collisionType = ObjectCollisionType::Box;
			ownerObject = _ownerObject;
			UpdateData();
		}

		virtual void UpdateData() override;

		glm::vec2 extents;
	private:
		glm::vec2 centerOffset;
		glm::vec2 extentsMultiplier;
	};

	struct SphereCollisionData : public CollisionData
	{
	public:
		SphereCollisionData(GameObject* _ownerObject, const glm::vec2& _centerOffset, float _radiusMultiplier)
			: centerOffset(_centerOffset)
			, radiusMultiplier(_radiusMultiplier)
		{
			collisionType = ObjectCollisionType::Sphere;
			ownerObject = _ownerObject;
			UpdateData();
		}

		virtual void UpdateData() override;

		float radius;
	private:
		glm::vec2 centerOffset;
		float radiusMultiplier;
	};

#define Super __super

#define RTTR_SPAWN(className) \
public: static className* SpawnGameObject(const glm::vec3& position)\
		{\
			return ZeoEngine::Level::Get().SpawnGameObject<className>(position);\
		}
#define RTTR_REGISTER(className, ...) RTTR_ENABLE(__VA_ARGS__) RTTR_SPAWN(className)

	enum class ClassMeta
	{
		/** This class is abstract and cannot be instanced to the level */
		Abstract,
		/** Pop up a tooltip text on mouse hovering this class */
		Tooltip,
	};

	enum class PropertyMeta
	{
		/** This property will display under this category in the object property window */
		Category,
		/** Pop up a tooltip text on mouse hovering this property */
		Tooltip,
		/** Defines the min value of numerical property slider */
		Min,
		/** Defines the max value of numerical property slider */
		Max,
		/** Controls how fast it changes when you are dragging a property value, default is 1.0f */
		DragSensitivity,
		/** This property is transient and will not get serialized */
		Transient,
		/**
		 * Used to filter out GameObject instances not derived from this specified class, which will not appear in the dropdown list.
		 * The value (class name) should be the form of const char* or std::string.
		 * NOTE: We ONLY support register GameObject* pointer for now!
		 * e.g.
		 * GameObject* m_Player;
		 *
		 * RTTR_REGISTRATION
		 * {
		 *     rttr::registration::class_<my_struct>("my_struct")
		 *         .property("Player", &TestObject::m_Player)
		 *         (
		 *             rttr::metadata(META_PROP_SUBCLASSOF, "Player")
		 *         );
		 * }
		 */
		SubclassOf,
	};

	// TODO: move some variables and functionalities to components
	/**
	 * Base class for spawnable objects.
	 *
	 * IMPORTANT NOTES:
	 * You should always call ZeoEngine::Level::Get().SpawnGameObject<>(); to spawn one to the level.
	 * You should add the macro RTTR_REGISTER() to the end of every class derived from GameObject class, which you want to instantiate. Also make sure you have included level.h.
	 * You should add the macro RTTR_REGISTRATION_FRIEND to the end of the class derived from GameObject class, in which you want to register protected or private variables directly instead of getters and setters.
	 * If you come across a compile error: fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj, add /bigobj to the CommandLine of project properties.
	 */
	class GameObject
	{
		friend class EditorLayer;
		friend class Level;

	protected:
		GameObject() = default;

		virtual ~GameObject();

		const std::string& GetUniqueName() const { return m_UniqueName; }
		const std::string& GetName() const { return m_Name; }
	private:
		void SetUniqueName(const std::string& uniqueName) { m_UniqueName = uniqueName; }
		void SetName(const std::string& name) { m_Name = name; }
	public:
		GameObject* GetOwner() const { return m_Owner; }
		void SetOwner(GameObject* owner) { m_Owner = owner; }
		bool IsActive() const { return m_bIsActive; }
		void SetActive(bool bIsActive) { m_bIsActive = bIsActive; }
		const Transform& GetTransform() const { return m_Transform; }
		void SetTransform(const Transform& transform) { m_Transform = transform; }
		const glm::vec2 GetPosition2D() const { return { m_Transform.position.x, m_Transform.position.y }; }
		const glm::vec3& GetPosition() const { return  m_Transform.position; }
		void SetPosition2D(const glm::vec2& position)
		{
			m_Transform.position.x = position.x;
			m_Transform.position.y = position.y;
			if (m_CollisionData)
			{
				m_CollisionData->UpdateData();
			}
		}
		/** For most cases, you should use SetPosition2D() for 2D rendering instead. */
		void SetPosition(const glm::vec3& position)
		{
			m_Transform.position = position;
			if (m_CollisionData)
			{
				m_CollisionData->UpdateData();
			}
		}
		/** For draw command, you should use radians as rotation. */
		const float GetRotation(bool bInRadians) const { return bInRadians ? glm::radians(m_Transform.rotation) : m_Transform.rotation; }
		void SetRotation(float rotation) { m_Transform.rotation = rotation; }
		const glm::vec2& GetScale() const { return m_Transform.scale; }
		void SetScale(const glm::vec2& scale)
		{
			m_Transform.scale = scale;
			if (m_CollisionData)
			{
				m_CollisionData->UpdateData();
			}
		}
		void SetScale(float uniformScale) { m_Transform.scale = { uniformScale, uniformScale }; }
		const glm::mat4& GetTransformMatrix() const { return m_TransformMatrix; }
		float GetSpeed() const { return m_Speed; }
		void SetSpeed(float speed) { m_Speed = speed; }
		const glm::vec2& GetVelocity() const { return m_Velocity; }
		ObjectCollisionType GetCollisionType() const { return m_CollisionData ? m_CollisionData->collisionType : ObjectCollisionType::None; }
		bool ShouldGenerateOverlapEvent() const { return m_bGenerateOverlapEvent; }
		void SetGenerateOverlapEvent(bool bGenerate) { m_bGenerateOverlapEvent = bGenerate; }
		bool IsTranslucent() const { return m_bIsTranslucent; }
		void SetTranslucent(bool bIsTranslucent) { m_bIsTranslucent = bIsTranslucent; }

		/**
		 * You should do initialization stuff here including loading the texture and updating IsTranslucent variable.
		 * Putting other gameplay related code here is not recommended.
		 */
		virtual void Init();
		virtual void BeginPlay() {}
		virtual void OnUpdate(DeltaTime dt);
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}

		/**
		 * Get forward vector based on this object's rotation.
		 * @see WORLD_UP_VECTOR
		 */
		const glm::vec2 GetForwardVector() const;
		/**
		 * Get right vector based on this object's rotation.
		 * @see WORLD_RIGHT_VECTOR
		 */
		const glm::vec2 GetRightVector() const;

		float FindLookAtRotation(const glm::vec2& sourcePosition, const glm::vec2& targetPosition);

		void TranslateTo(const glm::vec2& targetPosition);

		glm::vec2 GetRandomPositionInRange(const glm::vec2& center, const glm::vec2& extents);

		/** Reset necessary data, mostly used by ObjectPooler. */
		virtual void Reset();

		void SetBoxCollisionData(const glm::vec2& extentsMultiplier = { 1.0f, 1.0f }, const glm::vec2& centerOffset = { 0.0f, 0.0f });
		void SetSphereCollisionData(float radiusMultiplier = 1.0f, const glm::vec2& centerOffset = { 0.0f, 0.0f });

		bool IsCollisionEnabled() const { return m_CollisionData ? m_CollisionData->collisionType > ObjectCollisionType::None : false; }

		void DoCollisionTest(const std::vector<GameObject*>& objects);

		virtual void OnOverlap(GameObject* other) {}

		virtual void ApplyDamage(float damage, GameObject* target, GameObject* causer, GameObject* instigator);
		virtual void TakeDamage(float damage, GameObject* causer, GameObject* instigator) {}

		void Destroy();
		virtual void OnDestroyed() {}

	private:
		bool CheckCollision(GameObject* other);
		bool CheckCollision_BB(GameObject* other);
		bool CheckCollision_BS(GameObject* boxObject, GameObject* sphereObject);
		bool CheckCollision_SS(GameObject* other);

		/** Called after gizmo manipulation to update the transform property */
		void DecomposeTransformMatrix();
		/** Called after modifying the transform property to update the transform matrix used by gizmo */
		void ReComposeTransformMatrix();

		// TODO: Component system
	private:
		/** The unique name with suffix index number, e.g. "Player_2" */
		std::string m_UniqueName;
		/** The name displayed in the level outline, which can be modified by the user. By default, this is the same as m_UniqueName */
		std::string m_Name;
		GameObject* m_Owner;
		bool m_bIsActive = true;

		Transform m_Transform;
		/** TODO: Do not pass it directly to OpenGL draw call as rotation inside is calculated from degrees while OpenGL prefers radians */
		glm::mat4 m_TransformMatrix;
		/** A scalar indicating how fast this object moves */
		float m_Speed = 0.0f;
		/** A vector representing current velocity of this object  */
		glm::vec2 m_Velocity{ 0.0f, 0.0f };
		CollisionData* m_CollisionData = nullptr;
		bool m_bGenerateOverlapEvent = false;

		bool bPendingDestroy = false;
		GameObject* OverlappedObject = nullptr;
		bool m_bStartMoving = false;
		glm::vec2 m_SourcePosition = { 0.0f, 0.0f }, m_TargetPosition = { 0.0f, 0.0f };
		float m_MovingDistance = 0.0f;
		float m_MovingAlpha = 0.0f;
		glm::vec2 m_LastPosition{ 0.0f, 0.0f };

		bool m_bIsTranslucent = false;

#if WITH_EDITOR
		bool m_bIsSelectedInEditor = false;
#endif

		RTTR_ENABLE()
		RTTR_REGISTRATION_FRIEND
	};

}
