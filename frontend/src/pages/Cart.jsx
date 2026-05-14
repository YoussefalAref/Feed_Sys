import { useEffect, useRef, useState } from 'react';
import { Link } from 'react-router-dom';
import ProductImage from '../components/ProductImage';
import {
  checkout,
  getCart,
  getCurrentUser,
  getPaymentStatus,
  initiatePayment,
  removeFromCart,
  updateCartQuantity,
} from '../services/api';
import { useCart } from '../context/CartContext';

const POLL_INTERVAL_MS = 4000;

function Cart() {
  const [items, setItems]               = useState([]);
  const [message, setMessage]           = useState('');
  const [error, setError]               = useState('');
  const [pendingOrder, setPendingOrder] = useState(null);
  const [iframeUrl, setIframeUrl]       = useState('');
  const [iframeLoading, setIframeLoading] = useState(false);
  const [checkingOut, setCheckingOut]   = useState(false);
  const pollRef = useRef(null);

  const user = getCurrentUser() || { id: 1, name: 'Guest' };
  const { refreshCart } = useCart();

  // ── Cart loading ──────────────────────────────────────────────────────────
  const loadCart = async () => {
    const cartItems = await getCart(user.id);
    setItems(cartItems);
    refreshCart();
  };

  useEffect(() => {
    loadCart();
    return () => clearInterval(pollRef.current);
  }, [user.id]);

  // ── Quantity / remove ─────────────────────────────────────────────────────
  const handleRemove = async (itemId) => {
    await removeFromCart(user.id, itemId);
    await loadCart();
  };

  const handleQuantity = async (itemId, delta, currentQty) => {
    const next = currentQty + delta;
    if (next < 1) {
      await removeFromCart(user.id, itemId);
    } else {
      await updateCartQuantity(user.id, itemId, next);
    }
    await loadCart();
  };

  // ── Auto-poll for payment confirmation ───────────────────────────────────
  const startPolling = (orderId) => {
    clearInterval(pollRef.current);
    pollRef.current = setInterval(async () => {
      try {
        const result = await getPaymentStatus(orderId);
        if (result.payment_status === 'paid') {
          clearInterval(pollRef.current);
          setPendingOrder((cur) => ({ ...cur, ...result }));
          setIframeUrl('');
          setMessage('Payment confirmed! Your order has been fulfilled.');
          await loadCart();
        }
      } catch {
        // ignore transient errors during polling
      }
    }, POLL_INTERVAL_MS);
  };

  // ── Checkout → /pay → iframe ──────────────────────────────────────────────
  const handleCheckout = async () => {
    setCheckingOut(true);
    setError('');
    try {
      // Step A: create our internal Order
      const order = await checkout(user.id);
      setPendingOrder(order);

      // Step B: run the 3-step Paymob flow
      setIframeLoading(true);
      const payResult = await initiatePayment({
        amount: total,
        orderRef: order.paymob_reference,
        firstName: user.name?.split(' ')[0] || 'Customer',
        lastName: user.name?.split(' ').slice(1).join(' ') || 'Customer',
        email: user.email || 'customer@biteapple.test',
      });

      setIframeUrl(payResult.iframe_url);
      setMessage('Complete your payment below.');
      startPolling(order.order_id);
    } catch (err) {
      setError(err.message || 'Checkout failed. Please try again.');
    } finally {
      setCheckingOut(false);
      setIframeLoading(false);
    }
  };

  // ── Manual status refresh ─────────────────────────────────────────────────
  const handleRefreshStatus = async () => {
    if (!pendingOrder) return;
    const result = await getPaymentStatus(pendingOrder.order_id);
    setPendingOrder((cur) => ({ ...cur, ...result }));
    if (result.payment_status === 'paid') {
      clearInterval(pollRef.current);
      setIframeUrl('');
      setMessage('Payment confirmed! Your order has been fulfilled.');
      await loadCart();
    } else {
      setMessage(`Payment status: ${result.payment_status}`);
    }
  };

  const handleCancelPayment = () => {
    clearInterval(pollRef.current);
    setIframeUrl('');
    setPendingOrder(null);
    setMessage('');
  };

  const total = items.reduce((sum, item) => sum + item.product.price * item.quantity, 0);
  const isPaid = pendingOrder?.payment_status === 'paid';

  return (
    <div className="page cart-page">
      <div className="page-heading">
        <h1>Your Cart</h1>
        {items.length > 0 && (
          <p>
            {items.length} item{items.length !== 1 ? 's' : ''} · EGP {total.toLocaleString()}
          </p>
        )}
      </div>

      {error && <p className="form-error">{error}</p>}

      {/* ── Empty cart ── */}
      {items.length === 0 && !pendingOrder ? (
        <div className="empty-state">
          <h2>Your cart is empty</h2>
          <p>{message || 'Add products from the feed to get started.'}</p>
          <Link className="btn btn-primary" to="/">
            Browse Products
          </Link>
        </div>
      ) : (
        <>
          {/* ── Cart items (hidden once paying) ── */}
          {!iframeUrl && items.length > 0 && (
            <>
              <div className="cart-list">
                {items.map((item) => (
                  <article className="cart-item" key={item.item_id}>
                    <ProductImage className="cart-image" product={item.product} />
                    <div className="cart-item-main">
                      <h3>{item.product.name}</h3>
                      <p>EGP {item.product.price.toLocaleString()}</p>
                    </div>
                    <div className="qty-controls">
                      <button
                        className="qty-btn"
                        type="button"
                        onClick={() => handleQuantity(item.item_id, -1, item.quantity)}
                      >
                        −
                      </button>
                      <span className="qty-value">{item.quantity}</span>
                      <button
                        className="qty-btn"
                        type="button"
                        onClick={() => handleQuantity(item.item_id, +1, item.quantity)}
                      >
                        +
                      </button>
                    </div>
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
                <div>
                  <strong>Total</strong>
                  <span style={{ marginLeft: 10 }}>EGP {total.toLocaleString()}</span>
                </div>
                <button
                  className="btn btn-primary"
                  type="button"
                  onClick={handleCheckout}
                  disabled={checkingOut || iframeLoading}
                >
                  {checkingOut || iframeLoading ? 'Preparing payment…' : 'Checkout with Paymob'}
                </button>
              </div>
            </>
          )}

          {message && !iframeUrl && !isPaid && (
            <p className="success-text" style={{ padding: '0 4px' }}>
              {message}
            </p>
          )}

          {/* ── Paymob payment iframe ── */}
          {iframeUrl && !isPaid && (
            <div className="paymob-iframe-wrapper">
              <div className="paymob-iframe-header">
                <div>
                  <h3 className="paymob-iframe-title">Secure Payment via Paymob</h3>
                  {pendingOrder && (
                    <p className="paymob-iframe-ref">
                      Order #{pendingOrder.order_id} · EGP {pendingOrder.total?.toLocaleString()}
                    </p>
                  )}
                </div>
                <button
                  className="btn btn-ghost"
                  type="button"
                  onClick={handleCancelPayment}
                >
                  ✕ Cancel
                </button>
              </div>

              <div className="paymob-iframe-body">
                <div className="paymob-iframe-overlay-hint">
                  <span>🔒 Your payment is processed securely by Paymob (test mode)</span>
                </div>
                <iframe
                  src={iframeUrl}
                  title="Paymob Secure Payment"
                  className="paymob-iframe"
                  allow="payment"
                  sandbox="allow-forms allow-scripts allow-same-origin allow-top-navigation allow-popups"
                />
              </div>

              <div className="paymob-iframe-footer">
                <span className="paymob-polling-hint">
                  Waiting for payment confirmation…
                </span>
                <button
                  className="btn btn-secondary"
                  type="button"
                  onClick={handleRefreshStatus}
                >
                  Check Status
                </button>
              </div>
            </div>
          )}

          {/* ── Payment confirmed ── */}
          {isPaid && (
            <div className="payment-success-panel">
              <div className="payment-success-icon">✓</div>
              <h2>Payment Confirmed</h2>
              <p>
                Order #{pendingOrder.order_id} · Fulfillment:{' '}
                <strong>{pendingOrder.fulfillment_status}</strong>
              </p>
              <p className="payment-ref-label">
                Reference:{' '}
                <code className="payment-ref-code">
                  {pendingOrder.paymob_reference || pendingOrder.payment_reference}
                </code>
              </p>
              <Link className="btn btn-primary" to="/" style={{ marginTop: 8 }}>
                Continue Shopping
              </Link>
            </div>
          )}
        </>
      )}
    </div>
  );
}

export default Cart;
