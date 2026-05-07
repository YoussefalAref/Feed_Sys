const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || 'http://127.0.0.1:8000';

async function request(path, options = {}) {
  const response = await fetch(`${API_BASE_URL}${path}`, {
    headers: {
      'Content-Type': 'application/json',
      ...(options.headers || {}),
    },
    ...options,
  });

  const contentType = response.headers.get('content-type') || '';
  const data = contentType.includes('application/json') ? await response.json() : null;

  if (!response.ok) {
    const message = data?.detail || data?.message || 'Request failed';
    throw new Error(message);
  }

  return data;
}

function toProductPayload(productData) {
  return {
    name: productData.name,
    price: Number(productData.price),
    category: productData.category,
    stock: Number(productData.stock ?? 0),
    image: productData.image || '',
    description: productData.description || '',
    popularity_score: Number(productData.popularity_score ?? 40),
  };
}

export async function login(email, password) {
  return request('/auth/login', {
    method: 'POST',
    body: JSON.stringify({ email, password }),
  });
}

export async function signup(userData) {
  return request('/auth/signup', {
    method: 'POST',
    body: JSON.stringify(userData),
  });
}

export async function logout() {
  localStorage.removeItem('biteapple_user');
  localStorage.removeItem('biteapple_token');
  return { success: true };
}

export function getCurrentUser() {
  const stored = localStorage.getItem('biteapple_user');
  return stored ? JSON.parse(stored) : null;
}

export async function getProducts() {
  return request('/products');
}

export async function getItem(itemId) {
  return request(`/products/${itemId}`);
}

export async function getItemsByCategory(category) {
  return request(`/products/category/${encodeURIComponent(category)}`);
}

export async function addProduct(productData) {
  return request('/products', {
    method: 'POST',
    body: JSON.stringify(toProductPayload(productData)),
  });
}

export async function updateProduct(itemId, productData) {
  return request(`/products/${itemId}`, {
    method: 'PUT',
    body: JSON.stringify(toProductPayload(productData)),
  });
}

export async function deleteProduct(itemId) {
  return request(`/products/${itemId}`, {
    method: 'DELETE',
  });
}

export async function recordInteraction(userId, itemId, type) {
  return request('/interactions', {
    method: 'POST',
    body: JSON.stringify({
      user_id: Number(userId),
      item_id: Number(itemId),
      type,
    }),
  });
}

export async function getRecentInteractions() {
  return request('/interactions/recent');
}

export async function getRecommendations(userId) {
  return request(`/recommendations/${userId}`);
}

export async function getRelatedProducts(itemId) {
  return request(`/products/${itemId}/related`);
}

export async function getCart(userId) {
  return request(`/users/${userId}/cart`);
}

export async function addToCart(userId, itemId, quantity = 1) {
  return request(`/users/${userId}/cart`, {
    method: 'POST',
    body: JSON.stringify({
      item_id: Number(itemId),
      quantity: Number(quantity),
    }),
  });
}

export async function removeFromCart(userId, itemId) {
  return request(`/users/${userId}/cart/${itemId}`, {
    method: 'DELETE',
  });
}

export async function checkout(userId) {
  return request(`/users/${userId}/checkout`, {
    method: 'POST',
  });
}

export async function getPaymentStatus(orderId) {
  return request(`/orders/${orderId}/payment`);
}

export async function markMockPaymentPaid(orderId) {
  return request(`/payments/orders/${orderId}/mock-paid`, {
    method: 'POST',
  });
}

export async function getDashboardStats() {
  return request('/dashboard/stats');
}
