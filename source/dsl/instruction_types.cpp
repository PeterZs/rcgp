#include "dsl/instructions.hpp"

namespace rcgp {

std::string Type::repr() const
{
	vswitch ((*this)) {
	vcase(PrimitiveType): {
		const auto &primitive = as <PrimitiveType> ();
		return std::string(rcgp::repr(primitive));
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

} // namespace rcgp
