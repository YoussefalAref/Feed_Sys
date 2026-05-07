import { Link } from 'react-router-dom';

function NotFound() {
  return (
    <div className="page empty-state not-found">
      <h1>Page not found</h1>
      <p>The BiteApple route you opened does not exist in this frontend prototype.</p>
      <Link className="btn btn-primary" to="/">
        Back Home
      </Link>
    </div>
  );
}

export default NotFound;
