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

		struct TestStruct
		{
			bool operator==(const TestStruct& other) const
			{
				return EnumVar == other.EnumVar && I32Var == other.I32Var;
			}

			TestEnum EnumVar;
			int32_t I32Var;
		};

		struct TestNestedStruct
		{
			bool operator==(const TestNestedStruct& other) const
			{
				return TestStructVar == other.TestStructVar && FloatVar == other.FloatVar;
			}

			TestStruct TestStructVar;
			float FloatVar;
		};

		TestNestedStruct& GetTestNestedStructGetterVar() { return TestNestedStructGetterVar; }
		bool GetShowSequenceContainers() const { return bShowSequenceContainers; }
		void SetShowSequenceContainers(bool value) { bShowSequenceContainers = value; }
		auto& GetTestNestedStructVecGetterVar() { return TestNestedStructVecGetterVar; }

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
		TestStruct TestStructVar;
		TestNestedStruct TestNestedStructGetterVar;

		bool bShowSequenceContainers;

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
		std::vector<TestEnumClass> EnumClassVecVar;
		std::vector<std::string> StringVecVar;
		std::vector<glm::vec2> Vec2VecVar;
		std::vector<glm::vec3> Vec3VecVar;
		std::vector<glm::vec4> ColorVecVar;
		std::vector<Ref<Texture2D>> Texture2DVecVar;
		std::vector<Ref<ParticleTemplate>> ParticleTemplateVecVar;
		std::vector<TestStruct> TestStructVecVar;
		std::vector<TestNestedStruct> TestNestedStructVecGetterVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

}
