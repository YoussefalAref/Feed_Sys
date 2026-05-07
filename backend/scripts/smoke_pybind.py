from pathlib import Path
import sys


def load_core_module():
    repo_root = Path(__file__).resolve().parents[2]
    matches = list(repo_root.glob("build/phase6/**/biteapple_core*.pyd"))
    matches.extend(repo_root.glob("build/phase6/**/biteapple_core*.so"))

    if not matches:
        raise RuntimeError(
            "biteapple_core module was not found. Run: "
            "cmake -S . -B build\\phase6 && cmake --build build\\phase6"
        )

    sys.path.insert(0, str(matches[0].parent))
    import biteapple_core

    return biteapple_core


def main():
    core = load_core_module()

    products = [
        {
            "id": 101,
            "name": "AirPods Pro MaxCase",
            "price": 2499,
            "category": "Electronics",
            "popularity_score": 94,
            "stock": 18,
        },
        {
            "id": 102,
            "name": "Smart Fitness Band",
            "price": 1299,
            "category": "Fitness",
            "popularity_score": 87,
            "stock": 31,
        },
        {
            "id": 106,
            "name": "Wireless Keyboard",
            "price": 1650,
            "category": "Electronics",
            "popularity_score": 91,
            "stock": 12,
        },
    ]
    interactions = [
        {"user_id": 1, "item_id": 101, "type": "view", "timestamp": "2026-05-07T10:00:00"},
        {"user_id": 1, "item_id": 106, "type": "purchase", "timestamp": "2026-05-07T10:05:00"},
        {"user_id": 2, "item_id": 102, "type": "cart", "timestamp": "2026-05-07T10:10:00"},
    ]
    user = {"id": 1, "email": "mariam@biteapple.test", "category": "Electronics"}

    top = core.rank_top_products(products, 2)
    related = core.get_related_products(products, 101, 3)
    recent = core.get_recent_interactions(interactions, 2)
    recommendations = core.score_recommendations(products, interactions, user, 2)

    assert [item["id"] for item in top] == [101, 106]
    assert [item["id"] for item in related] == [106]
    assert [event["item_id"] for event in recent] == [102, 106]
    assert recommendations[0]["category"] == "Electronics"

    print("pybind smoke test passed")
    print("top:", top)
    print("related:", related)
    print("recent:", recent)
    print("recommendations:", recommendations)


if __name__ == "__main__":
    main()
