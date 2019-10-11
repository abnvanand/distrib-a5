#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define ROOT_ID 0

const int INFINITY = 1000000;


void printMatrix(int *local_mat, int n, int my_rank, int p, int source) {
    int *temp_mat = NULL;
    int j, count;

    if (my_rank == ROOT_ID) {
        temp_mat = malloc(n * n * sizeof(int));
        int nsquare = n * n;
        count = nsquare;

        MPI_Gather(local_mat, count / p, MPI_INT, temp_mat, count / p, MPI_INT, 0, MPI_COMM_WORLD);
        for (j = 0; j < n; j++) {
            int idx = (source - 1) * n + j;
            if (temp_mat[idx] == INFINITY)
                printf("source(%d) ----> node(%d) ==  inf ", source, j + 1);
            else
                printf("source(%d) ----> node(%d) == %d", source, j + 1, temp_mat[(source - 1) * n + j]);
            printf("\n");
        }
        printf("\n");
        free(temp_mat);
    } else {
        int nsquare = n * n;
        count = nsquare / p;
        MPI_Gather(local_mat, count, MPI_INT, temp_mat, count, MPI_INT, 0, MPI_COMM_WORLD);
    }
}

void readMatrix(int *local_mat, int n, int my_rank, int p, int e) {
    int v1, v2, wt;
    int idx = 0;
    int i, j, count;
    int *temp_mat = NULL;

    if (my_rank == ROOT_ID) {
        int nsquare = n * n;
        temp_mat = malloc(sizeof(int) * nsquare);
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++) {
                idx = i * n + j;
                if (i != j) {
                    temp_mat[idx] = INFINITY;
                } else {
                    temp_mat[idx] = 0;
                }
            }
        for (i = 0; i < e; i++) {
            scanf("%d %d %d", &v1, &v2, &wt);
            int temp1 = (v1 - 1);
            int temp2 = temp1 * n;
            temp2 += (v2 - 1);
            temp_mat[temp2] = wt;
        }
        nsquare = n * n;
        count = nsquare / p;
        MPI_Scatter(temp_mat, count, MPI_INT, local_mat, count, MPI_INT, 0, MPI_COMM_WORLD);
        free(temp_mat);
    } else {
        int nsquare = n * n;
        count = nsquare / p;
        MPI_Scatter(temp_mat, count, MPI_INT, local_mat, count, MPI_INT, 0, MPI_COMM_WORLD);
    }

}

void copyRow(int *local_mat, int n, int p, int *row_k, int k) {
    int nbyp = (n / p);
    int local_k = k % nbyp;
    int j;
    for (j = 0; j < n; j++) {
        int idx = local_k * n;
        idx += j;
        row_k[j] = local_mat[idx];
    }
}

void floyd(int *local_matrix, int n, int my_rank, int p) {
    int temp, index1, global_k, index2, index3, global_j, root, local_i;

    int *row_k = malloc(n * sizeof(int));

    for (global_k = 0; global_k < n; global_k++) {
        int nbyp = (n / p);
        root = global_k / nbyp;
        if (my_rank == root)
            copyRow(local_matrix, n, p, row_k, global_k);
        MPI_Comm mpicomm = MPI_COMM_WORLD;
        MPI_Bcast(row_k, n, MPI_INT, root, mpicomm);
        index1 = n;
        index1 /= p;

        for (local_i = 0; local_i < index1; local_i++)
            for (global_j = 0; global_j < n; global_j++) {
                index2 = local_i * n;
                index2 += global_k;
                temp = local_matrix[index2] + row_k[global_j];
                index3 = local_i * n;
                index3 += global_j;
                if (temp < local_matrix[index3])
                    local_matrix[index3] = temp;
            }
    }
    free(row_k);
}

int main(int argc, char *argv[]) {
    MPI_Init(NULL,NULL);

    int my_id, n_procs;
    int n, e, source;
    int *local_mat;


    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    if (my_id == ROOT_ID) {
        scanf("%d %d %d", &n, &e, &source);
    }
    int count = 1;
    MPI_Bcast(&n, count, MPI_INT, ROOT_ID, MPI_COMM_WORLD);
    local_mat = malloc(n * n / n_procs * sizeof(int));

    readMatrix(local_mat, n, my_id, n_procs, e);

    floyd(local_mat, n, my_id, n_procs);

    if (my_id == ROOT_ID)
        printf("Result:\n");

    printMatrix(local_mat, n, my_id, n_procs, source);

    free(local_mat);
    MPI_Finalize();

    return 0;
}
