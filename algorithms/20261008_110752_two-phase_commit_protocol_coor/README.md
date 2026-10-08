# Two-Phase Commit Protocol Coordinator and Participant State Machine

An in-memory reference implementation of **Two-Phase Commit Protocol Coordinator and Participant State Machine** in **C**, adhering to standard library idioms, clean data structures, and assertion test suites.

### Core Highlights
* **Language & Standard**: Modern `C` standard library conventions.
* **Architecture Pattern**: Designed for `Distributed Consensus & State Machine` using `Append-Only State Log & Version Matrix`.
* **Runtime Overhead**: Contiguous memory layouts and standard collections are favored for straightforward iteration and access.
* **Concurrency & Safety**: State consistency is verified after mutations through assertion test coverage.

---

### Complexity Analysis

| Dimension | Bound |
| :--- | :--- |
| **Time (Best Case)** | `O(1)` |
| **Time (Worst Case)** | `O(N) during sync` |
| **Auxiliary Space** | `O(N) state log` |

---

### Test Suite Execution

Self-contained verification drivers are embedded directly in `main.c` to validate happy paths, boundary inputs, and invariant preservation.

```bash
gcc -O3 main.c -o runner && ./runner
```

---

<sub>Standard C reference implementation • Maintained by [@myonathanlinkedin](https://github.com/myonathanlinkedin)</sub>