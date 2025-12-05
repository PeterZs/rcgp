#pragma once

#include "std430.hpp"
#include "scalar.hpp"

namespace layouts {

template <typename T>
using std430 = std430_layout::layout_engine <T>;

template <typename T>
using scalar = scalar_layout::layout_engine <T>;

} // namespace layouts
