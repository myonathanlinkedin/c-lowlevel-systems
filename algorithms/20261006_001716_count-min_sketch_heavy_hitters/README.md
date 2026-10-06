# Count-Min Sketch Heavy Hitters Frequency Estimator

A clean, dependency-free **C** reference implementation of **Count-Min Sketch Heavy Hitters Frequency Estimator**, focused on core algorithmic mechanics, clear memory layout, and test verification.

### Core Highlights
* **Language & Standard**: Modern `C` standard library conventions.
* **Architecture Pattern**: Designed for `Algorithmic Engineering` using `Standard Memory Primitives`.
* **Runtime Overhead**: Contiguous memory layouts and standard collections are favored for straightforward iteration and access.
* **Concurrency & Safety**: Execution behavior is validated against nominal workflows and boundary edge cases.

---

### Complexity Analysis

| Dimension | Bound |
| :--- | :--- |
| **Time (Best Case)** | `O(1)` |
| **Time (Worst Case)** | `O(N log N)` |
| **Auxiliary Space** | `O(N)` |

---

### Test Suite Execution

Self-contained verification drivers are embedded directly in `main.c` to validate happy paths, boundary inputs, and invariant preservation.

```bash
gcc -O3 main.c -o runner && ./runner
```

---

*Reference implementation verified by [@myonathanlinkedin](https://github.com/myonathanlinkedin)*
