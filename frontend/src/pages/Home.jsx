import { useEffect, useMemo, useRef, useState } from 'react';
import ProductList from '../components/ProductList';
import Toast from '../components/Toast';
import { useShop } from '../context/ShopContext';
import { products } from '../data/products';
import { sortByPopularityWithHeap } from '../utils/heapSort';

export default function Home() {
  const { addToCart } = useShop();
  const [loading, setLoading] = useState(true);
  const [toast, setToast] = useState('');
  const toastTimerRef = useRef(null);

  const trendingProducts = useMemo(() => sortByPopularityWithHeap(products), []);

  useEffect(() => {
    const timer = setTimeout(() => setLoading(false), 850);

    return () => {
      clearTimeout(timer);
      if (toastTimerRef.current) {
        clearTimeout(toastTimerRef.current);
      }
    };
  }, []);

  const handleAddToCart = (product) => {
    addToCart(product);
    setToast(`${product.name} added to cart`);

    if (toastTimerRef.current) {
      clearTimeout(toastTimerRef.current);
    }

    toastTimerRef.current = setTimeout(() => {
      setToast('');
    }, 1600);
  };

  return (
    <section className="page home-page">
      <div className="hero">
        <p className="hero-kicker">Precision Recommendations</p>
        <h1>Trending on BiteApple</h1>
        <p className="hero-copy">
          Discover products ranked by global popularity, powered by a heap-inspired feed.
        </p>
      </div>

      <ProductList products={trendingProducts} loading={loading} onAddToCart={handleAddToCart} />
      <Toast message={toast} show={Boolean(toast)} />
    </section>
  );
}
