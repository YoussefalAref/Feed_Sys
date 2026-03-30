import { Link, NavLink } from 'react-router-dom';
import { useShop } from '../context/ShopContext';

function BiteAppleLogo() {
  return (
    <svg
      width="34"
      height="34"
      viewBox="0 0 64 64"
      aria-hidden="true"
      className="logo-mark"
    >
      <defs>
        <linearGradient id="appleFill" x1="0" y1="0" x2="1" y2="1">
          <stop offset="0%" stopColor="#ff5b67" />
          <stop offset="100%" stopColor="#d62839" />
        </linearGradient>
      </defs>
      <path
        d="M36 9c-6 2-10 7-11 12 5 0 10-2 13-6 2-3 3-6 3-9-2 0-4 1-5 3z"
        fill="#1f2937"
      />
      <path
        d="M32 18c-13 0-23 9-23 22 0 10 7 19 18 19 4 0 6-2 10-2s6 2 10 2c11 0 18-9 18-19 0-13-10-22-23-22z"
        fill="url(#appleFill)"
      />
      <circle cx="15" cy="30" r="5" fill="#ffffff" opacity="0.95" />
      <circle cx="49" cy="36" r="6" fill="#ffffff" opacity="0.95" />
      <path
        d="M24 25c3-3 7-5 12-5"
        stroke="#ffffff"
        strokeWidth="2"
        strokeLinecap="round"
        opacity="0.75"
      />
    </svg>
  );
}

function CartIcon({ count }) {
  return (
    <div className="cart-icon-wrap" aria-label="Shopping cart">
      <svg width="22" height="22" viewBox="0 0 24 24" fill="none" aria-hidden="true">
        <path
          d="M3 4h2l2.1 9.1A2 2 0 0 0 9.1 15H18a2 2 0 0 0 1.9-1.4L22 7H7"
          stroke="currentColor"
          strokeWidth="1.8"
          strokeLinecap="round"
          strokeLinejoin="round"
        />
        <circle cx="10" cy="20" r="1.7" fill="currentColor" />
        <circle cx="17" cy="20" r="1.7" fill="currentColor" />
      </svg>
      <span className="cart-badge">{count}</span>
    </div>
  );
}

export default function Navbar() {
  const { cartCount, isAuthenticated, authUser, signOut } = useShop();

  return (
    <header className="navbar">
      <div className="nav-inner">
        <Link to="/" className="brand-link">
          <BiteAppleLogo />
          <span className="brand-text">BiteApple</span>
        </Link>

        <nav className="nav-links">
          <NavLink to="/" className="nav-link">Home</NavLink>
          <NavLink to="/cart" className="nav-link nav-cart-link">
            <CartIcon count={cartCount} />
          </NavLink>

          {!isAuthenticated ? (
            <>
              <NavLink to="/signin" className="btn btn-ghost">Sign In</NavLink>
              <NavLink to="/signup" className="btn btn-primary">Sign Up</NavLink>
            </>
          ) : (
            <>
              <span className="user-pill">Hi, {authUser.name}</span>
              <button type="button" onClick={signOut} className="btn btn-ghost">
                Sign Out
              </button>
            </>
          )}
        </nav>
      </div>
    </header>
  );
}
