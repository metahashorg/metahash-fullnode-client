#ifndef BENCHMARKS_H_
#define BENCHMARKS_H_

struct BenchmarkInfo {
    long opensslTest;
    long memory;
    long ioTest;
    long ioTest2;
};

BenchmarkInfo getBench();

#endif // BENCHMARKS_H_
