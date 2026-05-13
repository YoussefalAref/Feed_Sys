import { useEffect, useMemo, useState } from 'react';

function buildFallbackImage(product) {
  const name = product?.name || 'Product';
  const category = product?.category || 'Browse';
  const initials = name
    .split(/\s+/)
    .filter(Boolean)
    .slice(0, 2)
    .map((part) => part[0]?.toUpperCase() || '')
    .join('');
  const accent = product?.category ? '#207c51' : '#d93636';
  const svg = `
    <svg xmlns="http://www.w3.org/2000/svg" width="900" height="675" viewBox="0 0 900 675" role="img" aria-label="${name}">
      <defs>
        <linearGradient id="bg" x1="0" y1="0" x2="1" y2="1">
          <stop offset="0%" stop-color="#f7f8f5" />
          <stop offset="100%" stop-color="#e6efe5" />
        </linearGradient>
        <linearGradient id="accent" x1="0" y1="0" x2="1" y2="1">
          <stop offset="0%" stop-color="${accent}" />
          <stop offset="100%" stop-color="#19211f" />
        </linearGradient>
      </defs>
      <rect width="900" height="675" fill="url(#bg)" />
      <circle cx="740" cy="120" r="150" fill="rgba(32, 124, 81, 0.12)" />
      <circle cx="160" cy="560" r="180" fill="rgba(217, 54, 54, 0.10)" />
      <rect x="120" y="150" width="660" height="320" rx="36" fill="#ffffff" stroke="#d9e4d7" />
      <rect x="170" y="210" width="250" height="24" rx="12" fill="#dfe8dd" />
      <rect x="170" y="252" width="380" height="18" rx="9" fill="#edf2eb" />
      <rect x="170" y="284" width="320" height="18" rx="9" fill="#edf2eb" />
      <rect x="170" y="340" width="150" height="150" rx="28" fill="url(#accent)" opacity="0.96" />
      <text x="245" y="430" text-anchor="middle" font-family="Inter, Arial, sans-serif" font-size="66" font-weight="800" fill="#ffffff">${initials}</text>
      <text x="470" y="380" text-anchor="middle" font-family="Inter, Arial, sans-serif" font-size="30" font-weight="700" fill="#19211f">${category}</text>
      <text x="470" y="422" text-anchor="middle" font-family="Inter, Arial, sans-serif" font-size="20" font-weight="500" fill="#657069">Image unavailable</text>
    </svg>
  `;

  return `data:image/svg+xml;charset=UTF-8,${encodeURIComponent(svg)}`;
}

function ProductImage({ product, className, ...imgProps }) {
  const fallbackSrc = useMemo(() => buildFallbackImage(product), [product]);
  const resolvedSrc = product?.image?.trim() || fallbackSrc;
  const [src, setSrc] = useState(resolvedSrc);

  useEffect(() => {
    setSrc(resolvedSrc);
  }, [resolvedSrc]);

  return (
    <img
      className={className}
      src={src}
      alt={product?.name || 'Product'}
      onError={() => setSrc(fallbackSrc)}
      {...imgProps}
    />
  );
}

export default ProductImage;