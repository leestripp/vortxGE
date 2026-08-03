// vortxGE test runner — doctest entry point.
//
// All TEST_CASEs across tests/src/** are auto-registered by doctest;
// this translation unit only provides main(). Run via CTest:
//     cd build && ctest --output-on-failure
// or directly for doctest's full CLI (filters, durations, …):
//     ./tests/tests --test-case="lsMatrix4x4*" --success

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
