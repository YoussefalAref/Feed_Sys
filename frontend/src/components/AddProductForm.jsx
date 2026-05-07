import { useState } from 'react';

const emptyForm = {
  name: '',
  price: '',
  category: 'Electronics',
  popularity_score: 50,
  image: '',
  description: '',
  stock: 10,
};

function AddProductForm({ onAdd }) {
  const [form, setForm] = useState(emptyForm);
  const [error, setError] = useState('');

  const updateField = (event) => {
    const { name, value } = event.target;
    setForm((current) => ({ ...current, [name]: value }));
  };

  const handleSubmit = async (event) => {
    event.preventDefault();
    setError('');

    if (!form.name || !form.price || !form.category) {
      setError('Name, price, and category are required.');
      return;
    }

    await onAdd({
      ...form,
      image:
        form.image ||
        'https://images.unsplash.com/photo-1542838132-92c53300491e?auto=format&fit=crop&w=900&q=80',
    });
    setForm(emptyForm);
  };

  return (
    <form className="manager-form" onSubmit={handleSubmit}>
      <h3>Add Product</h3>
      <div className="form-grid">
        <label>
          Name
          <input name="name" value={form.name} onChange={updateField} />
        </label>
        <label>
          Price
          <input name="price" type="number" value={form.price} onChange={updateField} />
        </label>
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
          Stock
          <input name="stock" type="number" value={form.stock} onChange={updateField} />
        </label>
      </div>
      <label>
        Image URL
        <input name="image" value={form.image} onChange={updateField} />
      </label>
      <label>
        Description
        <textarea name="description" value={form.description} onChange={updateField} />
      </label>
      {error && <p className="form-error">{error}</p>}
      <button className="btn btn-primary" type="submit">
        Add Product
      </button>
    </form>
  );
}

export default AddProductForm;
