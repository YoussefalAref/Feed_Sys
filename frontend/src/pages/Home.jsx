import { useEffect, useState } from 'react';
import Feed from '../components/Feed';
import RecommendationSection from '../components/RecommendationSection';
import { getCurrentUser, getProducts, getRecommendations } from '../services/api';

function Home() {
  const [products, setProducts] = useState([]);
  const [recommendations, setRecommendations] = useState([]);
  const [message, setMessage] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(true);
  const user = getCurrentUser();
  const activeUser = user || { id: 1, name: 'Guest', category: 'Electronics' };

  useEffect(() => {
    async function loadHome() {
      try {
        const [productData, recommendationData] = await Promise.all([
          getProducts(),
          getRecommendations(activeUser.id),
        ]);
        setProducts(productData);
        setRecommendations(recommendationData);
      } catch (err) {
        setError(err.message || 'Could not load the product feed.');
      } finally {
        setLoading(false);
      }
    }

    loadHome();
  }, [activeUser.id]);

  const showMessage = (text) => {
    setMessage(text);
    setTimeout(() => setMessage(''), 2200);
  };

  return (
    <div className="page home-page">
      <section className="hero-panel">
        <div>
          <p className="hero-kicker">Smart e-commerce engine</p>
          <h1>Welcome {activeUser.name}.</h1>
          <p className="hero-copy">
            BiteApple turns clicks, carts, and purchases into a personalized product feed and
            business-ready insight layer.
          </p>
        </div>
        <div className="hero-stat">
          <strong>{products.length}</strong>
          <span>mock products live</span>
        </div>
      </section>

      {error && <p className="form-error">{error}</p>}
      {loading ? (
        <div className="empty-state">Loading product feed...</div>
      ) : (
        <>
          <RecommendationSection
            products={recommendations}
            user={activeUser}
            onCartChange={showMessage}
          />
          <Feed products={products} user={activeUser} onCartChange={showMessage} />
        </>
      )}
      {message && <div className="toast">{message}</div>}
    </div>
  );
}

export default Home;
