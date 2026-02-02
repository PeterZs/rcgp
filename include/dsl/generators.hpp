#pragma once

#include <string>

#include "instructions.hpp"

namespace rcgp {

std::string generate_assembly(const SharedBlockReference &sbr, bool debug = false);
std::string generate_glsl(const SharedBlockReference &sbr);
// TODO: SPIRV generator

} // namespace rcgp
