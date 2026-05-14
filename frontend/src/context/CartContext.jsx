import { createContext, useCallback, useContext, useEffect, useState } from 'react';
import { getCart, getCurrentUser } from '../services/api';

const CartContext = createContext(null);

export function CartProvider({ children }) {
  const user = getCurrentUser() || { id: 1 };
  const [cartItems, setCartItems] = useState([]);
  const [isDrawerOpen, setIsDrawerOpen] = useState(false);

  const refreshCart = useCallback(async () => {
    try {
      const items = await getCart(user.id);
      setCartItems(items);
    } catch {
      // ignore — user may not exist yet
    }
  }, [user.id]);

  useEffect(() => {
    refreshCart();
  }, [refreshCart]);

  const cartCount = cartItems.reduce((sum, item) => sum + item.quantity, 0);

  return (
    <CartContext.Provider
      value={{
        cartItems,
        cartCount,
        refreshCart,
        isDrawerOpen,
        openDrawer: () => setIsDrawerOpen(true),
        closeDrawer: () => setIsDrawerOpen(false),
      }}
    >
      {children}
    </CartContext.Provider>
  );
}

export function useCart() {
  return useContext(CartContext);
}
