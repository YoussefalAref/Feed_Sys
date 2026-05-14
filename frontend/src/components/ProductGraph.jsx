import { useEffect, useRef, useState } from 'react';
import { useNavigate } from 'react-router-dom';

const W = 760;
const H = 480;
const FOCAL_R = 28;
const NODE_R = 20;
const PADDING = 50;

// ── Category colour palette ──────────────────────────────────────────────────
const CATEGORY_COLORS = {
  Electronics: '#4f7eff',
  Fitness:     '#22c55e',
  Home:        '#f59e0b',
  Fashion:     '#ec4899',
  Kitchen:     '#f97316',
  Books:       '#8b5cf6',
  Gaming:      '#ef4444',
  Stationery:  '#06b6d4',
};
const DEFAULT_COLOR = '#94a3b8';

function categoryColor(cat) {
  return CATEGORY_COLORS[cat] || DEFAULT_COLOR;
}

// ── Force-directed layout (Fruchterman-Reingold, ~80 iterations) ─────────────
function buildLayout(nodes, edges, focalId) {
  const n = nodes.length;
  if (n === 0) return {};

  // seed positions: focal at centre, others on a circle
  const pos = {};
  nodes.forEach((node, i) => {
    if (node.id === focalId) {
      pos[node.id] = { x: W / 2, y: H / 2, vx: 0, vy: 0 };
    } else {
      const angle = (2 * Math.PI * i) / n;
      const r = Math.min(W, H) * 0.3;
      pos[node.id] = {
        x: W / 2 + r * Math.cos(angle),
        y: H / 2 + r * Math.sin(angle),
        vx: 0,
        vy: 0,
      };
    }
  });

  const edgeSet = new Map();
  edges.forEach((e) => {
    edgeSet.set(`${e.source}-${e.target}`, true);
    edgeSet.set(`${e.target}-${e.source}`, true);
  });

  const area = W * H;
  const k = Math.sqrt(area / Math.max(n, 1));
  const iterations = 80;

  for (let iter = 0; iter < iterations; iter++) {
    const temp = (1 - iter / iterations) * (Math.min(W, H) * 0.15);

    // repulsion
    for (let i = 0; i < nodes.length; i++) {
      pos[nodes[i].id].vx = 0;
      pos[nodes[i].id].vy = 0;
    }
    for (let i = 0; i < nodes.length; i++) {
      for (let j = i + 1; j < nodes.length; j++) {
        const ai = nodes[i].id;
        const aj = nodes[j].id;
        let dx = pos[ai].x - pos[aj].x;
        let dy = pos[ai].y - pos[aj].y;
        let d = Math.sqrt(dx * dx + dy * dy) || 0.01;
        const rep = (k * k) / d;
        pos[ai].vx += (dx / d) * rep;
        pos[ai].vy += (dy / d) * rep;
        pos[aj].vx -= (dx / d) * rep;
        pos[aj].vy -= (dy / d) * rep;
      }
    }

    // attraction along edges
    edges.forEach((e) => {
      const a = pos[e.source];
      const b = pos[e.target];
      if (!a || !b) return;
      let dx = b.x - a.x;
      let dy = b.y - a.y;
      let d = Math.sqrt(dx * dx + dy * dy) || 0.01;
      const att = (d * d) / k;
      const fx = (dx / d) * att;
      const fy = (dy / d) * att;
      a.vx += fx;
      a.vy += fy;
      b.vx -= fx;
      b.vy -= fy;
    });

    // pin focal node at centre with a strong spring
    const foc = pos[focalId];
    if (foc) {
      foc.vx += (W / 2 - foc.x) * 2;
      foc.vy += (H / 2 - foc.y) * 2;
    }

    // apply displacement clamped to temp, with boundary
    nodes.forEach((node) => {
      const p = pos[node.id];
      const mag = Math.sqrt(p.vx * p.vx + p.vy * p.vy) || 0.01;
      const disp = Math.min(mag, temp);
      p.x += (p.vx / mag) * disp;
      p.y += (p.vy / mag) * disp;
      p.x = Math.max(PADDING, Math.min(W - PADDING, p.x));
      p.y = Math.max(PADDING, Math.min(H - PADDING, p.y));
    });
  }

  return pos;
}

