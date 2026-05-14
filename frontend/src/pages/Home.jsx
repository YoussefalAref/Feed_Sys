import { useEffect, useState } from 'react';
import Feed from '../components/Feed';
import RecommendationSection from '../components/RecommendationSection';
import { getCurrentUser, getProducts, getRecommendations } from '../services/api';

function Home() {
  const [products, setProducts] = useState([]);
  const [recommendations, setRecommendations] = useState([]);
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

  return (
    <div className="page home-page">
      <section className="hero-panel">
        <div>
          <p className="hero-kicker">Smart e-commerce engine</p>
          <h1>Welcome{user ? `, ${user.name}` : ''}.</h1>
          <p className="hero-copy">
            BiteApple turns clicks, carts, and purchases into a personalized product feed and
            business-ready insight layer.
          </p>
        </div>
        <div className="hero-stat">
          <strong>{products.length}</strong>
          <span>products live</span>
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
          />
          <Feed products={products} user={activeUser} />
        </>
      )}
    </div>
  );
}

export default Home;
