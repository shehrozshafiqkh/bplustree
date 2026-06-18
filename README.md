# B+ Tree Test Assignment

A single-file, templated, in-memory **B+ Tree** in C++11.

- **Assignment:** Test assignment of the Professorship for Data Engineering,
  Otto-Friedrich-University Bamberg —
  <https://www.uni-bamberg.de/en/dt/study/test-assignment/>
  (originally from the Chair for Database Systems, TU Munich —
  <https://db.in.tum.de/teaching/theses/hiwitest/>).
- **Solution:** [`bplustree.cpp`](bplustree.cpp) — implementation + test suite + `main`.

## Build & run

```bash
g++ -std=c++11 bplustree.cpp -o bplustree
./bplustree
```

On Windows (PowerShell, with MinGW-w64 / MSYS2 `g++` on `PATH`):

```powershell
g++ -std=c++11 bplustree.cpp -o bplustree.exe
.\bplustree.exe
```

A passing run prints exactly:

```
=============== TESTS PASSED! ===============
```

## Documentation

See [`DOCUMENTATION.md`](DOCUMENTATION.md) for a full explanation of the
requirements, the data structures, and how each operation (including the
`O(log n)` `split`) works.
