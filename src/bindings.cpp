#include <../pybind11/pybind11.h>
#include "InteractionManager.h"
#include "RecommendationEngine.h"
#include "DTOs.h"

namespace py = pybind11;

// This macro creates the 'engine' module that Python will import
PYBIND11_MODULE(cpp_core, m) {
    m.doc() = "Cluster 3: Interactions and Recommendations Engine";

    // 1. Expose InteractionManager functions
    m.def("record_interaction", &InteractionManager::record_interaction);
    m.def("get_recent_interactions", &InteractionManager::get_recent_interactions);

    // 2. Expose RecommendationEngine functions
    m.def("get_recommendations", &RecommendationEngine::get_recommendations);
    m.def("get_trending", &RecommendationEngine::get_trending);
    m.def("get_related_products", &RecommendationEngine::get_related_products);
}