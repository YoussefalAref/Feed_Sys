#!/bin/bash

# Replace all remaining range-based for loops with g_cart
sed -i '/for (const auto& \[key, entry\] : g_cart)/,/^[[:space:]]*}[[:space:]]*$/{ 
    s/for (const auto& \[key, entry\] : g_cart) {/Vector<std::string> cartKeys = g_cart.getAllKeys();\n    for (int i = 0; i < cartKeys.getSize(); ++i) {\n        std::string key = cartKeys[i];\n        CartEntry entry = g_cart.search(key);/
}' src/bindings/biteapple_core.cpp

# Replace all remaining range-based for loops with g_products  
sed -i '/for (const auto& \[id, product\] : g_products)/,/^[[:space:]]*}[[:space:]]*$/{ 
    s/for (const auto& \[id, product\] : g_products) {/Vector<int> productIds = g_products.getAllKeys();\n    for (int i = 0; i < productIds.getSize(); ++i) {\n        int id = productIds[i];\n        ProductState* product = g_products.searchPointer(id);\n        if (product) {/
}' src/bindings/biteapple_core.cpp

# Fix .erase() calls to use .remove()
sed -i 's/\.erase(/.remove(/g' src/bindings/biteapple_core.cpp

echo "Fixes applied"
