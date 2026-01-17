#include "dsl/instructions.hpp"

std::string Type::repr() const
{
	vswitch ((*this)) {
	vcase(PrimitiveType): {
		const auto &primitive = as <PrimitiveType> ();
		vswitch (primitive) {
		vcase(bool): return "bool";
		vcase(int32_t): return "i32";
		vcase(uint32_t): return "u32";
		vcase(float): return "f32";
		vcase(VectorType <uint32_t, 2>): return "uvec2";
		vcase(VectorType <uint32_t, 3>): return "uvec3";
		vcase(VectorType <uint32_t, 4>): return "uvec4";
		vcase(VectorType <int32_t, 2>): return "ivec2";
		vcase(VectorType <int32_t, 3>): return "ivec3";
		vcase(VectorType <int32_t, 4>): return "ivec4";
		vcase(VectorType <float, 2>): return "vec2";
		vcase(VectorType <float, 3>): return "vec3";
		vcase(VectorType <float, 4>): return "vec4";
		vcase(MatrixType <int32_t, 2, 2>): return "imat2";
		vcase(MatrixType <int32_t, 3, 3>): return "imat3";
		vcase(MatrixType <int32_t, 4, 4>): return "imat4";
		vcase(MatrixType <uint32_t, 2, 2>): return "umat2";
		vcase(MatrixType <uint32_t, 3, 3>): return "umat3";
		vcase(MatrixType <uint32_t, 4, 4>): return "umat4";
		vcase(MatrixType <float, 2, 2>): return "mat2";
		vcase(MatrixType <float, 3, 3>): return "mat3";
		vcase(MatrixType <float, 4, 4>): return "mat4";
		}
		return "Primitive(unknown)";
	}
	vcase(AggregateType): {
		const auto &agg = as <AggregateType> ();
		std::string repr = "Aggregate(";
		for (size_t i = 0; i < agg.size(); i++) {
			if (i)
				repr += ", ";
			repr += agg[i] ? agg[i]->repr() : "null";
		}
		repr += ")";
		return repr;
	}
	vcase(ArrayType): {
		const auto &array = as <ArrayType> ();
		auto base_repr = array.base ? array.base->repr() : "null";
		return "Array(size=" + std::to_string(array.size) + " base=" + base_repr + ")";
	}
	default:
		break;
	}

	return "Type(unknown)";
}
