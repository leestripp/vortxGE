# lsAsyncFileLoader — async file loading (Phase 7 M1)

Worker-thread file I/O for the streaming Asset Manager ("Stage A" of
the two-stage pipeline: CPU-side load on workers, GPU upload on the
main thread in M2+).

## API

```cpp
#include "assets/lsAsyncFileLoader.h"

lsAsyncFileLoader loader( 2 );                  // 2 worker threads

// Async — result via future
std::future<lsAsyncFileLoader::LoadResult> fut = loader.Load( "mesh.vbm" );
auto result = fut.get();                        // std::expected<vector<byte>, string>
if( result ) { /* use *result */ }
else         { /* result.error() */ }

// Sync — same code path, for tools/tests
auto sync = lsAsyncFileLoader::LoadFile( "mesh.vbm" );
```

## Semantics

- Results are `std::expected<std::vector<std::byte>, std::string>`
  (C++23 error model for all new asset code — Plan.md Phase 6 step 5)
- Destructor **drains**: queued tasks complete before threads join
- Pool size is fixed at construction; default 1 worker
- Missing/unreadable files return `std::unexpected` with a message —
  never throw, never crash

## Tests

`tests/src/assets/lsAsyncFileLoader_tests.cpp` — sync load (success,
missing, empty), async via future, 8 concurrent loads on a 2-thread
pool, error propagation through futures.

*Added 2026-08-03 (Plan.md Phase 7 M1). See [../index.md](../index.md).*
