import { Link } from 'react-router-dom';
import { useShop } from '../context/ShopContext';

export default function Cart() {
  const { cartItems, cartTotal, removeFromCart, updateQuantity } = useShop();

  return (
    <section className="page cart-page">
      <h1 className="page-title">Your Cart</h1>

      {cartItems.length === 0 ? (
        <div className="empty-state">
          <p>Your cart is empty. Add products from the trending list.</p>
          <Link to="/" className="btn btn-primary">Explore Trending</Link>
        </div>
      ) : (
        <>
          <div className="cart-list">
            {cartItems.map(({ product, quantity }) => (
              <article className="cart-item" key={product.id}>
                <img src={product.image} alt={product.name} className="cart-image" />
                <div className="cart-item-main">
                  <h3>{product.name}</h3>
                  <p>${product.price.toFixed(2)}</p>
                </div>

                <div className="cart-qty">
                  <button
                    type="button"
                    className="qty-btn"
                    onClick={() => updateQuantity(product.id, quantity - 1)}
                  >
                    -
                  </button>
                  <span>{quantity}</span>
                  <button
                    type="button"
                    className="qty-btn"
                    onClick={() => updateQuantity(product.id, quantity + 1)}
                  >
                    +
                  </button>
                </div>

                <button
                  type="button"
                  className="btn btn-ghost remove-btn"
                  onClick={() => removeFromCart(product.id)}
                >
                  Remove
                </button>
              </article>
            ))}
          </div>

          <div className="cart-total-bar">
            <span>Total</span>
            <strong>${cartTotal.toFixed(2)}</strong>
          </div>
        </>
      )}
    </section>
  );
}
