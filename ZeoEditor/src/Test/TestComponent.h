#pragma once

#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

#ifndef DOCTEST_CONFIG_DISABLE
	struct TestComponent : public IComponent
	{
		enum TestEnum
		{
			TestEnum1, TestEnum2, TestEnum3
		};

		enum class TestEnumClass
		{
			TestEnumClass1, TestEnumClass2, TestEnumClass3
		};

		struct TestStruct1
		{
			bool operator==(const TestStruct1& other) const
			{
				return EnumVar == other.EnumVar && I32Var == other.I32Var;
			}

			TestEnum EnumVar;
			int32_t I32Var;
		};

		struct TestStruct2
		{
			bool operator==(const TestStruct2& other) const
			{
				return TestStruct1Var == other.TestStruct1Var && FloatVar == other.FloatVar;
			}

			TestStruct1 TestStruct1Var;
			float FloatVar;
		};

		TestStruct2& GetTestStruct2GetterVar() { return TestStruct2GetterVar; }
		bool GetShowSequenceContainers() const { return ShowSequenceContainers; } void SetShowSequenceContainers(bool value) { ShowSequenceContainers = value; }
		auto& GetTestStruct2VecGetterVar() { return TestStruct2VecGetterVar; }

		bool BoolVar;
		uint8_t Ui8Var;
		uint32_t Ui32Var;
		uint64_t Ui64Var;
		int8_t I8Var;
		int32_t I32Var;
		int64_t I64Var;
		float FloatVar;
		double DoubleVar;
		TestEnum EnumVar;
		TestEnumClass EnumClassVar;
		std::string StringVar;
		glm::vec2 Vec2Var;
		glm::vec3 Vec3Var;
		glm::vec4 ColorVar;
		Ref<Texture2D> Texture2DVar;
		Ref<ParticleTemplate> ParticleTemplateVar;
		TestStruct1 TestStruct1Var;
		TestStruct2 TestStruct2GetterVar;
		bool ShowSequenceContainers;

		std::vector<bool> BoolVecVar;
		std::vector<uint8_t> Ui8VecVar;
		std::vector<uint32_t> Ui32VecVar;
		std::vector<uint64_t> Ui64VecVar;
		std::vector<int8_t> I8VecVar;
		std::vector<int32_t> I32VecVar;
		std::vector<int64_t> I64VecVar;
		std::vector<float> FloatVecVar;
		std::vector<double> DoubleVecVar;
		std::vector<TestEnum> EnumVecVar;
		std::vector<std::string> StringVecVar;
		std::vector<glm::vec2> Vec2VecVar;
		std::vector<glm::vec3> Vec3VecVar;
		std::vector<glm::vec4> ColorVecVar;
		std::vector<Ref<Texture2D>> Texture2DVecVar;
		std::vector<Ref<ParticleTemplate>> ParticleTemplateVecVar;
		std::vector<TestStruct1> TestStruct1VecVar;
		std::vector<TestStruct2> TestStruct2VecGetterVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

}
