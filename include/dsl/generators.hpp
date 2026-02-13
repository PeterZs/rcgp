#pragma once

#include <string>

#include "instruction_nodes.hpp"

namespace rcgp {

std::string generate_assembly(
        const SharedBlockReference &sbr,
        bool debug = false,
        bool verbose = false
);

std::string generate_glsl(const SharedBlockReference &sbr);
// TODO: SPIRV generator

} // namespace rcgp
