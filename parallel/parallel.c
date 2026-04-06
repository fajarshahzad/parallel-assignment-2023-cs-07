#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

// Standard Merge function for two sorted halves
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

// Recursive Merge Sort function
void merge_sort(int *arr, int *temp, int low, int high) {
    if (low < high) {
        int mid = low + (high - low) / 2;
        merge_sort(arr, temp, low, mid);
        merge_sort(arr, temp, mid + 1, high);
        merge(arr, temp, low, mid, high);
    }
}

int main(int argc, char** argv) {
    int rank, size, n = 100000;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get array size from argument or default
    if (argc > 1) n = atoi(argv[1]);

    // Error check for divisibility
    if (n % size != 0) {
        if (rank == 0) printf("Error: Array size %d must be divisible by %d processes.\n", n, size);
        MPI_Finalize();
        return 0;
    }

    int local_n = n / size;
    int *local_data = malloc(local_n * sizeof(int));
    int *full_data = NULL;

    if (rank == 0) {
        full_data = malloc(n * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < n; i++) full_data[i] = rand() % 10000;
        printf("Parallel sorting started for N=%d on P=%d\n", n, size);
    }

    // --- Start Parallel Timing ---
    // Start timing after data generation but including Scatter
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // 1. Distribute data
    MPI_Scatter(full_data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // 2. Local Sort
    int *temp_local = malloc(local_n * sizeof(int));
    merge_sort(local_data, temp_local, 0, local_n - 1);
    free(temp_local);

    // 3. Tree-based Merging
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int current_size = local_n * step;
                int *recv_buf = malloc(current_size * sizeof(int));
                
                MPI_Recv(recv_buf, current_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                int *merged_buf = malloc(current_size * 2 * sizeof(int));
                int i = 0, j = 0, k = 0;
                while (i < current_size && j < current_size) {
                    if (local_data[i] <= recv_buf[j]) merged_buf[k++] = local_data[i++];
                    else merged_buf[k++] = recv_buf[j++];
                }
                while (i < current_size) merged_buf[k++] = local_data[i++];
                while (j < current_size) merged_buf[k++] = recv_buf[j++];

                free(local_data);
                free(recv_buf);
                local_data = merged_buf;
            }
        } else {
            MPI_Send(local_data, local_n * step, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
            break; 
        }
        step *= 2;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    // --- End Parallel Timing ---

    if (rank == 0) {
        double time_taken = end_time - start_time;
        printf("Parallel Execution Time: %f seconds\n", time_taken);

        // --- CSV Logging in Project Root ---
        const char *dir_path = "../results";
        const char *file_path = "../results/parallel_results.csv";
        
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            mkdir(dir_path, 0777);
        }

        FILE *fp = fopen(file_path, "a");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            if (ftell(fp) == 0) {
                fprintf(fp, "Size,Processes,Parallel_Time\n");
            }
            fprintf(fp, "%d,%d,%f\n", n, size, time_taken);
            fclose(fp);
            printf("Results saved to %s\n", file_path);
        }
        free(full_data);
    }

    free(local_data);
    MPI_Finalize();
    return 0;
}