// ── Component ────────────────────────────────────────────────────────────────
function ProductGraph({ graphData, focalId }) {
  const navigate = useNavigate();
  const svgRef = useRef(null);
  const [pos, setPos] = useState({});
  const [tooltip, setTooltip] = useState(null);
  const [dragging, setDragging] = useState(null);
  const dragOffset = useRef({ x: 0, y: 0 });

  const { nodes = [], edges = [] } = graphData || {};

  useEffect(() => {
    if (nodes.length > 0) {
      setPos(buildLayout(nodes, edges, focalId));
    }
  }, [nodes, edges, focalId]);

  // ── drag ────────────────────────────────────────────────────────────────
  const startDrag = (e, nodeId) => {
    e.preventDefault();
    const svgRect = svgRef.current.getBoundingClientRect();
    const clientX = e.touches ? e.touches[0].clientX : e.clientX;
    const clientY = e.touches ? e.touches[0].clientY : e.clientY;
    const scaleX = W / svgRect.width;
    const scaleY = H / svgRect.height;
    dragOffset.current = {
      x: (clientX - svgRect.left) * scaleX - pos[nodeId].x,
      y: (clientY - svgRect.top) * scaleY - pos[nodeId].y,
    };
    setDragging(nodeId);
  };

  const onMouseMove = (e) => {
    if (dragging == null) return;
    const svgRect = svgRef.current.getBoundingClientRect();
    const clientX = e.touches ? e.touches[0].clientX : e.clientX;
    const clientY = e.touches ? e.touches[0].clientY : e.clientY;
    const scaleX = W / svgRect.width;
    const scaleY = H / svgRect.height;
    const nx = Math.max(PADDING, Math.min(W - PADDING, (clientX - svgRect.left) * scaleX - dragOffset.current.x));
    const ny = Math.max(PADDING, Math.min(H - PADDING, (clientY - svgRect.top) * scaleY - dragOffset.current.y));
    setPos((prev) => ({ ...prev, [dragging]: { ...prev[dragging], x: nx, y: ny } }));
  };

  const stopDrag = () => setDragging(null);

  if (nodes.length === 0) return null;

  // group nodes by category for legend
  const categories = [...new Set(nodes.map((n) => n.category))].sort();

  return (
    <div className="graph-container">
      <div className="graph-legend">
        {categories.map((cat) => (
          <span key={cat} className="legend-chip" style={{ '--chip-color': categoryColor(cat) }}>
            {cat}
          </span>
        ))}
      </div>

      <svg
        ref={svgRef}
        viewBox={`0 0 ${W} ${H}`}
        className="product-graph-svg"
        onMouseMove={onMouseMove}
        onMouseUp={stopDrag}
        onMouseLeave={stopDrag}
        onTouchMove={onMouseMove}
        onTouchEnd={stopDrag}
      >
        {/* ── defs: glow filter ── */}
        <defs>
          <filter id="glow" x="-30%" y="-30%" width="160%" height="160%">
            <feGaussianBlur stdDeviation="3" result="blur" />
            <feMerge>
              <feMergeNode in="blur" />
              <feMergeNode in="SourceGraphic" />
            </feMerge>
          </filter>
        </defs>

        {/* ── edges ── */}
        {edges.map((e, i) => {
          const a = pos[e.source];
          const b = pos[e.target];
          if (!a || !b) return null;
          const isFocalEdge = e.source === focalId || e.target === focalId;
          return (
            <line
              key={i}
              x1={a.x} y1={a.y}
              x2={b.x} y2={b.y}
              stroke={isFocalEdge ? '#94a3b8' : '#cbd5e1'}
              strokeWidth={isFocalEdge ? 2 : 1.2}
              strokeDasharray={isFocalEdge ? '0' : '5 3'}
              opacity={isFocalEdge ? 0.7 : 0.45}
            />
          );
        })}

        {/* ── nodes ── */}
        {nodes.map((node) => {
          const p = pos[node.id];
          if (!p) return null;
          const isFocal = node.id === focalId;
          const r = isFocal ? FOCAL_R : NODE_R;
          const color = categoryColor(node.category);
          const label = node.name.length > 14 ? node.name.slice(0, 13) + '…' : node.name;

          return (
            <g
              key={node.id}
              style={{ cursor: isFocal ? 'grab' : 'pointer' }}
              onMouseDown={(e) => startDrag(e, node.id)}
              onTouchStart={(e) => startDrag(e, node.id)}
              onMouseEnter={() => setTooltip(node)}
              onMouseLeave={() => setTooltip(null)}
              onClick={() => {
                if (dragging == null && node.id !== focalId) {
                  navigate(`/products/${node.id}`);
                }
              }}
            >
              {/* outer glow ring for focal */}
              {isFocal && (
                <circle
                  cx={p.x} cy={p.y} r={r + 8}
                  fill="none"
                  stroke={color}
                  strokeWidth={2}
                  opacity={0.3}
                  filter="url(#glow)"
                />
              )}
              <circle
                cx={p.x} cy={p.y} r={r}
                fill={color}
                fillOpacity={isFocal ? 1 : 0.85}
                stroke="white"
                strokeWidth={isFocal ? 3 : 2}
                filter={isFocal ? 'url(#glow)' : undefined}
              />
              {/* popularity ring */}
              <circle
                cx={p.x} cy={p.y} r={r}
                fill="none"
                stroke="white"
                strokeWidth={2}
                strokeDasharray={`${(node.popularity_score / 100) * 2 * Math.PI * r} ${2 * Math.PI * r}`}
                strokeDashoffset={-Math.PI * r * 0.5}
                opacity={0.35}
              />
              <text
                x={p.x} y={p.y + 4}
                textAnchor="middle"
                fontSize={isFocal ? 10 : 9}
                fontWeight="800"
                fill="white"
                style={{ pointerEvents: 'none', userSelect: 'none' }}
              >
                {label}
              </text>
              {/* popularity score below node */}
              <text
                x={p.x} y={p.y + r + 13}
                textAnchor="middle"
                fontSize={9}
                fill="#64748b"
                fontWeight="700"
                style={{ pointerEvents: 'none', userSelect: 'none' }}
              >
                🔥{node.popularity_score}
              </text>
            </g>
          );
        })}
      </svg>

      {/* ── tooltip ── */}
      {tooltip && (
        <div className="graph-tooltip">
          <strong>{tooltip.name}</strong>
          <span>{tooltip.category}</span>
          <span>EGP {tooltip.price?.toLocaleString()}</span>
          <span>🔥 {tooltip.popularity_score}</span>
          {tooltip.id !== focalId && (
            <button
              className="btn btn-primary"
              style={{ marginTop: 6, padding: '5px 10px', fontSize: '0.8rem' }}
              onClick={() => navigate(`/products/${tooltip.id}`)}
            >
              View →
            </button>
          )}
        </div>
      )}

      <p className="graph-hint">
        Nodes are colour-coded by category. Drag to rearrange · Click a node to view that product.
      </p>
    </div>
  );
}

export default ProductGraph;
