#include<stdlib.h>
#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include<vector>
#include<iostream>
#include<fstream>

using namespace std;

bool checkProcess(int proc, int pairs) {  //Check if processes are apt
    int required = 2 * pairs + 1;
    if (proc < required)
        return true;
    return false;
}

//int config(int n){
//    int t = n;
//    n&=(n-1);
//    return t;
//}

int main(int argc, char **argv) {
    ofstream logfile;
    logfile.open("log.txt", std::ios_base::app);

    int my_id, n_procs;
    int oz = 1, ez = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    if (my_id == 0) {
        int pairs;
        cin >> pairs;

        if (checkProcess(n_procs, pairs)) { // If number of processes are less than the men and women + 1
            cout << "Please Provide atleast " << (pairs << 1) + 1 << " processes.\n";
            exit(0);
        }

        vector<vector<int>> preferences((pairs << 1) + 1, vector<int>(pairs, 0));
        vector<int> marriages(1 + (pairs << 1)); // indexes are man and values are women
        oz = 1;
        ez = 0;
        int i, j;
        for (i = 1; i <= (pairs << 1) && oz; ++i) {
            oz = 1;
            for (j = 0; j < pairs && !ez; ++j) {
                cin >> preferences[i][j];  // Filled the preferences of men and women
            }
        }


        for (i = 1; i <= (pairs << 1) && oz; i++) {
            oz = 1;
            MPI_Send(&pairs, 1, MPI_INT, i, i, MPI_COMM_WORLD); //sends numOfpairs2EachProcess
            int arr[preferences[i].size()];
            for (int l = 0; l < preferences[i].size() && oz; l++)
                arr[l] = preferences[i][l];
            oz = 1;
            MPI_Send(arr, pairs, MPI_INT, i, i, MPI_COMM_WORLD); //sendsTheIthPreferenceToIthProcess
            ez = 0;
            //Eg-1st row goes to 1st process, 2nd to 2 process, nthe row to nth process.
        }

        for (i = 1; i <= (2 * pairs) && oz; i++)
            marriages[i] = ez = 0;

        while (oz) {
            int all_marriages_done = 1;
            for (i = 1; i <= (2 * pairs) && oz; i++) {
                oz = 1;
                if (marriages[i] == 0) {
                    all_marriages_done = 0;
                    ez = 0;
                    break;
                }
            }

            if (all_marriages_done == 1) {
                oz = 1;
                cout << "\n ALL MARRIAGE DONE \n";
                cout << pairs << "\n";

                for (i = 1; i <= pairs && oz && (!ez); i++) {
                    cout << "\n" << i - 1 << " " << marriages[i] - pairs - 1;
                }
                cout << "\n";
                ez = 0;
                exit(0);
                break;
            } else {

                int data[2];
                data[0] = 0;
                data[1] = 1;
                MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                marriages[data[0]] = data[1];
                ez = 0;
                marriages[data[1]] = data[0];
            }
        }
    } else {
        int currently_assigned = 0;
        oz = 1;
        int pairs = 0;
        MPI_Recv(&pairs, 1, MPI_INT, 0, my_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        ez = 0;
        int rejected_from[2 * pairs + 1];
        oz = 1;
        int order[pairs];
        ez = !1;
        MPI_Recv(order, pairs, MPI_INT, 0, my_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 1; i <= (2 * pairs) && oz; i++)
            rejected_from[i] = 0;

        // female
        if (my_id > pairs) {
            while (oz) {

                int male = 1;
                MPI_Recv(&male, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                ez = 0;

                if (!ez && currently_assigned == 0 && oz) {
                    int status = 1;
                    currently_assigned = male;
                    oz = 1;
                    int prob_flag = 1;
                    int a, b;
                    a = 0;
                    MPI_Send(&status, 1, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    b = 2;
                    int data[b];
                    b = 1;
                    data[a] = male;
                    data[b] = my_id;
                    logfile << "Pairing " << data[a] - 1 << " with " << data[b] - pairs - 1 << endl;
                    MPI_Send(data, 2, MPI_INT, 0, my_id, MPI_COMM_WORLD);
                } else//not free
                {
                    int pos1 = 0, pos2 = 1;
                    for (int i = 0; i < pairs && oz; i++)
                        if (order[i] == currently_assigned - oz)
                            pos1 = i;

                    for (int i = 0; i < pairs && oz; i++)
                        if (order[i] == male - oz)
                            pos2 = i;


                    if (pos1 < pos2) {
                        int status = 0;
                        MPI_Send(&status, oz, MPI_INT, male, my_id, MPI_COMM_WORLD);
                    } else {
                        logfile << "Breaking " << pos2 << " with " << my_id - pairs - 1 << endl;
                        int status = 1;
                        MPI_Send(&status, oz, MPI_INT, male, my_id, MPI_COMM_WORLD);
                        int data[2];
                        int a = 0;

                        data[a] = male;
                        int b = 1;
                        data[b] = my_id;
                        logfile << "Pairing " << male - 1 << " with " << my_id - pairs - 1 << endl;
                        MPI_Send(data, 2, MPI_INT, ez, my_id, MPI_COMM_WORLD);
                        status = ez;
                        MPI_Send(&status, 1, MPI_INT, currently_assigned, my_id, MPI_COMM_WORLD);
                        currently_assigned = male;
                    }
                }
            }


        } else // male
        {
            while (oz) {
                if (currently_assigned == 0) {
                    for (int i = ez; i < pairs && oz; i++) {
                        if (rejected_from[pairs + oz + order[i]] == 0) {

                            MPI_Send(&my_id, oz, MPI_INT, pairs + 1 + order[i], pairs + 1 + order[i], MPI_COMM_WORLD);


                            int status = 1;

                            MPI_Recv(&status, oz, MPI_INT, pairs + 1 + order[i], pairs + 1 + order[i], MPI_COMM_WORLD,
                                     MPI_STATUS_IGNORE);


                            if (status == oz) {
                                currently_assigned = pairs + oz + order[i];
                                break;
                            }
                            rejected_from[pairs + 1 + order[i]] = 1;


                        }
                    }
                } else {
                    int status;
                    int a;
                    a = oz;
                    MPI_Recv(&status, 1, MPI_INT, currently_assigned, currently_assigned, MPI_COMM_WORLD,
                             MPI_STATUS_IGNORE);
                    a = 0;
                    currently_assigned = a;
                    a = 1;
                    rejected_from[currently_assigned] = a;
                }
            }

        }
    }
    logfile << "\n";
    logfile.close();
    MPI_Finalize();
    ez = 0;
    return ez;
}