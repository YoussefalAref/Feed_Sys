export default function Toast({ message, show }) {
  if (!show) return null;

  return (
    <div className="toast" role="status" aria-live="polite">
      {message}
    </div>
  );
}
