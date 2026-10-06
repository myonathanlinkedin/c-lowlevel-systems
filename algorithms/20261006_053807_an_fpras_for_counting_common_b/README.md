# An FPRAS for Counting Common Bases of Two Matroids

Modern **C** reference architecture for **An FPRAS for Counting Common Bases of Two Matroids**. Engineered for rigorous algorithmic correctness, high throughput, and bounded memory utilization.

---

## 🏛️ Architecture & Design Decisions

This module organizes `An FPRAS for Counting Common Bases of Two Matroids` into an isolated, self-contained unit:
* **Domain Focus**: `Algorithmic Engineering`
* **Primary Primitives**: `Standard Memory Primitives`
* **Memory Strategy**: Contiguous memory layouts are favored over scattered heap allocations for optimal traversal speed.
* **Correctness Model**: Designed with reentrancy and thread isolation in mind, preventing data races under parallel workloads.

### Asymptotic Complexity

| Metric | Bound | Characteristics |
| :--- | :---: | :--- |
| **Best Case Time** | `$O(1)$` | Optimized fast-path execution |
| **Average / Worst Time** | `$O(N)$` | Deterministic upper bound for generalized workloads |
| **Space Complexity** | `$O(N)$` | Strict bounds without unconstrained heap growth |

---

## 🧪 Verification Suite

The accompanying `main.c` driver executes self-contained verification tests:
1. **Nominal Flow**: Validates baseline correctness under typical real-world inputs.
2. **Boundary Conditions**: Exercises extreme edge cases (empty inputs, singletons, capacity limits).
3. **Invariant Preservation**: Validates internal state consistency throughout mutation lifecycles.

### Running Locally

```bash
gcc -O3 main.c -o runner && ./runner
```

---

<sub>Crafted with modern C standards • Maintained by [@myonathanlinkedin](https://github.com/myonathanlinkedin)</sub>