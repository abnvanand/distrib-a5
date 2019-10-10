#include<iostream>
#include<bits/stdc++.h>
#include<mpi.h>

using namespace std;

void MergeArrays(vector<int> &v1, int *v2, int m) {
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

void QuickSort(int i, int j, int *A) {
    if (i >= j) {
        return;
    } else {
        int pivot = i;
        int low = i + 1;
        int high = j;
        while (low <= high) {
            // cout<<"hey";
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
        QuickSort(high + 1, j, A);
        QuickSort(i, high - 1, A);
    }
}

int main(int argc, char *argv[]) {

    int my_id, n_procs;
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int array_size;
    if (my_id == 0) {
        // get array size in root proc
//        array_size = stoi(argv[1]);
        cin >> array_size;
        cout << "array_size=" << array_size << endl;
    }

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int A[array_size];
    if (my_id == 0) {
        for (int i = 0; i < array_size; i++) {
//            A[i] = stoi(argv[i + 2]);
            cin >> A[i];
            cout << "A[" << i << "]=" << A[i] << " ";
        }
        cout << endl;
    }

    int ele_per_proc = array_size / n_procs;
    int ele_fst_proc = array_size - (n_procs - 1) * ele_per_proc;


    vector<int> result;
    if (my_id == 0) {
        QuickSort(0, ele_fst_proc - 1, A);
        int j = 0;
        while (j < ele_fst_proc) {
            result.push_back(A[j]);
            j++;
        }

        for (int i = 1; i < n_procs; i++) {
            MPI_Send(&A[0 + (ele_fst_proc) + (i - 1) * ele_per_proc], ele_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD);
            // cout<<"send : "<<i<<endl;
        }

        for (int i = 1; i < n_procs; i++) {
            int temp_proc_0[ele_per_proc];
            MPI_Recv(&temp_proc_0[0], ele_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // for(int k=0;k<ele_per_proc;k++)
            // {
            //     cout<<temp_proc_0[k]<<" ";
            // }
            // cout<<endl;

            MergeArrays(result, temp_proc_0, ele_per_proc);
            // cout<<"recv : "<<i<<endl;
        }

    } else {
        int temp_proc_1[ele_per_proc];
        MPI_Recv(&temp_proc_1[0], ele_per_proc, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        QuickSort(0, ele_per_proc - 1, temp_proc_1);

        MPI_Send(&temp_proc_1[0], ele_per_proc, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (my_id == 0) {
        for (int i = 0; i < array_size; i++) {
            cout << result[i] << " ";
        }
    }

    MPI_Finalize();
    return 0;
}