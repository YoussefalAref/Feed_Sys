import { Link } from 'react-router-dom';
import { getCurrentUser, removeFromCart } from '../services/api';
import { useCart } from '../context/CartContext';
import ProductImage from './ProductImage';

function CartDrawer() {
  const { cartItems, cartCount, refreshCart, isDrawerOpen, closeDrawer } = useCart();
  const user = getCurrentUser() || { id: 1 };

  if (!isDrawerOpen) return null;

  const total = cartItems.reduce((sum, item) => sum + item.product.price * item.quantity, 0);

  const handleRemove = async (itemId) => {
    await removeFromCart(user.id, itemId);
    await refreshCart();
  };

  return (
    <>
      <div className="drawer-overlay" onClick={closeDrawer} />
      <aside className="cart-drawer">
        <div className="drawer-header">
          <h2 className="drawer-title">
            Cart
            {cartCount > 0 && <span className="drawer-count">{cartCount}</span>}
          </h2>
          <button className="drawer-close" type="button" onClick={closeDrawer} aria-label="Close cart">
            ✕
          </button>
        </div>

        <div className="drawer-body">
          {cartItems.length === 0 ? (
            <div className="drawer-empty">
              <p>Your cart is empty.</p>
              <button className="btn btn-primary full" type="button" onClick={closeDrawer}>
                Keep Shopping
              </button>
            </div>
          ) : (
            <ul className="drawer-items">
              {cartItems.map((item) => (
                <li className="drawer-item" key={item.item_id}>
                  <ProductImage className="drawer-item-img" product={item.product} />
                  <div className="drawer-item-info">
                    <p className="drawer-item-name">{item.product.name}</p>
                    <p className="drawer-item-price">
                      EGP {item.product.price.toLocaleString()} × {item.quantity}
                    </p>
                  </div>
                  <button
                    className="btn btn-danger drawer-remove"
                    type="button"
                    onClick={() => handleRemove(item.item_id)}
                    aria-label="Remove"
                  >
                    ✕
                  </button>
                </li>
              ))}
            </ul>
          )}
        </div>

        {cartItems.length > 0 && (
          <div className="drawer-footer">
            <div className="drawer-total">
              <span>Total</span>
              <strong>EGP {total.toLocaleString()}</strong>
            </div>
            <Link className="btn btn-primary full" to="/cart" onClick={closeDrawer}>
              View Cart &amp; Checkout
            </Link>
            <button className="btn btn-ghost full" type="button" onClick={closeDrawer}>
              Continue Shopping
            </button>
          </div>
        )}
      </aside>
    </>
  );
}

export default CartDrawer;
