# LSM-Tree MemTable and SSTable Flush Compaction Engine

An in-memory reference implementation of **LSM-Tree MemTable and SSTable Flush Compaction Engine** in **C**, adhering to standard library idioms, clean data structures, and assertion test suites.

### Core Highlights
* **Language & Standard**: Modern `C` standard library conventions.
* **Architecture Pattern**: Designed for `Balanced Hierarchical Indexing` using `Node Pointers & Self-Balancing Trees`.
* **Runtime Overhead**: Contiguous memory layouts and standard collections are favored for straightforward iteration and access.
* **Concurrency & Safety**: State transitions follow clear ordering guarantees with explicit validation at each phase.

---

### Complexity Analysis

| Dimension | Bound |
| :--- | :--- |
| **Time (Best Case)** | `O(1)` |
| **Time (Worst Case)** | `O(log N)` |
| **Auxiliary Space** | `O(N)` |

---

### Test Suite Execution

Self-contained verification drivers are embedded directly in `main.c` to validate happy paths, boundary inputs, and invariant preservation.

```bash
gcc -O3 main.c -o runner && ./runner
```

---

> ### 🔬 Module Engineering Origin & Verification Disclaimer
> **Algorithmic Origin & Operational Risk Allocation:**
> - **Autonomous Algorithmic Synthesis:** The implementation of `LSM-Tree MemTable and SSTable Flush Compaction Engine` in `C`, including accompanying unit tests, harnesses, and build scripts, was synthesized via an autonomous AI engineering agent directed by system-level prompt specifications.
> - **Verification Scope:** Correctness assertions are derived solely from deterministic test execution and deterministic test suites. The maintainer governs the testing parameters and algorithmic criteria.
> - **ASSUMPTION OF OPERATIONAL RISK:** THIS MODULE IS DISTRIBUTED AS AN OPEN ALGORITHMIC STUDY. THE MAINTAINER DISCLAIMS ANY WARRANTY OF FITNESS FOR PRODUCTION WORKLOADS. USERS ASSUME COMPLETE LIABILITY FOR VALIDATING MEMORY SAFETY, THREAD SAFETY, CONCURRENCY HAZARDS, AND SECURITY INTEGRITY PRIOR TO PRODUCTION INTEGRATION.

---

*Part of the Polyglot Systems Lab • Maintained by [@myonathanlinkedin](https://github.com/myonathanlinkedin)*