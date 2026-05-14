import { Navigate, Route, Routes } from 'react-router-dom';
import { CartProvider } from './context/CartContext';
import CartDrawer from './components/CartDrawer';
import Navbar from './components/Navbar';
import Cart from './pages/Cart';
import Dashboard from './pages/Dashboard';
import Home from './pages/Home';
import Login from './pages/Login';
import NotFound from './pages/NotFound';
import ProductDetails from './pages/ProductDetails';
import Signup from './pages/SignUp';

const isAdminApp = import.meta.env.MODE === 'admin';

function App() {
  if (isAdminApp) {
    return (
      <div className="app-shell admin-app-shell">
        <main className="app-main">
          <Routes>
            <Route path="/" element={<Dashboard />} />
            <Route path="/dashboard" element={<Navigate to="/" replace />} />
            <Route path="*" element={<NotFound />} />
          </Routes>
        </main>
      </div>
    );
  }

  return (
    <CartProvider>
      <div className="app-shell">
        <Navbar />
        <CartDrawer />
        <main className="app-main">
          <Routes>
            <Route path="/" element={<Home />} />
            <Route path="/login" element={<Login />} />
            <Route path="/signup" element={<Signup />} />
            <Route path="/products/:itemId" element={<ProductDetails />} />
            <Route path="/cart" element={<Cart />} />
            <Route path="*" element={<NotFound />} />
          </Routes>
        </main>
      </div>
    </CartProvider>
  );
}

export default App;
