#include "Test/TestComponent.h"

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

#ifndef DOCTEST_CONFIG_DISABLE
	ZE_REGISTRATION
	{
		ZENUM(TestComponent::TestEnum)
			ZENUM_DATA(TestComponent::TestEnum, TestEnum1)
			ZENUM_DATA(TestComponent::TestEnum, TestEnum2)
			ZENUM_DATA(TestComponent::TestEnum, TestEnum3);

		ZENUM(TestComponent::TestEnumClass)
			ZENUM_DATA(TestComponent::TestEnumClass, TestEnumClass1)
			ZENUM_DATA(TestComponent::TestEnumClass, TestEnumClass2)
			ZENUM_DATA(TestComponent::TestEnumClass, TestEnumClass3);

		ZSTRUCT(TestComponent::TestStruct)
			ZDATA(TestComponent::TestStruct, EnumVar)
			ZDATA(TestComponent::TestStruct, I32Var);

		ZSTRUCT(TestComponent::TestNestedStruct)
			ZDATA(TestComponent::TestNestedStruct, TestStructVar)
			ZDATA(TestComponent::TestNestedStruct, FloatVar);

		ZCOMPONENT(TestComponent, ZPROP(DisplayName, ZTEXT("Test")), ZPROP(Tooltip, ZTEXT("²âÊÔ×é¼þ")))
			ZDATA(TestComponent, BoolVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, Ui8Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, 1))
			ZDATA(TestComponent, Ui32Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, 1))
			ZDATA(TestComponent, Ui64Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, 1))
			ZDATA(TestComponent, I8Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, -1))
			ZDATA(TestComponent, I32Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, -1))
			ZDATA(TestComponent, I64Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, -1))
			ZDATA(TestComponent, FloatVar, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, 1.0f), ZPROP(ClampMax, 360.0f), ZPROP(ClampOnlyDuringDragging))
			ZDATA(TestComponent, DoubleVar, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, 1.0))
			ZDATA(TestComponent, EnumVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, EnumClassVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, StringVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, Vec2Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, 5.0f))
			ZDATA(TestComponent, Vec3Var, ZPROP(Category, ZTEXT("Basic")), ZPROP(ClampMin, 5.0f))
			ZDATA(TestComponent, ColorVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, Texture2DVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, ParticleTemplateVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, TestStructVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA_GETTER_REF(TestComponent, TestNestedStructGetterVar, GetTestNestedStructGetterVar, ZPROP(Category, ZTEXT("Basic")))
			ZDATA_SETTER_GETTER(TestComponent, bShowSequenceContainers, SetShowSequenceContainers, GetShowSequenceContainers, ZPROP(Category, ZTEXT("Basic")))
			ZDATA(TestComponent, BoolVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, Ui8VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, Ui32VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, Ui64VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, I8VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(ClampMin, 1), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, I32VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, I64VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, FloatVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, DoubleVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, EnumVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, EnumClassVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, StringVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, Vec2VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, Vec3VecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, ColorVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, Texture2DVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, ParticleTemplateVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA(TestComponent, TestStructVecVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"))
			ZDATA_GETTER_REF(TestComponent, TestNestedStructVecGetterVar, GetTestNestedStructVecGetterVar, ZPROP(Category, ZTEXT("Sequence Container")), ZPROP(HideCondition, "bShowSequenceContainers == False"));
	}
#endif

}
