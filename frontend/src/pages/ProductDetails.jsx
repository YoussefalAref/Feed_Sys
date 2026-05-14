import { useEffect, useState } from 'react';
import { Link, useParams } from 'react-router-dom';
import ProductCard from '../components/ProductCard';
import ProductImage from '../components/ProductImage';
import {
  addToCart,
  getCurrentUser,
  getItem,
  getRelatedProducts,
  recordInteraction,
} from '../services/api';
import { useCart } from '../context/CartContext';

function ProductDetails() {
  const { itemId } = useParams();
  const [product, setProduct] = useState(null);
  const [related, setRelated] = useState([]);
  const [error, setError] = useState('');
  const [message, setMessage] = useState('');
  const user = getCurrentUser() || { id: 1, name: 'Guest' };
  const { refreshCart, openDrawer } = useCart();

  useEffect(() => {
    async function loadProduct() {
      try {
        const [item, relatedItems] = await Promise.all([
          getItem(itemId),
          getRelatedProducts(itemId),
        ]);
        setProduct(item);
        setRelated(relatedItems);
        await recordInteraction(user.id, itemId, 'view');
      } catch (err) {
        setError(err.message || 'Product not found.');
      }
    }

    loadProduct();
  }, [itemId, user.id]);

  const handleAddToCart = async () => {
    await addToCart(user.id, product.id, 1);
    await refreshCart();
    setMessage('Added to cart');
    openDrawer();
  };

  const handleBuyNow = async () => {
    await addToCart(user.id, product.id, 1);
    await refreshCart();
    openDrawer();
  };

  if (error) {
    return (
      <div className="page empty-state">
        <h1>{error}</h1>
        <Link className="btn btn-primary" to="/">
          Back Home
        </Link>
      </div>
    );
  }

  if (!product) {
    return <div className="page empty-state">Loading product details...</div>;
  }

  const inStock = product.stock == null || product.stock > 0;

  return (
    <div className="page details-page">
      {/* ── Product hero ── */}
      <section className="details-layout">
        <ProductImage className="details-image" product={product} />
        <div className="details-content">
          <span className="category-pill">{product.category}</span>
          <h1>{product.name}</h1>
          <p className="details-price">EGP {product.price.toLocaleString()}</p>
          <p>{product.description}</p>
          <div className="details-metrics">
            <span>🔥 {product.popularity_score} popularity</span>
            <span>
              {inStock
                ? `${product.stock} in stock`
                : <strong className="out-of-stock-text">Out of stock</strong>}
            </span>
          </div>
          <div className="details-actions">
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
          {message && <p className="success-text">{message}</p>}
        </div>
      </section>

      {/* ── Related Products ── */}
      {related.length > 0 && (
        <section className="section-block">
          <h2 className="section-heading">Related Products</h2>
          <div className="recommendation-row">
            {related.map((item) => (
              <ProductCard key={item.id} product={item} user={user} />
            ))}
          </div>
        </section>
      )}
    </div>
  );
}

export default ProductDetails;
