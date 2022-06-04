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
			I32 I32Var;
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
		U8 Ui8Var;
		U32 Ui32Var;
		U64 Ui64Var;
		I8 I8Var;
		I32 I32Var;
		I64 I64Var;
		float FloatVar;
		double DoubleVar;
		TestEnum EnumVar;
		TestEnumClass EnumClassVar;
		std::string StringVar;
		Vec2 Vec2Var;
		Vec3 Vec3Var;
		Vec4 ColorVar;
		AssetHandle<Texture2D> Texture2DVar;
		AssetHandle<ParticleTemplate> ParticleTemplateVar;
		TestStruct TestStructVar;
		TestNestedStruct TestNestedStructGetterVar;

		bool bShowSequenceContainers;

		std::vector<bool> BoolVecVar;
		std::vector<U8> Ui8VecVar;
		std::vector<U32> Ui32VecVar;
		std::vector<U64> Ui64VecVar;
		std::vector<I8> I8VecVar;
		std::vector<I32> I32VecVar;
		std::vector<I64> I64VecVar;
		std::vector<float> FloatVecVar;
		std::vector<double> DoubleVecVar;
		std::vector<TestEnum> EnumVecVar;
		std::vector<TestEnumClass> EnumClassVecVar;
		std::vector<std::string> StringVecVar;
		std::vector<Vec2> Vec2VecVar;
		std::vector<Vec3> Vec3VecVar;
		std::vector<Vec4> ColorVecVar;
		std::vector<AssetHandle<Texture2D>> Texture2DVecVar;
		std::vector<AssetHandle<ParticleTemplate>> ParticleTemplateVecVar;
		std::vector<TestStruct> TestStructVecVar;
		std::vector<TestNestedStruct> TestNestedStructVecGetterVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

}
