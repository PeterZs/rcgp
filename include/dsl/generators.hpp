#pragma once

#include <string>

#include "instructions.hpp"

namespace rcgp {

std::string generate_assembly(const SharedBlockReference &sbr, size_t tabs = 0);
std::string generate_glsl(const SharedBlockReference &sbr, size_t tabs = 0);

} // namespace rcgp
