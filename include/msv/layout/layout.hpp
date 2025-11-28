#pragma once

#include "std430.hpp"

namespace layouts {

template <typename ... Ts>
using std430 = decltype(std430::layout_engine(sequence <Ts...> ::singleton));

} // namespace layouts

template <typename ... Ts>
using std430_layout_t = layouts::std430 <Ts...>;
