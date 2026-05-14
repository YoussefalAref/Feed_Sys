import ProductCard from './ProductCard';

function Feed({ products, user }) {
  if (!products.length) {
    return <div className="empty-state">No products available yet.</div>;
  }

  return (
    <section className="section-block">
      <div className="section-heading">
        <h2>Product Feed</h2>
        <p>Smartly sorted by popularity and personalized to your browsing behavior.</p>
      </div>
      <div className="product-grid">
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

export default Feed;
