import { NavLink, Link, useNavigate } from 'react-router-dom';
import { getCurrentUser, logout } from '../services/api';

function Navbar() {
  const navigate = useNavigate();
  const user = getCurrentUser();

  const handleLogout = async () => {
    await logout();
    navigate('/login');
  };

  return (
    <header className="navbar">
      <div className="nav-inner">
        <Link className="brand-link" to="/">
          <span className="logo-mark">B</span>
          <span className="brand-text">BiteApple</span>
        </Link>

        <nav className="nav-links">
          <NavLink className="nav-link" to="/">
            Home
          </NavLink>
          <NavLink className="nav-link" to="/cart">
            Cart
          </NavLink>
          {user ? (
            <>
              <span className="user-pill">{user.name}</span>
              <button className="btn btn-ghost" type="button" onClick={handleLogout}>
                Logout
              </button>
            </>
          ) : (
            <>
              <NavLink className="nav-link" to="/login">
                Login
              </NavLink>
              <NavLink className="btn btn-primary" to="/signup">
                Signup
              </NavLink>
            </>
          )}
        </nav>
      </div>
    </header>
  );
}

export default Navbar;
