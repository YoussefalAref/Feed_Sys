import { Link } from 'react-router-dom';
import { addToCart } from '../services/api';
import { useCart } from '../context/CartContext';
import ProductImage from './ProductImage';

function ProductCard({ product, user, onCartChange }) {
  const userId = user?.id || 1;
  const { refreshCart, openDrawer } = useCart();
  const inStock = product.stock == null || product.stock > 0;

  const handleAddToCart = async () => {
    await addToCart(userId, product.id, 1);
    await refreshCart();
    onCartChange?.(`${product.name} added to cart`);
    openDrawer();
  };

  const handleBuyNow = async () => {
    await addToCart(userId, product.id, 1);
    await refreshCart();
    openDrawer();
  };

  return (
    <article className="product-card">
      <div className="product-image-wrap">
        <ProductImage className="product-image" product={product} />
        {!inStock && <span className="stock-badge">Out of stock</span>}
      </div>
      <div className="product-content">
        <div className="card-meta">
          <span className="card-category">{product.category}</span>
          <span className="pop-indicator" title="Popularity score">
            🔥 {product.popularity_score}
          </span>
        </div>
        <h3 className="product-title">{product.name}</h3>
        <p className="product-description">{product.description}</p>
        <p className="product-price">EGP {product.price.toLocaleString()}</p>
        <div className="card-actions">
          <Link className="btn btn-ghost" to={`/products/${product.id}`}>
            View
          </Link>
          <button
            className="btn btn-light"
            type="button"
            onClick={handleAddToCart}
            disabled={!inStock}
          >
            Add to Cart
          </button>
          <button
            className="btn btn-primary"
            type="button"
            onClick={handleBuyNow}
            disabled={!inStock}
          >
            Buy Now
          </button>
        </div>
      </div>
    </article>
  );
}

export default ProductCard;
