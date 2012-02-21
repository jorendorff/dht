#include <cstring>
#include <cmath>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <sys/time.h>
#include "tables.h"

using namespace std;

// === Code for measuring speed
//
// Instead of producing a single number, we want to produce several data
// points. Then we'll plot them, and we'll be able to see noise, nonlinearity,
// and any other nonobvious weirdness.

// Run a Test of size n once. Return the elapsed time in seconds.
template <class Test>
double measure_single_run(size_t n)
{
    Test test;
    test.setup(n);

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    test.run(n);
    gettimeofday(&t1, NULL);

    return t1.tv_sec - t0.tv_sec + 1e-6 * (t1.tv_usec - t0.tv_usec);
}

const double min_run_seconds = 0.1;
const double max_run_seconds = 1.0;
const int trials = 10; // can't be 1

// Run several Tests of different sizes. Write results to stdout.
//
// We intentionally don't scale the test size exponentially, because hash
// tables can have nonlinear performance-falls-off-a-cliff points (table
// resizes) that occur at exponentially spaced intervals. We want to make sure
// we don't miss those.
//
template <class Test>
void run_time_trials()
{
    // Estimate how many iterations per second we can do.
    double estimated_speed;
    for (size_t n = 1; ; n *= 2) {
        double dt = measure_single_run<Test>(n);
        if (dt >= min_run_seconds) {
            estimated_speed = n / dt;
            break;
        }
    }
    cout << "\t#estimated speed: " << estimated_speed << endl;

    // Now run trials of increasing size and print the results.
    double total = 0;
    for (int i = 0; i < trials; i++) {
        double target_dt = min_run_seconds + double(i) / (trials - 1) * (max_run_seconds - min_run_seconds);
        size_t n = size_t(ceil(estimated_speed * target_dt));
        double dt = measure_single_run<Test>(n);
        cout << '\t' << n << '\t' << dt << endl;
    }
}


// === Tests

template <class Table>
struct InsertTest {
    Table table;
    void setup(size_t) {}
    void run(size_t n) {
        Key k = 1;
        for (size_t i = 0; i < n; i++) {
            table.set(k, k);
            k = k * 1103515245 + 12345;
        }
    }
};

template <class Table>
struct LookupHitTest {
    Table table;
    size_t errors;

    void setup(size_t n) {
        Key k = 1;
        for (size_t i = 0; i < n; i++) {
            table.set(k, k);
            k = k * 1103515245 + 12345;
        }
        errors = 0;
    }

    void run(size_t n) {
        Key k = 1;
        for (size_t i = 0; i < n; i++) {
            if (table.get(k) != k)
                errors++;
            k = k * 1103515245 + 12345;
        }
    }
};

template <template <class> class Test>
void run_test(const char *name)
{
    cout << name << " OpenTable:" << endl;
    run_time_trials<Test<OpenTable> >();
    cout << endl;

    cout << name << " CloseTable:" << endl;
    run_time_trials<Test<CloseTable> >();
    cout << endl;
}

int main(int argc, const char **argv) {
    if (argc > 1 && (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "-w") == 0)) {
        ByteSizeOption opt = (argv[1][1] == 'm' ? BytesAllocated : BytesWritten);
        OpenTable ht1;
        CloseTable ht2;

        for (int i = 0; i < 100000; i++) {
            cout << i << '\t' << ht1.byte_size(opt) << '\t' << ht2.byte_size(opt) << endl;
            ht1.set(i, i);
            ht2.set(i, i);
        }
        return 0;
    }

    run_test<InsertTest>("InsertTest");
    run_test<LookupHitTest>("LookupHitTest");

    return 0;
}
