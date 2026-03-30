import { createContext, useContext, useMemo, useState } from 'react';

const ShopContext = createContext(null);

const API_DELAY_MS = 900;

const simulateApi = (handler) =>
  new Promise((resolve, reject) => {
    setTimeout(() => {
      try {
        resolve(handler());
      } catch (error) {
        reject(error);
      }
    }, API_DELAY_MS);
  });

export function ShopProvider({ children }) {
  const [authUser, setAuthUser] = useState(null);
  const [cartItems, setCartItems] = useState([]);

  const cartCount = useMemo(
    () => cartItems.reduce((sum, item) => sum + item.quantity, 0),
    [cartItems]
  );

  const cartTotal = useMemo(
    () => cartItems.reduce((sum, item) => sum + item.product.price * item.quantity, 0),
    [cartItems]
  );

  const addToCart = (product) => {
    setCartItems((previous) => {
      const found = previous.find((entry) => entry.product.id === product.id);
      if (found) {
        return previous.map((entry) =>
          entry.product.id === product.id
            ? { ...entry, quantity: entry.quantity + 1 }
            : entry
        );
      }

      return [...previous, { product, quantity: 1 }];
    });
  };

  const removeFromCart = (productId) => {
    setCartItems((previous) => previous.filter((entry) => entry.product.id !== productId));
  };

  const updateQuantity = (productId, quantity) => {
    if (quantity <= 0) {
      removeFromCart(productId);
      return;
    }

    setCartItems((previous) =>
      previous.map((entry) =>
        entry.product.id === productId ? { ...entry, quantity } : entry
      )
    );
  };

  const signIn = ({ email, password }) =>
    simulateApi(() => {
      if (!email || !password) {
        throw new Error('Email and password are required.');
      }

      const user = {
        name: email.split('@')[0],
        email,
      };

      setAuthUser(user);
      return user;
    });

  const signUp = ({ name, email, password }) =>
    simulateApi(() => {
      if (!name || !email || !password) {
        throw new Error('All fields are required.');
      }

      const user = { name, email };
      setAuthUser(user);
      return user;
    });

  const signOut = () => setAuthUser(null);

  const value = {
    authUser,
    isAuthenticated: Boolean(authUser),
    cartItems,
    cartCount,
    cartTotal,
    addToCart,
    removeFromCart,
    updateQuantity,
    signIn,
    signUp,
    signOut,
  };

  return <ShopContext.Provider value={value}>{children}</ShopContext.Provider>;
}

export function useShop() {
  const context = useContext(ShopContext);

  if (!context) {
    throw new Error('useShop must be used within ShopProvider');
  }

  return context;
}
