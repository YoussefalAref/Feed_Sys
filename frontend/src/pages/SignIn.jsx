import { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { useShop } from '../context/ShopContext';

export default function SignIn() {
  const navigate = useNavigate();
  const { signIn } = useShop();

  const [form, setForm] = useState({ email: '', password: '' });
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  const handleChange = (event) => {
    const { name, value } = event.target;
    setForm((previous) => ({ ...previous, [name]: value }));
  };

  const validate = () => {
    if (!form.email.trim() || !form.password.trim()) {
      return 'Please complete all fields.';
    }

    const emailPattern = /\S+@\S+\.\S+/;
    if (!emailPattern.test(form.email)) {
      return 'Please enter a valid email address.';
    }

    return '';
  };

  const handleSubmit = async (event) => {
    event.preventDefault();
    const validationError = validate();

    if (validationError) {
      setError(validationError);
      return;
    }

    setError('');
    setLoading(true);

    try {
      await signIn(form);
      navigate('/');
    } catch (submitError) {
      setError(submitError.message || 'Unable to sign in.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <section className="page auth-page">
      <form className="auth-card" onSubmit={handleSubmit} noValidate>
        <h1>Sign In</h1>
        <p>Welcome back to BiteApple.</p>

        <label htmlFor="email">Email</label>
        <input
          id="email"
          name="email"
          type="email"
          value={form.email}
          onChange={handleChange}
          placeholder="you@example.com"
        />

        <label htmlFor="password">Password</label>
        <input
          id="password"
          name="password"
          type="password"
          value={form.password}
          onChange={handleChange}
          placeholder="Enter your password"
        />

        {error ? <p className="form-error">{error}</p> : null}

        <button className="btn btn-primary full" type="submit" disabled={loading}>
          {loading ? 'Signing in...' : 'Sign In'}
        </button>

        <p className="auth-footer">
          New here? <Link to="/signup">Create an account</Link>
        </p>
      </form>
    </section>
  );
}
