import { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import ProductImage from '../components/ProductImage';
import {
  checkout,
  getCart,
  getCurrentUser,
  getPaymentStatus,
  markMockPaymentPaid,
  removeFromCart,
} from '../services/api';

function Cart() {
  const [items, setItems] = useState([]);
  const [message, setMessage] = useState('');
  const [pendingOrder, setPendingOrder] = useState(null);
  const user = getCurrentUser() || { id: 1, name: 'Guest' };

  const loadCart = async () => {
    const cartItems = await getCart(user.id);
    setItems(cartItems);
  };

  useEffect(() => {
    loadCart();
  }, [user.id]);

  const handleRemove = async (itemId) => {
    await removeFromCart(user.id, itemId);
    await loadCart();
  };

  const handleCheckout = async () => {
    const result = await checkout(user.id);
    setPendingOrder(result);
    setMessage(`Payment reference: ${result.payment_reference || result.paymob_reference || result.fawry_reference}`);
  };

  const handleRefreshPayment = async () => {
    if (!pendingOrder) return;
    const result = await getPaymentStatus(pendingOrder.order_id);
    setPendingOrder((current) => ({ ...current, ...result }));
    setMessage(`Payment status: ${result.payment_status}`);
    if (result.payment_status === 'paid') {
      await loadCart();
    }
  };

  const handleMockPaid = async () => {
    if (!pendingOrder) return;
    const result = await markMockPaymentPaid(pendingOrder.order_id);
    setPendingOrder((current) => ({
      ...current,
      payment_status: result.payment.payment_status,
      fulfillment_status: result.payment.fulfillment_status,
    }));
    setMessage(`${result.fulfillment.purchased} purchase interaction(s) recorded.`);
    await loadCart();
  };

  const total = items.reduce((sum, item) => sum + item.product.price * item.quantity, 0);

  return (
    <div className="page cart-page">
      <div className="page-heading">
        <h1>Cart</h1>
        <p>Cart data is mock state served through api.js.</p>
      </div>
      {items.length === 0 ? (
        <div className="empty-state">
          <h2>Your cart is empty</h2>
          <p>{message || 'Add products from the feed to test cart behavior.'}</p>
          <Link className="btn btn-primary" to="/">
            Browse Products
          </Link>
        </div>
      ) : (
        <>
          <div className="cart-list">
            {items.map((item) => (
              <article className="cart-item" key={item.item_id}>
                <ProductImage className="cart-image" product={item.product} />
                <div className="cart-item-main">
                  <h3>{item.product.name}</h3>
                  <p>EGP {item.product.price.toLocaleString()}</p>
                </div>
                <div className="cart-qty">Quantity: {item.quantity}</div>
                <button
                  className="btn btn-danger"
                  type="button"
                  onClick={() => handleRemove(item.item_id)}
                >
                  Remove
                </button>
              </article>
            ))}
          </div>
          <div className="cart-total-bar">
            <strong>Total: EGP {total.toLocaleString()}</strong>
            <button className="btn btn-primary" type="button" onClick={handleCheckout}>
              Checkout
            </button>
          </div>
          {pendingOrder && (
            <div className="cart-total-bar">
              <strong>
                Order #{pendingOrder.order_id}: {pendingOrder.payment_status}
              </strong>
              <button className="btn btn-secondary" type="button" onClick={handleRefreshPayment}>
                Refresh Payment
              </button>
              <button className="btn btn-primary" type="button" onClick={handleMockPaid}>
                Confirm Mock Payment
              </button>
            </div>
          )}
        </>
      )}
    </div>
  );
}

export default Cart;
