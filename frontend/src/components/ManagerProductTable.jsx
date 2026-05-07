import { useState } from 'react';

function ManagerProductTable({ products, onUpdate, onDelete }) {
  const [editingId, setEditingId] = useState(null);
  const [draft, setDraft] = useState(null);

  const beginEdit = (product) => {
    setEditingId(product.id);
    setDraft(product);
  };

  const updateDraft = (event) => {
    const { name, value } = event.target;
    setDraft((current) => ({ ...current, [name]: value }));
  };

  const saveEdit = async () => {
    await onUpdate(editingId, draft);
    setEditingId(null);
    setDraft(null);
  };

  return (
    <div className="table-wrap">
      <table className="manager-table">
        <thead>
          <tr>
            <th>Name</th>
            <th>Category</th>
            <th>Price</th>
            <th>Popularity</th>
            <th>Stock</th>
            <th>Actions</th>
          </tr>
        </thead>
        <tbody>
          {products.map((product) => {
            const isEditing = editingId === product.id;
            const row = isEditing ? draft : product;

            return (
              <tr key={product.id}>
                <td>
                  {isEditing ? <input name="name" value={row.name} onChange={updateDraft} /> : product.name}
                </td>
                <td>
                  {isEditing ? (
                    <input name="category" value={row.category} onChange={updateDraft} />
                  ) : (
                    product.category
                  )}
                </td>
                <td>
                  {isEditing ? (
                    <input name="price" type="number" value={row.price} onChange={updateDraft} />
                  ) : (
                    `EGP ${product.price.toLocaleString()}`
                  )}
                </td>
                <td>
                  {isEditing ? (
                    <input
                      name="popularity_score"
                      type="number"
                      value={row.popularity_score}
                      onChange={updateDraft}
                    />
                  ) : (
                    product.popularity_score
                  )}
                </td>
                <td>
                  {isEditing ? (
                    <input name="stock" type="number" value={row.stock} onChange={updateDraft} />
                  ) : (
                    product.stock
                  )}
                </td>
                <td className="table-actions">
                  {isEditing ? (
                    <>
                      <button className="btn btn-primary" type="button" onClick={saveEdit}>
                        Save
                      </button>
                      <button className="btn btn-ghost" type="button" onClick={() => setEditingId(null)}>
                        Cancel
                      </button>
                    </>
                  ) : (
                    <>
                      <button className="btn btn-light" type="button" onClick={() => beginEdit(product)}>
                        Edit
                      </button>
                      <button className="btn btn-danger" type="button" onClick={() => onDelete(product.id)}>
                        Delete
                      </button>
                    </>
                  )}
                </td>
              </tr>
            );
          })}
        </tbody>
      </table>
    </div>
  );
}

export default ManagerProductTable;
