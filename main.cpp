#include<bits/stdc++.h>
#include <mpi.h>

#define ROOT_ID 0

using namespace std;

bool allMarried(vector<int> &marriages, int pairs) {
    // Check all except root proc for married status
    for (int i = 1; i < 2 * pairs + 1; i++) {
        if (!marriages[i]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    ofstream logfile;
    logfile.open("log.txt", std::ios_base::app);

    int my_id, n_procs;
    MPI_Init(nullptr, nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    if (my_id == ROOT_ID) {
        logfile << endl << "---------------" << endl;

        int n_pairs;
        cin >> n_pairs;
        int n_procs_needed = 1 + (n_pairs << 1);
        if (n_procs < n_procs_needed) {
            cout << "Need " << n_procs_needed << " processes." << endl;
            MPI_Finalize();
            exit(0);
        }

        vector<int> result(n_procs_needed, 0);
        vector<vector<int>> preferences(n_procs_needed, vector<int>(n_pairs));

        for (int i = 1; i <= 2 * n_pairs; i++) {
            for (int j = 0; j < n_pairs; j++) {
                cin >> preferences[i][j];
            }
        }

        for (int child_proc_id = 1; child_proc_id < n_procs_needed; child_proc_id++) {
            // send n_pairs value to child procs
            MPI_Send(&n_pairs, 1, MPI_INT, child_proc_id, child_proc_id, MPI_COMM_WORLD);

            int arr[preferences[child_proc_id].size()];
            for (auto women = 0; women < n_pairs; women++)
                arr[women] = preferences[child_proc_id][women];

            // send array column values to child procs
            MPI_Send(arr, n_pairs, MPI_INT, child_proc_id, child_proc_id, MPI_COMM_WORLD);
        }

        while (true) {
            if (allMarried(result, n_pairs)) {
                for (int i = 0; i < n_pairs; i++) {
                    int men_id = i, women_id = result[i + 1] - n_pairs - 1;
                    cout << men_id << " " << women_id << endl;
                }

                MPI_Finalize();
                exit(0);
                break;
            } else {
                int data[2] = {0};
                MPI_Status mpiStatus;

                MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpiStatus);
                int x = data[0], y = data[1];
                result[y] = x;
                result[x] = y;
            }
        }
    } else {
        int n_procs_needed;
        int n_pairs;
        MPI_Status mpiStatus;

        MPI_Recv(&n_pairs, 1, MPI_INT, ROOT_ID, my_id, MPI_COMM_WORLD, &mpiStatus);
        n_procs_needed = (n_pairs << 1) + 1;
        vector<int> rejection(n_procs_needed, 0);
        int order[n_pairs];
        int currently_assigned = 0;
        MPI_Recv(order, n_pairs, MPI_INT, ROOT_ID, my_id, MPI_COMM_WORLD, &mpiStatus);
        if (my_id > n_pairs) {
            while (true) {
                int male;
                MPI_Recv(&male, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpiStatus);
                if (currently_assigned != 0) {
                    int men1, men2, temp;
                    for (int i = 0; i < n_pairs; i++) {
                        int val = currently_assigned;
                        val -= 1;
                        if (order[i] == val) {
                            temp = i;
                            men1 = temp;
                        }
                    }

                    for (int i = 0; i < n_pairs; i++) {
                        int val = male;
                        val -= 1;
                        if (order[i] == val) {
                            temp = i;
                            men2 = temp;
                        }
                    }

                    if (men1 > men2) {
                        int status = 1;
                        int count = 1;
                        MPI_Send(&status, count, MPI_INT, male, my_id, MPI_COMM_WORLD);

                        int data[2];
                        data[1] = my_id;
                        logfile << "breaking " << men1 << " and " << my_id - n_pairs - 1 << endl;
                        data[0] = male;
                        count = 2;
                        MPI_Send(data, count, MPI_INT, ROOT_ID, my_id, MPI_COMM_WORLD);

                        logfile << "pairing " << male - 1 << " and " << my_id - n_pairs - 1 << endl;

                        status = 0;
                        currently_assigned = male;
                        count = 1;
                        MPI_Send(&status, count, MPI_INT, currently_assigned, my_id, MPI_COMM_WORLD);
                    } else {
                        int status = 0;
                        int count = 1;
                        MPI_Send(&status, count, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    }
                } else {
                    int status = 1;
                    int count = 1;
                    MPI_Send(&status, count, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    int data[2];
                    data[1] = my_id;
                    currently_assigned = male;
                    data[0] = currently_assigned;
                    count = 2;
                    MPI_Send(data, count, MPI_INT, 0, my_id, MPI_COMM_WORLD);
                    logfile << "pairing " << data[0] - 1 << " and " << data[1] - n_pairs - 1 << endl;
                }
            }
        } else {
            while (true) {
                if (currently_assigned != 0) {
                    int count = 1;
                    int status = 0;
                    MPI_Recv(&status, count, MPI_INT, currently_assigned, currently_assigned, MPI_COMM_WORLD,
                             &mpiStatus);
                    currently_assigned = 0;
                    rejection[currently_assigned] = 1;
                } else {
                    for (int i = 0; i < n_pairs; i++) {
                        int index = order[i];
                        index += n_pairs + 1;
                        if (not rejection[index]) {
                            int count = 1;
                            MPI_Send(&my_id, count, MPI_INT, index, index, MPI_COMM_WORLD);
                            int status = 0;
                            MPI_Recv(&status, count, MPI_INT, index, index, MPI_COMM_WORLD, &mpiStatus);
                            if (status != 1) {
                                rejection[index] = 1;
                            } else {
                                currently_assigned = index;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
//    logfile << endl;
    logfile.flush();
    logfile.close();
    MPI_Finalize();
    return 0;
}