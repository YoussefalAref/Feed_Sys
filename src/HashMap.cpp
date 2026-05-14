// HashMap method implementations are now inline in HashMap.h.
// This file keeps explicit instantiations for common non-pybind11 types.
#include "HashMap.h"
#include "Item.h"
#include "User.h"
#include "StateStructs.h"

template class HashMap<int, User>;
template class HashMap<int, Item>;
template class HashMap<std::string, User>;
template class HashMap<int, ProductState>;
template class HashMap<int, UserState>;
template class HashMap<std::string, CartEntry>;
template class HashMap<std::string, double>;
