function InteractionBadge({ type }) {
  return <span className={`interaction-badge badge-${type}`}>{type}</span>;
}

export default InteractionBadge;
