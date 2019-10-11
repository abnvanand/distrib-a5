#include<iostream>
#include<bits/stdc++.h>
#include<mpi.h>

#define ROOT_ID 0

using namespace std;

// Merge algo TC = O(m+n)
void merge(vector<int> &v1, int *v2, int m) {
    vector<int> res;
    int n = v1.size();
    int i = 0, j = 0;

    while (i < n && j < m) {
        if (v1[i] < v2[j]) {
            res.push_back(v1[i]);
            i++;
        } else {
            res.push_back(v2[j]);
            j++;
        }
    }
    if (i == n) {
        while (j < m) {
            res.push_back(v2[j]);
            j++;
        }
    }
    if (j == m) {
        while (i < n) {
            res.push_back(v1[i]);
            i++;
        }
    }
    v1 = res;
}

void quickSort(int i, int j, int *A) {
    if (i >= j) {
        return;
    } else {
        int pivot = i;
        int low = i + 1;
        int high = j;
        while (low <= high) {
            while (low <= high && A[high] > A[pivot]) {
                high--;
            }
            while (low <= high && A[low] < A[pivot]) {
                low++;
            }
            if (low > high) {
                break;
            }
            swap(A[high], A[low]);
            low++;
            high--;
        }
        swap(A[high], A[pivot]);
        quickSort(high + 1, j, A);
        quickSort(i, high - 1, A);
    }
}

int main(int argc, char *argv[]) {

    int my_id, n_procs;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int array_size;
    if (my_id == ROOT_ID) {
        // get array size in root proc
        cin >> array_size;
//        cout << "array_size=" << array_size << endl;
    }

    MPI_Bcast(&array_size, 1, MPI_INT, ROOT_ID, MPI_COMM_WORLD);

    int A[array_size];
    if (my_id == ROOT_ID) {
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
    if (my_id == ROOT_ID) {
        // for root proc send then recv

        // Sort root proc's part
        quickSort(0, subarray_size_root_proc - 1, A);
        int j = 0;
        for (;j < subarray_size_root_proc; j++) {
            // fill root proc's sorted part
            final_sorted_array.push_back(A[j]);
        }

        for (int proc_id = 1; proc_id < n_procs; proc_id++) {
            // send data to child procs
            MPI_Send(&A[0 + (subarray_size_root_proc) + (proc_id - 1) * subarray_size],
                     subarray_size, MPI_INT, proc_id, 0,
                     MPI_COMM_WORLD);
        }

        for (int proc_id = 1; proc_id < n_procs; proc_id++) {
            int sorted_subarrays[subarray_size];
            // receive sorted data from child procs
            MPI_Recv(&sorted_subarrays[0], subarray_size, MPI_INT,
                     proc_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            merge(final_sorted_array, sorted_subarrays, subarray_size);
        }

    } else {
        // for child procs recv then send

        int sorted_subarrays[subarray_size];
        // Receive unsorted subarrays in each child proc
        MPI_Recv(&sorted_subarrays[0], subarray_size, MPI_INT,
                 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        quickSort(0, subarray_size - 1, sorted_subarrays);

        // send sorted subarrays to root proc
        MPI_Send(&sorted_subarrays[0], subarray_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (my_id == 0) {
        // print final_sorted_array in root proc
        for (int i = 0; i < array_size; i++) {
            cout << final_sorted_array[i] << " ";
        }
    }

    MPI_Finalize();
    return 0;
}