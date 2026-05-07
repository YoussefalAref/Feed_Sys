import { useEffect, useState } from 'react';
import { Link, useParams } from 'react-router-dom';
import ProductCard from '../components/ProductCard';
import {
  addToCart,
  getCurrentUser,
  getItem,
  getRelatedProducts,
  recordInteraction,
} from '../services/api';

function ProductDetails() {
  const { itemId } = useParams();
  const [product, setProduct] = useState(null);
  const [related, setRelated] = useState([]);
  const [error, setError] = useState('');
  const [message, setMessage] = useState('');
  const user = getCurrentUser() || { id: 1, name: 'Guest' };

  useEffect(() => {
    async function loadProduct() {
      try {
        const [item, relatedItems] = await Promise.all([getItem(itemId), getRelatedProducts(itemId)]);
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
    setMessage('Added to cart');
  };

  const handleBuy = async () => {
    await recordInteraction(user.id, product.id, 'purchase');
    setMessage('Purchase interaction recorded');
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

  return (
    <div className="page details-page">
      <section className="details-layout">
        <img className="details-image" src={product.image} alt={product.name} />
        <div className="details-content">
          <span className="category-pill">{product.category}</span>
          <h1>{product.name}</h1>
          <p className="details-price">EGP {product.price.toLocaleString()}</p>
          <p>{product.description}</p>
          <div className="details-metrics">
            <span>Popularity: {product.popularity_score}</span>
            <span>Stock: {product.stock}</span>
          </div>
          <div className="details-actions">
            <button className="btn btn-light" type="button" onClick={handleAddToCart}>
              Add to Cart
            </button>
            <button className="btn btn-primary" type="button" onClick={handleBuy}>
              Buy
            </button>
          </div>
          {message && <p className="success-text">{message}</p>}
        </div>
      </section>

      {related.length > 0 && (
        <section className="section-block">
          <div className="section-heading">
            <h2>Related Products</h2>
          </div>
          <div className="recommendation-row">
            {related.map((item) => (
              <ProductCard key={item.id} product={item} user={user} onCartChange={setMessage} />
            ))}
          </div>
        </section>
      )}
    </div>
  );
}

export default ProductDetails;
