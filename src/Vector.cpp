// Vector method implementations are now inline in Vector.h.
// This file keeps explicit instantiations for common non-pybind11 types.
#include "Vector.h"
#include "Item.h"
#include "DTOs.h"
#include "StateStructs.h"

template class Vector<Item>;
template class Vector<RecommendationDTO>;
template class Vector<RelatedProductDTO>;
template class Vector<ProductDTO>;
template class Vector<InteractionState>;
template class Vector<ProductState>;
template class Vector<UserState>;
template class Vector<CartEntry>;
template class Vector<int>;
template class Vector<std::string>;
template class Vector<InteractionDTO>;
