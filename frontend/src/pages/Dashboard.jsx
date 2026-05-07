import { useEffect, useState } from 'react';
import AddProductForm from '../components/AddProductForm';
import InteractionBadge from '../components/InteractionBadge';
import ManagerProductTable from '../components/ManagerProductTable';
import {
  addProduct,
  deleteProduct,
  getDashboardStats,
  getProducts,
  getRecentInteractions,
  updateProduct,
} from '../services/api';

function Dashboard() {
  const [stats, setStats] = useState(null);
  const [products, setProducts] = useState([]);
  const [interactions, setInteractions] = useState([]);
  const [error, setError] = useState('');

  const loadDashboard = async () => {
    try {
      const [statData, productData, interactionData] = await Promise.all([
        getDashboardStats(),
        getProducts(),
        getRecentInteractions(),
      ]);
      setStats(statData);
      setProducts(productData);
      setInteractions(interactionData);
    } catch (err) {
      setError(err.message || 'Dashboard failed to load.');
    }
  };

  useEffect(() => {
    loadDashboard();
  }, []);

  const handleAdd = async (productData) => {
    await addProduct(productData);
    await loadDashboard();
  };

  const handleUpdate = async (itemId, productData) => {
    await updateProduct(itemId, productData);
    await loadDashboard();
  };

  const handleDelete = async (itemId) => {
    await deleteProduct(itemId);
    await loadDashboard();
  };

  return (
    <div className="page dashboard-page">
      <div className="page-heading">
        <h1>Manager Dashboard</h1>
        <p>Product management and business metrics powered by mock API functions.</p>
      </div>
      {error && <p className="form-error">{error}</p>}

      <section className="stats-grid">
        <div className="stat-card">
          <span>Total Products</span>
          <strong>{stats?.totalProducts ?? '-'}</strong>
        </div>
        <div className="stat-card">
          <span>Total Users</span>
          <strong>{stats?.totalUsers ?? '-'}</strong>
        </div>
        <div className="stat-card">
          <span>Total Interactions</span>
          <strong>{stats?.totalInteractions ?? '-'}</strong>
        </div>
        <div className="stat-card">
          <span>Most Popular Category</span>
          <strong>{stats?.mostPopularCategory ?? '-'}</strong>
        </div>
      </section>

      <section className="dashboard-grid">
        <AddProductForm onAdd={handleAdd} />
        <div className="recent-panel">
          <h3>Recent Interactions</h3>
          {interactions.map((interaction) => (
            <div className="interaction-row" key={interaction.id}>
              <InteractionBadge type={interaction.type} />
              <span>User {interaction.user_id}</span>
              <span>Item {interaction.item_id}</span>
            </div>
          ))}
        </div>
      </section>

      <section className="section-block">
        <div className="section-heading">
          <h2>Product Management</h2>
          <p>Edit basic product data or remove products from the mock catalog.</p>
        </div>
        <ManagerProductTable products={products} onUpdate={handleUpdate} onDelete={handleDelete} />
      </section>
    </div>
  );
}

export default Dashboard;
