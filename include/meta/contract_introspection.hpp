#pragma once

#include "contract.hpp"
#include "mirror.hpp"

namespace rcgp {

// Contract to resource handle
template <auto &rsrc>
using ResourceTypeFor = ResourceType <contract_base_t <rsrc>>;

template <auto &rsrc>
using DataTypeFor = DataType <contract_base_t <rsrc>>;

template <auto &rsrc>
using DynamicElementTypeFor = DynamicElementType <contract_base_t <rsrc>>;

} // namespace rcgp
