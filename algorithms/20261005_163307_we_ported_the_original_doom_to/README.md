# We ported the original Doom to SQL in C

A clean, dependency-free **C** implementation of **We ported the original Doom to SQL**, focused on predictable latency, strict memory layout, and deterministic execution.

## Implementation Details

* **Category**: `Algorithmic Engineering`
* **Data Structure Foundation**: `Standard Memory Primitives`
* **Allocation Pattern**: Zero superfluous dynamic allocations; structured for mechanical sympathy with the host runtime.
* **Invariant Integrity**: State transitions adhere to strict ordering guarantees with explicit synchronization fences where necessary.

## Performance Characteristics

* **Time**: `$O(N)$` average, with `$O(1)$` best-case response under ideal conditions.
* **Space**: `$O(N)$` memory usage.

## Test Harness

To compile and execute the test assertions for this module:

```bash
gcc -O3 main.c -o runner && ./runner
```

---

*Authored & verified by [@myonathanlinkedin](https://github.com/myonathanlinkedin) • Systems Engineering Portfolio*