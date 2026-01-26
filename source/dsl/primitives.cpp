#include "dsl/scalar.hpp"
#include "dsl/vector.hpp"
#include "dsl/matrix.hpp"

namespace rcgp {

// Explicit instantiations for common DSL primitives.
template class scalar <int32_t>;
template class scalar <uint32_t>;
template class scalar <float>;

template struct vector <int32_t, 2>;
template struct vector <int32_t, 3>;
template struct vector <int32_t, 4>;
template struct vector <uint32_t, 2>;
template struct vector <uint32_t, 3>;
template struct vector <uint32_t, 4>;
template struct vector <float, 2>;
template struct vector <float, 3>;
template struct vector <float, 4>;

template class matrix <int32_t, 2, 2>;
template class matrix <int32_t, 3, 3>;
template class matrix <int32_t, 4, 4>;
template class matrix <uint32_t, 2, 2>;
template class matrix <uint32_t, 3, 3>;
template class matrix <uint32_t, 4, 4>;
template class matrix <float, 2, 2>;
template class matrix <float, 3, 3>;
template class matrix <float, 4, 4>;

} // namespace rcgp
