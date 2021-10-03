#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;

void qsort_seq(vector<long long>& array, long long l, long long r) {
    long long i = l;
    long long j = r;
    long long q = array[(i + j) / 2];

    while (i <= j)
    {
        while (array[i] < q) {
            i++;
        }
        while (array[j] > q) {
            j--;
        }
        if (i >= j) {
            break;
        }
        swap(array[i], array[j]);
        i++;
        j--;
    }
    if (j > l) {
        qsort_seq(array, l, j);
    }
    if (j + 1 < r) {
        qsort_seq(array, j + 1, r);
    }
}

void qsort_par(vector<long long>& array, long long l, long long r) {
    long long i = l;
    long long j = r;
    long long q = array[(i + j) / 2];

    while (i <= j) {
        while (array[i] < q) {
            i++;
        }
        while (array[j] > q) {
            j--;
        }
        if (i >= j) {
            break;
        }
        swap(array[i], array[j]);
        i++;
        j--;
    }
    if (j > l && j + 1 < r) {
        cilk_spawn qsort_par(array, l, j);
        qsort_par(array, j + 1, r);
        cilk_sync;
    } else if (j > l) {
        qsort_par(array, l, j);
    } else if (j + 1 < r) {
        qsort_par(array, j + 1, r);
    }
}

vector<long long> generate(long long size) {
    srand(0);
    vector<long long> res(size);
    for (long long i = 0; i < size; ++i) {
        res[i] = rand();
    }
    return res;
}

vector<chrono::duration<int64_t, nano>> runtimes_seq, runtimes_par;

void compare_seq_and_par(long long array_size) {
    vector<long long> to_sort_seq = generate(array_size);
    vector<long long> to_sort_par = to_sort_seq;

    // cout << "Start sort seq" << endl;
    auto start_time_seq = std::chrono::high_resolution_clock::now();
    qsort_seq(to_sort_seq, 0, to_sort_seq.size() - 1);
    auto end_time_seq = std::chrono::high_resolution_clock::now();
    auto time_seq = end_time_seq - start_time_seq;
    runtimes_seq.push_back(time_seq);
    cout << "qsort_seq time: " << time_seq / std::chrono::milliseconds(1) << " ms" << endl;

    // cout << "Start sort par" << endl;
    auto start_time_par = std::chrono::high_resolution_clock::now();
    qsort_par(to_sort_par, 0, to_sort_par.size() - 1);
    auto end_time_par = std::chrono::high_resolution_clock::now();
    auto time_par = end_time_par - start_time_par;
    runtimes_par.push_back(time_par);
    cout << "qsort_par time: " << time_par / std::chrono::milliseconds(1) << " ms" << endl;

    cout << "Assert results of sort equal: ";
    bool equal = true;
    for (long long i = 0; i < to_sort_seq.size(); ++i) {
        if (to_sort_seq[i] != to_sort_par[i]) {
            equal = false;
            break;
        }
    }
    cout << (equal ? "Equal" : "Not equal") << endl;
    // if (!equal) {
        // cout << to_sort_seq[0] << endl << to_sort_par[0] << endl;
        // cout << to_sort_seq.size() << endl << to_sort_par.size() << endl;
        // for (long long i = 0; i < to_sort_seq.size(); ++i) {
        //     cout << to_sort_seq[i] << " ";
        // }
        // cout << endl;
        // for (long long i = 0; i < to_sort_par.size(); ++i) {
        //     cout << to_sort_par[i] << " ";
        // }
        // cout << endl;
    // }
}

int main(int argc, char *argv[]) {
    long long runs = 5;
    long long array_size = 1e8;

    if (argc >= 2) {
        runs = stoll(argv[1]);
    }
    if (argc >= 3) {
        array_size = stoll(argv[2]);
    }

    freopen("result.txt", "w", stdout);

    __cilkrts_set_param("nworkers", "4");
    int numWorkers = __cilkrts_get_nworkers();
    cout << "Runs " << runs << endl;
    cout << "Array size " << array_size << endl;
    cout << "Workers " << numWorkers << endl;

    for (long long i = 0; i < runs; ++i) {
        cout << "Test #" << i + 1 << endl;
        compare_seq_and_par(array_size);
        cout << endl;
    }

    chrono::duration<int64_t, nano> time_seq_avg = chrono::duration<int64_t, nano>::zero(), time_par_avg = chrono::duration<int64_t, nano>::zero();
    for (long long i = 0; i < runtimes_seq.size(); ++i) {
        time_seq_avg += runtimes_seq[i];
    }
    time_seq_avg /= runtimes_seq.size();
    cout << "qsort_seq average time: " << time_seq_avg / std::chrono::milliseconds(1) << " ms" << endl;


    for (long long i = 0; i < runtimes_par.size(); ++i) {
        time_par_avg += runtimes_par[i];
    }
    time_par_avg /= runtimes_par.size();
    cout << "qsort_par average time: " << time_par_avg / std::chrono::milliseconds(1) << " ms" << endl;

    return (0);
}