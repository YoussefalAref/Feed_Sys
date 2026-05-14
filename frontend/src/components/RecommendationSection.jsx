import ProductCard from './ProductCard';

function RecommendationSection({ products, user }) {
  if (!products.length) return null;

  return (
    <section className="section-block">
      <div className="section-heading">
        <h2>Recommended for You</h2>
        <p>Personalized picks based on your purchase history and category preferences.</p>
      </div>
      <div className="recommendation-row">
        {products.map((product) => (
          <ProductCard
            key={product.id}
            product={product}
            user={user}
          />
        ))}
      </div>
    </section>
  );
}

export default RecommendationSection;
