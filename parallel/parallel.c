#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STR_LEN 16

void merge(char **arr, char **temp, int low, int mid, int high) {
    int i = low, j = mid + 1, k = low;
    while (i <= mid && j <= high) {
        if (strcmp(arr[i], arr[j]) <= 0) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= high) temp[k++] = arr[j++];
    for (i = low; i <= high; i++) arr[i] = temp[i];
}

void merge_sort(char **arr, char **temp, int low, int high) {
    if (low < high) {
        int mid = low + (high - low) / 2;
        merge_sort(arr, temp, low, mid);
        merge_sort(arr, temp, mid + 1, high);
        merge(arr, temp, low, mid, high);
    }
}

int main(int argc, char** argv) {
    int rank, size, n = 1000, opt;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
        if (opt == 'n') n = atoi(optarg);
    }

    int local_n = n / size;
    char *local_buffer = malloc(local_n * STR_LEN);
    char **local_ptr = malloc(local_n * sizeof(char*));

    if (rank == 0) {
        char *full_buffer = malloc(n * STR_LEN);
        srand(42); 
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < STR_LEN - 1; j++) full_buffer[i * STR_LEN + j] = 'A' + (rand() % 26);
            full_buffer[i * STR_LEN + STR_LEN - 1] = '\0';
        }

        // Point-to-Point Distribution
        for (int i = 1; i < size; i++) {
            MPI_Send(full_buffer + (i * local_n * STR_LEN), local_n * STR_LEN, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
        memcpy(local_buffer, full_buffer, local_n * STR_LEN);
        free(full_buffer);
    } else {
        MPI_Recv(local_buffer, local_n * STR_LEN, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    for (int i = 0; i < local_n; i++) local_ptr[i] = local_buffer + (i * STR_LEN);
    char **temp_ptr = malloc(local_n * sizeof(char*));

    // Local Sort
    merge_sort(local_ptr, temp_ptr, 0, local_n - 1);

    // Tree-based Merging
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int recv_count = local_n * step;
                char *recv_buf = malloc(recv_count * STR_LEN);
                MPI_Recv(recv_buf, recv_count * STR_LEN, MPI_CHAR, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                int total_elements = recv_count * 2;
                char *new_buffer = malloc(total_elements * STR_LEN);
                
                int i = 0, j = 0, k = 0;
                while (i < recv_count && j < recv_count) {
                    if (strcmp(local_ptr[i], recv_buf + (j * STR_LEN)) <= 0) 
                        memcpy(new_buffer + (k++ * STR_LEN), local_ptr[i++], STR_LEN);
                    else 
                        memcpy(new_buffer + (k++ * STR_LEN), recv_buf + (j++ * STR_LEN), STR_LEN);
                }
                while (i < recv_count) memcpy(new_buffer + (k++ * STR_LEN), local_ptr[i++], STR_LEN);
                while (j < recv_count) memcpy(new_buffer + (k++ * STR_LEN), recv_buf + (j++ * STR_LEN), STR_LEN);

                free(local_buffer); free(local_ptr); free(recv_buf);
                local_buffer = new_buffer;
                local_ptr = malloc(total_elements * sizeof(char*));
                for(int m=0; m<total_elements; m++) local_ptr[m] = local_buffer + (m * STR_LEN);
            }
        } else {
            MPI_Send(local_buffer, local_n * step * STR_LEN, MPI_CHAR, rank - step, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    if (rank == 0) {
        double end_time = MPI_Wtime();
        printf("--- Parallel Result ---\n");
        printf("Processes Used: %d\n", size);
        printf("Execution Time: %f seconds\n", end_time - start_time);
        
        int sorted = 1;
        for (int i = 0; i < n - 1; i++) {
            if (strcmp(local_ptr[i], local_ptr[i+1]) > 0) { sorted = 0; break; }
        }
        printf("Verification: %s\n", sorted ? "PASSED" : "FAILED");
    }

    MPI_Finalize();
    return 0;
}
