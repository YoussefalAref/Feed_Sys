import { Link } from 'react-router-dom';
import { addToCart, recordInteraction } from '../services/api';

function ProductCard({ product, user, onCartChange }) {
  const userId = user?.id || 1;

  const handleAddToCart = async () => {
    await addToCart(userId, product.id, 1);
    onCartChange?.(`${product.name} added to cart`);
  };

  const handleBuy = async () => {
    await recordInteraction(userId, product.id, 'purchase');
    onCartChange?.(`Purchase recorded for ${product.name}`);
  };

  return (
    <article className="product-card">
      <div className="product-image-wrap">
        <img className="product-image" src={product.image} alt={product.name} />
      </div>
      <div className="product-content">
        <div className="card-meta">
          <span>{product.category}</span>
          <span>{product.popularity_score}% popular</span>
        </div>
        <h3 className="product-title">{product.name}</h3>
        <p className="product-description">{product.description}</p>
        <p className="product-price">EGP {product.price.toLocaleString()}</p>
        <div className="card-actions">
          <Link className="btn btn-ghost" to={`/products/${product.id}`}>
            View
          </Link>
          <button className="btn btn-light" type="button" onClick={handleAddToCart}>
            Add
          </button>
          <button className="btn btn-primary" type="button" onClick={handleBuy}>
            Buy
          </button>
        </div>
      </div>
    </article>
  );
}

export default ProductCard;
