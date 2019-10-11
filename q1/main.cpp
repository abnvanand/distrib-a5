#include<bits/stdc++.h>
#include<mpi.h>

#define ROOT_ID 0

using namespace std;

// Merge algo TC = O(m+n)
void merge(vector<int> &v1, int *v2, int n, int m) {
    vector<int> res;
    int i = 0, j = 0;

    while (i < n and j < m) {
        if (v1[i] < v2[j]) {
            res.push_back(v1[i]);
            i += 1;
        } else {
            res.push_back(v2[j]);
            j += 1;
        }
    }
    if (i == n) {
        while (j < m) {
            res.push_back(v2[j]);
            j += 1;
        }
    }
    if (j == m) {
        while (i < n) {
            res.push_back(v1[i]);
            i += 1;
        }
    }
    v1 = res;
}

bool isRootProc(int my_id) { return my_id == ROOT_ID; }

void quickSort(int *A, int i, int j) {
    // Invalid range
    if (i >= j)
        return;

    int pivot = i;
    int lo = pivot + 1;
    int hi = j;
    while (lo <= hi) {
        while (lo <= hi and A[hi] > A[pivot]) {
            hi -= 1;
        }
        while (lo <= hi and A[lo] < A[pivot]) {
            lo += 1;
        }

        if (lo > hi) {
            break;
        }

        swap(A[hi], A[lo]);
        lo += 1;
        hi -= 1;
    }
    swap(A[hi], A[pivot]);
    quickSort(A, hi + 1, j);
    quickSort(A, i, hi - 1);
}

int main(int argc, char *argv[]) {

    int my_id, n_procs;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int array_size;
    if (isRootProc(my_id)) {
        // get array size in root proc
        cin >> array_size;
//        cout << "array_size=" << array_size << endl;
    }

    MPI_Bcast(&array_size, 1, MPI_INT, ROOT_ID, MPI_COMM_WORLD);

    int A[array_size];
    if (isRootProc(my_id)) {
        // read input array in root proc
//        cout << "Input array " << endl;
        for (int i = 0; i < array_size; i++) {
            cin >> A[i];
//            cout << A[i] << " ";
        }
        cout << endl;
    }

    int subarray_size = array_size / n_procs;
    int subarray_size_root_proc = array_size - (n_procs - 1) * subarray_size;

    vector<int> final_sorted_array;
    if (isRootProc(my_id)) {
        // for root proc send then recv

        // Sort root proc's part
        quickSort(A, 0, subarray_size_root_proc - 1);
        int j = 0;
        for (; j < subarray_size_root_proc; j++) {
            // fill root proc's sorted part
            final_sorted_array.push_back(A[j]);
        }

        for (int child_proc_id = 1; child_proc_id < n_procs; child_proc_id++) {
            // send data to child procs
            int offset = (subarray_size_root_proc)
                         + (child_proc_id - 1) * subarray_size;
            MPI_Send(&A[offset],
                     subarray_size, MPI_INT, child_proc_id, 0,
                     MPI_COMM_WORLD);
        }

        for (int proc_id = 1; proc_id < n_procs; proc_id++) {
            int sorted_subarrays[subarray_size];
            MPI_Status mpiStatus;
            // receive sorted data from child procs
            MPI_Recv(&sorted_subarrays[0], subarray_size, MPI_INT,
                     proc_id, 0, MPI_COMM_WORLD, &mpiStatus);
            merge(final_sorted_array, sorted_subarrays, final_sorted_array.size(), subarray_size);
        }

    } else {
        // for child procs recv then send
        MPI_Status mpiStatus;

        int sorted_subarrays[subarray_size];
        // Receive unsorted subarrays in each child proc
        MPI_Recv(&sorted_subarrays[0], subarray_size, MPI_INT,
                 ROOT_ID, 0, MPI_COMM_WORLD, &mpiStatus);
        quickSort(sorted_subarrays, 0, subarray_size - 1);

        // send sorted subarrays to root proc
        MPI_Send(&sorted_subarrays[0], subarray_size, MPI_INT,
                 ROOT_ID, 0, MPI_COMM_WORLD);
    }

    if (isRootProc(my_id)) {
        // print final_sorted_array in root proc
        for (auto elem:final_sorted_array)
            cout << elem << " ";
    }

    MPI_Finalize();
    return 0;
}