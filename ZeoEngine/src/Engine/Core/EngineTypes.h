#pragma once

#include <glm/glm.hpp>

namespace ZeoEngine {

	using U8 = uint8_t;
	using U16 = uint16_t;
	using U32 = uint32_t;
	using U64 = uint64_t;

	using I8 = int8_t;
	using I16 = int16_t;
	using I32 = int32_t;
	using I64 = int64_t;

	using SizeT = size_t;

	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using UVec3 = glm::uvec3;
	using Vec4 = glm::vec4;

	using Mat2 = glm::mat2;
	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;

	using Quat = glm::quat;

	enum class FieldType
	{
		None = 0,
		Bool, I8, U8, I16, U16, I32, U32, I64, U64,
		Float, Double,
		Vec2, Vec3, Vec4,
		Enum, String,
		SeqCon, AssCon,
		Asset, Entity,
	};

}
