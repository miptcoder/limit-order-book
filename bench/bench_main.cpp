#include <benchmark/benchmark.h>

static void BM_Noop(benchmark::State &s) {
  int x = 0;
  for (auto _ : s) {
    benchmark::DoNotOptimize(x);
  }
}

BENCHMARK(BM_Noop);
