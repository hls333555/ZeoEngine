#include "Test/TestComponent.h"

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

#ifndef DOCTEST_CONFIG_DISABLE
	ZE_REFL_REGISTRATION
	{
		ZE_REFL_ENUM(TestComponent::TestEnum)
			ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum1)
			ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum2)
			ZE_REFL_ENUM_DATA(TestComponent::TestEnum, TestEnum3);

		ZE_REFL_ENUM(TestComponent::TestEnumClass)
			ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass1)
			ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass2)
			ZE_REFL_ENUM_DATA(TestComponent::TestEnumClass, TestEnumClass3);

		ZE_REFL_STRUCT(TestComponent::TestStruct1)
			ZE_REFL_DATA(TestComponent::TestStruct1, EnumVar)
			ZE_REFL_DATA(TestComponent::TestStruct1, I32Var);

		ZE_REFL_STRUCT(TestComponent::TestStruct2)
			ZE_REFL_DATA(TestComponent::TestStruct2, TestStruct1Var)
			ZE_REFL_DATA(TestComponent::TestStruct2, FloatVar);

		ZE_REFL_COMPONENT(TestComponent, ZE_REFL_PROP_PAIR(DisplayName, ZE_TEXT("Test")), ZE_REFL_PROP_PAIR(Tooltip, ZE_TEXT("²âÊÔ×é¼þ")))
			ZE_REFL_DATA(TestComponent, BoolVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, Ui8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
			ZE_REFL_DATA(TestComponent, Ui32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
			ZE_REFL_DATA(TestComponent, Ui64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1))
			ZE_REFL_DATA(TestComponent, I8Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
			ZE_REFL_DATA(TestComponent, I32Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
			ZE_REFL_DATA(TestComponent, I64Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, -1))
			ZE_REFL_DATA(TestComponent, FloatVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1.0f), ZE_REFL_PROP_PAIR(ClampMax, 360.0f), ZE_REFL_PROP(ClampOnlyDuringDragging))
			ZE_REFL_DATA(TestComponent, DoubleVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 1.0))
			ZE_REFL_DATA(TestComponent, EnumVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, EnumClassVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, StringVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, Vec2Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
			ZE_REFL_DATA(TestComponent, Vec3Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")), ZE_REFL_PROP_PAIR(ClampMin, 5.0f))
			ZE_REFL_DATA(TestComponent, ColorVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, Texture2DVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, ParticleTemplateVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, TestStruct1Var, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA_GETTER_REF(TestComponent, TestStruct2GetterVar, GetTestStruct2GetterVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA_SETTER_GETTER(TestComponent, ShowSequenceContainers, SetShowSequenceContainers, GetShowSequenceContainers, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Basic")))
			ZE_REFL_DATA(TestComponent, BoolVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, Ui8VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, Ui32VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, Ui64VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, I8VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(ClampMin, 1), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, I32VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, I64VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, FloatVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, DoubleVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, EnumVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, StringVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, Vec2VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, Vec3VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, ColorVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, Texture2DVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, ParticleTemplateVecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA(TestComponent, TestStruct1VecVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"))
			ZE_REFL_DATA_GETTER_REF(TestComponent, TestStruct2VecGetterVar, GetTestStruct2VecGetterVar, ZE_REFL_PROP_PAIR(Category, ZE_TEXT("Sequence Container")), ZE_REFL_PROP_PAIR(HideCondition, "ShowSequenceContainers == False"));
	}
#endif

}
