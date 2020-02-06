#include "TestObject.h"

//RTTR_REGISTRATION
//{
//	using namespace rttr;
//	using namespace ZeoEngine;
//	registration::class_<TestObject>("TestObject")
//		(
//			metadata(ClassMeta::Tooltip, u8"我是一个TestObject类。")
//		)
//		.constructor(&TestObject::SpawnGameObject, policy::ctor::as_raw_ptr)
//		.enumeration<TestObject::Enum>("Enum")
//		(
//			value("Enum1", TestObject::Enum::Enum1),
//			value("Enum2", TestObject::Enum::Enum2),
//			value("Enum3", TestObject::Enum::Enum3)
//		)
//		.enumeration<TestObject::EnumClass>("EnumClass")
//		(
//			value("EnumClass1", TestObject::EnumClass::EnumClass1),
//			value("EnumClass2", TestObject::EnumClass::EnumClass2),
//			value("EnumClass3", TestObject::EnumClass::EnumClass3)
//		)
//		.property("Bool", &TestObject::m_Bool)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个bool。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Int8", &TestObject::m_Int8)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个int8。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Int32", &TestObject::m_Int32)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个int32。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Int64", &TestObject::m_Int64)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个int64。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("UInt8", &TestObject::m_UInt8)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个uint8。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("UInt32", &TestObject::m_UInt32)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个uint32。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("UInt64", &TestObject::m_UInt64)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个uint64。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Float", &TestObject::m_Float)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个float。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Double", &TestObject::m_Double)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个double。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Enum", &TestObject::m_Enum)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个enum。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("EnumClass", &TestObject::m_EnumClass)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个enum class。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("String", &TestObject::m_String)
//		(
//			policy::prop::bind_as_ptr,
//			metadata(PropertyMeta::Tooltip, u8"我是一个string。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Vec2", &TestObject::m_Vec2)
//		(
//			policy::prop::bind_as_ptr,
//			metadata(PropertyMeta::Tooltip, u8"我是一个vec2。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Vec3", &TestObject::m_Vec3)
//		(
//			policy::prop::bind_as_ptr,
//			metadata(PropertyMeta::Tooltip, u8"我是一个vec3。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Color", &TestObject::m_Color)
//		(
//			policy::prop::bind_as_ptr,
//			metadata(PropertyMeta::Tooltip, u8"我是一个color。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("Player", &TestObject::m_GameObject)
//		(
//			metadata(PropertyMeta::Category, "Test"),
//			metadata(PropertyMeta::Tooltip, u8"我是一个Player。"),
//			metadata(PropertyMeta::SubclassOf, "Player")
//		)
//		.property("Struct", &TestObject::m_Struct)
//		(
//			policy::prop::bind_as_ptr,
//			metadata(PropertyMeta::Tooltip, u8"我是一个Struct。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		// TODO: VectorBool issue
//		//.property("VectorBool", &TestObject::m_VectorBool)
//		//(
//		//	metadata(PropertyMeta::Tooltip, u8"我是一个bool vector。"),
//		//	metadata(PropertyMeta::Category, "Test")
//		//)
//		.property("VectorInt8", &TestObject::m_VectorInt8)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个int8 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorInt32", &TestObject::m_VectorInt32)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个int32 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorInt64", &TestObject::m_VectorInt64)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个int64 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorUInt8", &TestObject::m_VectorUInt8)
//			(
//			metadata(PropertyMeta::Tooltip, u8"我是一个uint8 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorUInt32", &TestObject::m_VectorUInt32)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个uint32 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorUInt64", &TestObject::m_VectorUInt64)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个uint64 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorFloat", &TestObject::m_VectorFloat)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个float vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorDouble", &TestObject::m_VectorDouble)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个double vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorEnum", &TestObject::m_VectorEnum)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个enum vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorEnumClass", &TestObject::m_VectorEnumClass)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个enum class vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorString", &TestObject::m_VectorString)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个string vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorVec2", &TestObject::m_VectorVec2)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个vec2 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorVec3", &TestObject::m_VectorVec3)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个vec3 vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorColor", &TestObject::m_VectorColor)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个color vector。"),
//			metadata(PropertyMeta::Category, "Test")
//		)
//		.property("VectorGameObject", &TestObject::m_VectorGameObject)
//		(
//			metadata(PropertyMeta::Tooltip, u8"我是一个GameObject* vector。"),
//			metadata(PropertyMeta::Category, "Test"),
//			metadata(PropertyMeta::SubclassOf, "Enemy")
//		);
//}
