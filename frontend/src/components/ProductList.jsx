import ProductCard from './ProductCard';

function ProductSkeleton() {
  return (
    <div className="product-card skeleton">
      <div className="skeleton-image" />
      <div className="product-content">
        <div className="skeleton-line" />
        <div className="skeleton-line short" />
        <div className="skeleton-line" />
        <div className="skeleton-button" />
      </div>
    </div>
  );
}

export default function ProductList({ products, loading, onAddToCart }) {
  if (loading) {
    return (
      <section className="product-grid" aria-label="Loading products">
        {Array.from({ length: 8 }, (_, index) => (
          <ProductSkeleton key={index} />
        ))}
      </section>
    );
  }

  return (
    <section className="product-grid">
      {products.map((product) => (
        <ProductCard key={product.id} product={product} onAddToCart={onAddToCart} />
      ))}
    </section>
  );
}
