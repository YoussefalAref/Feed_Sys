function heapify(array, length, rootIndex) {
  let largest = rootIndex;
  const left = 2 * rootIndex + 1;
  const right = 2 * rootIndex + 2;

  if (left < length && array[left].popularity > array[largest].popularity) {
    largest = left;
  }

  if (right < length && array[right].popularity > array[largest].popularity) {
    largest = right;
  }

  if (largest !== rootIndex) {
    [array[rootIndex], array[largest]] = [array[largest], array[rootIndex]];
    heapify(array, length, largest);
  }
}

export function sortByPopularityWithHeap(items) {
  const heap = [...items];
  const sorted = [];

  for (let i = Math.floor(heap.length / 2) - 1; i >= 0; i -= 1) {
    heapify(heap, heap.length, i);
  }

  for (let i = heap.length - 1; i >= 0; i -= 1) {
    [heap[0], heap[i]] = [heap[i], heap[0]];
    sorted.unshift(heap[i]);
    heapify(heap, i, 0);
  }

  return sorted;
}
