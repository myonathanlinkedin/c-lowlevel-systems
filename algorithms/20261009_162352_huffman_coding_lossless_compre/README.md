# Huffman Coding Lossless Compression and Decompression (C)

> An in-memory reference implementation of **Huffman Coding Lossless Compression and Decompression** in **C**, adhering to standard library idioms, clean data structures, and assertion test suites.

## Overview & Mechanics

The implementation focuses on the core mathematical properties of **Huffman Coding Lossless Compression and Decompression**:
* **Data Organization**: Built upon `Lookup Tables & Bitwise Bitvectors` to ensure predictable traversal and storage overhead.
* **Safety Invariants**: Zero external heap dependencies; designed as a pure in-memory algorithmic component.
* **Execution Guarantees**: State transitions follow clear ordering guarantees with explicit validation at each phase.

## Complexity Profile

* **Time Complexity**:
  * Fast Path (Best): `O(N log N)`
  * Generalized (Avg / Worst): `O(N log N)`
* **Space Footprint**: `O(N)` resident heap / stack overhead.

## Verification & Test Scenarios

The test suite in `main.c` validates:
* Standard operational paths against expected outcomes.
* Extreme values and edge inputs to ensure robust failure handling.
* State stability across sequential and repeated operations.

```bash
# Execute local verification runner
gcc -O3 main.c -o runner && ./runner
```

---

*Part of the Polyglot Systems Lab • Maintained by [@myonathanlinkedin](https://github.com/myonathanlinkedin)*