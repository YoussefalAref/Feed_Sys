import ProductCard from './ProductCard';

function RecommendationSection({ products, user, onCartChange }) {
  return (
    <section className="section-block">
      <div className="section-heading">
        <h2>Recommended for You</h2>
        <p>Mock recommendations based on the user profile category and popularity score.</p>
      </div>
      <div className="recommendation-row">
        {products.map((product) => (
          <ProductCard
            key={product.id}
            product={product}
            user={user}
            onCartChange={onCartChange}
          />
        ))}
      </div>
    </section>
  );
}

export default RecommendationSection;
