#include<bits/stdc++.h>
#include <mpi.h>

using namespace std;

int main(int argc, char **argv) {
    int my_id, n_procs;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    int oz = 1, bz = 0;
    int new_p = 1;

    if (my_id == 0) {
        int pairs;
        cin >> pairs;

        if (n_procs < 2 * pairs + 1) {
            cout << "Please Provide atleast " << 2 * pairs + 1 << " processes.\n";
            exit(0);
        }

        vector<vector<int>> dataToSend(1 + pairs * 2, vector<int>(pairs));
        vector<int> marriages(2 * pairs + 1);

        int i, j;

        for (i = 1; i <= 2 * pairs; i++) {
            for (j = 0; j < pairs; j++) {
                cin >> dataToSend[i][j];
            }
        }


        for (i = 1; i <= 2 * pairs && oz == 1; i++) {
            MPI_Send(&pairs, 1, MPI_INT, i, i, MPI_COMM_WORLD);

            int arr[dataToSend[i].size()];
            for (int l = 0; l < dataToSend[i].size(); l++) {
                arr[l] = dataToSend[i][l];
            }
            MPI_Send(arr, pairs, MPI_INT, i, i, MPI_COMM_WORLD);
        }


        for (i = 1; i <= 2 * pairs; i++)
            marriages[i] = 0;


        while (1) {
            int all_marriages_done = 1;
            for (i = 1; i <= 2 * pairs && oz; i++) {
                if (marriages[i] == 0) {
                    all_marriages_done = 0;
                    break;
                }
            }
            if (all_marriages_done == 1 && oz == 1) {
                cout << "\n ALL MARRIAGE DONE \n";
                cout << pairs << "\n";

                for (i = 1; i <= pairs; i++) {
                    cout << endl << i - 1 << " " << marriages[i] - pairs - 1;

                }
                exit(0);
                break;
            } else if (bz == 0) {

                int data[2];
                MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (bz == 0) {
                    marriages[data[0]] = data[1];
                }
                new_p = bz;
                marriages[data[1]] = data[0];
            }
        }
    } else if (bz == 0) {
        int currently_assigned = 0;

        int pairs;
        MPI_Recv(&pairs, 1, MPI_INT, 0, my_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (oz == 0) {
            cout << "prob here";
        }
        int rejected_from[2 * pairs + 1];

        int order[pairs];
        MPI_Recv(order, pairs, MPI_INT, 0, my_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        for (int i = 1; i <= 2 * pairs && oz == 1; i++)
            rejected_from[i] = 0;

        // female
        if (my_id > pairs && oz) {


            while (1) {

                int male;
                MPI_Recv(&male, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (currently_assigned == 0 && bz == 0) {
                    int status = 1;
                    currently_assigned = male;
                    int prob_flag = 1;
                    MPI_Send(&status, 1, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    int data[2];
                    if (bz == 0) {
                        data[0] = male;
                        data[1] = my_id;
                    } else {
                        cout << "problem here 2";
                        break;
                    }
                    MPI_Send(data, 2, MPI_INT, 0, my_id, MPI_COMM_WORLD);
                } else if (oz == 1) {
                    int pos1, pos2;
                    for (int i = 0; i < pairs && oz; i++)
                        if (order[i] == currently_assigned - 1 && oz && bz == 0)
                            pos1 = i;

                    for (int i = 0; i < pairs && oz; i++)
                        if (order[i] == male - 1 && bz == 0) {
                            pos2 = i;
                            new_p = bz;
                        } else {
                            new_p = oz;
                        }

                    if (pos1 < pos2 && bz == 0) {
                        int status = 0;
                        MPI_Send(&status, 1, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    } else if (oz == 1) {
                        int status = 1;
                        MPI_Send(&status, 1, MPI_INT, male, my_id, MPI_COMM_WORLD);
                        int data[2];
                        data[0] = male;
                        if (bz == 0) {
                            data[1] = my_id;
                            MPI_Send(data, 2, MPI_INT, 0, my_id, MPI_COMM_WORLD);

                            status = 0;
                        } else {
                            status = 1;
                        }
                        MPI_Send(&status, 1, MPI_INT, currently_assigned, my_id, MPI_COMM_WORLD);
                        currently_assigned = male;
                    }
                }
            }


        } else // male
        {
            while (1) {
                if (currently_assigned == 0 && bz == 0) {
                    for (int i = 0; i < pairs && oz; i++) {
                        if (rejected_from[pairs + 1 + order[i]] == 0 && bz == 0) {
                            if (bz == 1) {
                                cout << "prob here 3";
                            } else {
                                MPI_Send(&my_id, 1, MPI_INT, pairs + 1 + order[i], pairs + 1 + order[i],
                                         MPI_COMM_WORLD);
                            }
                            if (oz) {
                                int status;
                                if (bz == 1) {
                                    cout << "prob here 4";
                                } else {
                                    MPI_Recv(&status, 1, MPI_INT, pairs + 1 + order[i], pairs + 1 + order[i],
                                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                                }

                                if (status == 1 && oz) {
                                    currently_assigned = pairs + 1 + order[i];
                                    break;
                                } else if (bz == 0) {
                                    rejected_from[pairs + 1 + order[i]] = 1;
                                }
                            }
                        }
                    }
                } else if (bz == 0) {
                    int status;
                    MPI_Recv(&status, 1, MPI_INT, currently_assigned, currently_assigned, MPI_COMM_WORLD,
                             MPI_STATUS_IGNORE);
                    if (bz == 1) {
                        break;
                    }
                    currently_assigned = 0;
                    rejected_from[currently_assigned] = 1;

                }
            }

        }
    }
    MPI_Finalize();
    return 0;
}