import ProductCard from './ProductCard';

function Feed({ products, user, onCartChange }) {
  if (!products.length) {
    return <div className="empty-state">No products available yet.</div>;
  }

  return (
    <section className="section-block">
      <div className="section-heading">
        <h2>Product Feed</h2>
        <p>Smartly sorted products ready for browsing, carting, and purchase signals.</p>
      </div>
      <div className="product-grid">
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

export default Feed;
