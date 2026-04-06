#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

// Standard Merge for integers
void merge(int *arr, int *temp, int low, int mid, int high) {
    int i = low, j = mid + 1, k = low;
    while (i <= mid && j <= high) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= high) temp[k++] = arr[j++];
    for (i = low; i <= high; i++) arr[i] = temp[i];
}

void merge_sort(int *arr, int *temp, int low, int high) {
    if (low < high) {
        int mid = low + (high - low) / 2;
        merge_sort(arr, temp, low, mid);
        merge_sort(arr, temp, mid + 1, high);
        merge(arr, temp, low, mid, high);
    }
}

void print_array(int *arr, int n, const char *msg) {
    printf("%s", msg);
    if (n > 20) {
        printf("[Array too large to print fully]\n");
        return;
    }
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

int main(int argc, char** argv) {
    int rank, size, n = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Enter array size: ");
        if (scanf("%d", &n) != 1 || n <= 0) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast size to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_n = n / size;
    int *local_data = malloc(local_n * sizeof(int));
    int *full_data = NULL;

    if (rank == 0) {
        full_data = malloc(n * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < n; i++) full_data[i] = rand() % 1000;
        
        print_array(full_data, n, "\n--- Original Array ---\n");
    }

    // Distribute data
    MPI_Scatter(full_data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // Local Sort
    int *temp = malloc(local_n * sizeof(int));
    merge_sort(local_data, temp, 0, local_n - 1);
    free(temp);

    // Tree-based Merging
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int recv_size = local_n * step;
                int *recv_buf = malloc(recv_size * sizeof(int));
                MPI_Recv(recv_buf, recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                int total_size = recv_size * 2;
                int *merged_buf = malloc(total_size * sizeof(int));
                
                // Merge two sorted halves
                int i = 0, j = 0, k = 0;
                while (i < recv_size && j < recv_size) {
                    if (local_data[i] <= recv_buf[j]) merged_buf[k++] = local_data[i++];
                    else merged_buf[k++] = recv_buf[j++];
                }
                while (i < recv_size) merged_buf[k++] = local_data[i++];
                while (j < recv_size) merged_buf[k++] = recv_buf[j++];

                free(local_data);
                free(recv_buf);
                local_data = merged_buf;
            }
        } else {
            int send_size = local_n * step;
            MPI_Send(local_data, send_size, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }

    if (rank == 0) {
        double end_time = MPI_Wtime();
        double time_taken = end_time - start_time;

        print_array(local_data, n, "\n--- Sorted Array ---\n");

        // Metrics (Note: Speedup/Efficiency requires sequential time. 
        // Here we show results for the current P processes)
        printf("\n--- Parallel Performance ---\n");
        printf("Processes (P)   : %d\n", size);
        printf("Execution Time  : %f seconds\n", time_taken);

        // CSV Logging
        const char *dir_path = "../results";
        const char *file_path = "../results/parallel_results.csv";
        
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) mkdir(dir_path, 0777);

        FILE *fp = fopen(file_path, "a");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            if (ftell(fp) == 0) fprintf(fp, "Processes,Size,Time\n");
            fprintf(fp, "%d,%d,%f\n", size, n, time_taken);
            fclose(fp);
            printf("Saved to %s\n", file_path);
        }
    }

    if (rank == 0) free(full_data);
    free(local_data);
    MPI_Finalize();
    return 0;
}
