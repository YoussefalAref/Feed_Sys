import { Navigate, Route, Routes } from 'react-router-dom';
import Navbar from './components/Navbar';
import Home from './pages/Home';
import SignIn from './pages/SignIn';
import SignUp from './pages/SignUp';
import Cart from './components/Cart';
import { useShop } from './context/ShopContext';

function App() {
  const { authUser } = useShop();

  return (
    <div className="app-shell">
      <Navbar />
      <main className="app-main">
        <Routes>
          <Route path="/" element={<Home />} />
          <Route path="/signin" element={<SignIn />} />
          <Route path="/signup" element={<SignUp />} />
          <Route path="/cart" element={<Cart />} />
          <Route
            path="*"
            element={<Navigate to={authUser ? '/' : '/signin'} replace />}
          />
        </Routes>
      </main>
    </div>
  );
}

export default App;
