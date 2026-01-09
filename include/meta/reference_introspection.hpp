#pragma once

#include "reference.hpp"
#include "mirror.hpp"

// Reference to resource handle
template <auto &rsrc>
using ResourceTypeFor = ResourceType <reference_base_t <rsrc>>;

template <auto &rsrc>
using DataTypeFor = DataType <reference_base_t <rsrc>>;

template <auto &rsrc>
using DynamicElementTypeFor = DynamicElementType <reference_base_t <rsrc>>;
