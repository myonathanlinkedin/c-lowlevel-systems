# D2K-Bench: Can LLM Agents Turn Expert Designs into Efficient GPU Kernels?

A clean, dependency-free **C** implementation of **D2K-Bench: Can LLM Agents Turn Expert Designs into Efficient GPU Kernels?**, focused on predictable latency, strict memory layout, and deterministic execution.

### Core Highlights
* **Language & Standard**: Modern `C` standard library conventions.
* **Architecture Pattern**: Designed for `Algorithmic Engineering` using `Standard Memory Primitives`.
* **Runtime Overhead**: Contiguous memory layouts are favored over scattered heap allocations for optimal traversal speed.
* **Concurrency & Safety**: State transitions adhere to strict ordering guarantees with explicit synchronization fences where necessary.

---

### Complexity Analysis

| Dimension | Bound |
| :--- | :--- |
| **Time (Best Case)** | `$O(1)$` |
| **Time (Worst Case)** | `$O(N \log N)$` |
| **Auxiliary Space** | `$O(N)$` |

---

### Test Suite Execution

Self-contained verification drivers are embedded directly in `main.c` to validate happy paths, boundary inputs, and invariant preservation.

```bash
gcc -O3 main.c -o runner && ./runner
```

---

*Authored & verified by [@myonathanlinkedin](https://github.com/myonathanlinkedin) • Systems Engineering Portfolio*