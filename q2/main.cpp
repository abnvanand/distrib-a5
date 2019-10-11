#include<bits/stdc++.h>
#include <mpi.h>

#define ROOT_ID 0
using namespace std;

bool allMarried(vector<int> &marriages, int pairs) {
    int N = 2 * pairs;
    for (int i = 1; i <= N; i++) {
        if (marriages[i] == 0) {
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
        int n_pairs;
        cin >> n_pairs;
        int n_procs_needed = 1 + (n_pairs << 1);
        if (n_procs < n_procs_needed) {
            cout << "Need " << n_procs_needed << " processes." << endl;
            exit(0);
        }
        vector<int> marriages(n_procs_needed, 0);
        vector<vector<int>> input(n_procs_needed, vector<int>(n_pairs));

        for (int i = 1; i <= 2 * n_pairs; i++) {
            for (int j = 0; j < n_pairs; j++) {
                cin >> input[i][j];
            }
        }

        for (int i = 1; i < n_procs_needed; i++) {
            int count = 1;
            MPI_Send(&n_pairs, count, MPI_INT, i, i, MPI_COMM_WORLD);
            int s = n_pairs;
            int arr[s];
            for (int l = 0; l < n_pairs; l++) {
                arr[l] = input[i][l];
            }
            MPI_Send(arr, s, MPI_INT, i, i, MPI_COMM_WORLD);
        }

        while (true) {
            if (allMarried(marriages, n_pairs)) {
                for (int i = 1; i <= n_pairs; i++) {
                    int men_id = i - 1, women_id = marriages[i] - n_pairs - 1;
                    cout << men_id << " " << women_id << endl;
                }
                break;
            } else {
                int data[2] = {0};
                int count = 2;
                MPI_Recv(data, count, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int x = data[0], y = data[1];
                marriages[x] = y;
                marriages[y] = x;
            }
        }
    } else {
        int n_procs_need;
        int pairs;
        int count = 1;
        MPI_Recv(&pairs, count, MPI_INT, ROOT_ID, my_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        n_procs_need = 2 * pairs + 1;
        int currently_assigned = 0;
        int order[pairs];
        vector<int> rejection(n_procs_need, 0);
        MPI_Recv(order, pairs, MPI_INT, ROOT_ID, my_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (my_id > pairs) {
            while (true) {
                int male;
                count = 1;
                MPI_Recv(&male, count, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (currently_assigned != 0) {
                    int pos1 = 0, pos2 = 0;
                    for (int i = 0; i < pairs; i++) {
                        int val = currently_assigned;
                        val -= 1;
                        if (order[i] == val) {
                            pos1 = i;
                        }
                    }

                    for (int i = 0; i < pairs; i++) {
                        int val = male;
                        val -= 1;
                        if (order[i] == val) {
                            pos2 = i;
                        }
                    }

                    if (pos1 > pos2) {
                        int status = 1;
                        count = 1;
                        MPI_Send(&status, count, MPI_INT, male, my_id, MPI_COMM_WORLD);
                        int data[2];
                        data[1] = my_id;
                        logfile << "breaking  " << pos1 << " and " << my_id - pairs - 1 << endl;
                        logfile.flush();
//                        cout << "Breaking Pairs of  " << pos1 << " " << my_id - pairs - 1 << endl;
                        data[0] = male;
                        count = 2;
                        MPI_Send(data, count, MPI_INT, 0, my_id, MPI_COMM_WORLD);
                        status = 0;
                        int men_id = male - 1, women_id = my_id - pairs - 1;
                        logfile << "pairing  " << men_id << " and " << women_id << endl;
                        logfile.flush();
//                        cout << "Making Pairs of  " << men_id << " " << women_id << endl;
                        count = 1;
                        MPI_Send(&status, count, MPI_INT, currently_assigned, my_id, MPI_COMM_WORLD);
                        currently_assigned = male;
                    } else {
                        int status = 0;
                        count = 1;
                        MPI_Send(&status, count, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    }
                } else {
                    int status = 1;
                    count = 1;
                    MPI_Send(&status, count, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    int data[2];
                    data[1] = my_id;
                    currently_assigned = male;
                    data[0] = currently_assigned;
                    count = 2;
//                    cout << "Making Pairs of  " << data[0] - 1 << " " << data[1] - pairs - 1 << endl;
                    int men_id = data[0] - 1, women_id = data[1] - pairs - 1;
                    logfile << "pairing " << men_id << " and " << women_id << endl;
                    logfile.flush();
                    MPI_Send(data, count, MPI_INT, ROOT_ID, my_id, MPI_COMM_WORLD);
                }

            }
        } else {
            while (true) {
                if (currently_assigned != 0) {
                    int status;
                    int count = 1;
                    MPI_Status mpiStatus;
                    MPI_Recv(&status, count, MPI_INT, currently_assigned, currently_assigned, MPI_COMM_WORLD,
                             &mpiStatus);
                    currently_assigned = 0;
                    rejection[currently_assigned] = 1;
                } else {
                    for (int i = 0; i < pairs; i++) {
                        int index = order[i];
                        index += (pairs + 1);
                        if (rejection[index] == 0) {
                            count = 1;
                            MPI_Send(&my_id, count, MPI_INT, index, index, MPI_COMM_WORLD);
                            int status;
                            MPI_Status mpiStatus;

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
    logfile << endl;
    logfile.flush();
    logfile.close();
    MPI_Finalize();
    return 0;
}