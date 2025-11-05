#pragma once

#include "../instructions.hpp"

namespace generators {

struct GLSL {
	const Block &block;

	// TODO: options... (e.g. version)
	
	std::string generate_type(Reference type) {
		return "?";
	}

	std::string generate(size_t tabs = 0) {
		if (block.context.model != ExecutionModel::eAgnostic) {
			std::string result = "#version 460\n";

			// TODO: generate type definitions

			// TODO: generate globals

			// TODO: model specific thread inputs...
			result += "\n";
			for (auto &tin : block.context.thread_inputs) {
				// TODO: this needs to be resolved at pipeline time...
				result += fmt::format("layout (location = {}) in {} lin{};\n",
			  		generate_type(tin.type), tin.argi, tin.argi);
			}

			// TODO: generate functions
			result += "\n";

			result += "void main()\n";
			result += "{\n";
			result += "}";

			return result;
		} else {
			__builtin_trap();
		}
	}
};

} // namespace generators
