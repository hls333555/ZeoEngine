#pragma once

#include "ZeoEngine.h"

/** A dummy GameObject listing currently supported properties which can be registered. */
class TestObject : public ZeoEngine::GameObject
{
public:
	enum Enum
	{
		Enum1, Enum2, Enum3
	};

	enum EnumClass
	{
		EnumClass1, EnumClass2, EnumClass3
	};

	bool m_Bool;
	int8_t m_Int8 = 0;
	int32_t m_Int32 = 0;
	int64_t m_Int64 = 0;
	uint8_t m_UInt8 = 0;
	uint32_t m_UInt32 = 0;
	uint64_t m_UInt64 = 0;
	float m_Float = 0.0f;
	double m_Double = 0.0;
	Enum m_Enum = Enum1;
	EnumClass m_EnumClass = EnumClass::EnumClass1;
	std::string m_String;
	glm::vec2 m_Vec2;
	glm::vec3 m_Vec3;
	glm::vec4 m_Color;
	GameObject* m_GameObject;
	ZeoEngine::Transform m_Struct;
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_Texture;
	ZeoEngine::ParticleSystem* m_ParticleSystem;

	std::vector<bool> m_VectorBool;
	std::vector<int8_t> m_VectorInt8;
	std::vector<int32_t> m_VectorInt32;
	std::vector<int64_t> m_VectorInt64;
	std::vector<uint8_t> m_VectorUInt8;
	std::vector<uint32_t> m_VectorUInt32;
	std::vector<uint64_t> m_VectorUInt64;
	std::vector<float> m_VectorFloat;
	std::vector<double> m_VectorDouble;
	std::vector<Enum> m_VectorEnum;
	std::vector<EnumClass> m_VectorEnumClass;
	std::vector<std::string> m_VectorString;
	std::vector<glm::vec2> m_VectorVec2;
	std::vector<glm::vec3> m_VectorVec3;
	std::vector<glm::vec4> m_VectorColor;
	std::vector<GameObject*> m_VectorGameObject;
	std::vector<ZeoEngine::Ref<ZeoEngine::Texture2D>> m_VectorTexture;
	std::vector<ZeoEngine::ParticleSystem*> m_VectorParticleSystem;

	RTTR_REGISTER(TestObject, GameObject);
};
