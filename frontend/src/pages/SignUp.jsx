import { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { signup } from '../services/api';

function Signup() {
  const navigate = useNavigate();
  const [form, setForm] = useState({
    name: '',
    email: '',
    password: '',
    category: 'Electronics',
    region: 'Cairo',
  });
  const [error, setError] = useState('');

  const updateField = (event) => {
    const { name, value } = event.target;
    setForm((current) => ({ ...current, [name]: value }));
  };

  const handleSubmit = async (event) => {
    event.preventDefault();
    setError('');

    try {
      const result = await signup(form);
      localStorage.setItem('biteapple_user', JSON.stringify(result.user));
      localStorage.setItem('biteapple_token', result.token);
      navigate('/');
    } catch (err) {
      setError(err.message || 'Signup failed.');
    }
  };

  return (
    <div className="page auth-page">
      <form className="auth-card" onSubmit={handleSubmit}>
        <h1>Signup</h1>
        <p>Create a mock user profile for personalized recommendations.</p>
        <label>
          Name
          <input name="name" value={form.name} onChange={updateField} />
        </label>
        <label>
          Email
          <input name="email" type="email" value={form.email} onChange={updateField} />
        </label>
        <label>
          Password
          <input name="password" type="password" value={form.password} onChange={updateField} />
        </label>
        <div className="form-grid">
          <label>
            Category
            <select name="category" value={form.category} onChange={updateField}>
              <option>Electronics</option>
              <option>Fitness</option>
              <option>Home</option>
              <option>Fashion</option>
              <option>Kitchen</option>
            </select>
          </label>
          <label>
            Region
            <input name="region" value={form.region} onChange={updateField} />
          </label>
        </div>
        {error && <p className="form-error">{error}</p>}
        <button className="btn btn-primary full" type="submit">
          Signup
        </button>
        <p className="auth-footer">
          Already have an account? <Link to="/login">Login</Link>
        </p>
      </form>
    </div>
  );
}

export default Signup;
