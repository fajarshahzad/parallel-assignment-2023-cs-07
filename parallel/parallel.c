#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

// Standard Merge function
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

// Recursive Merge Sort
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

    if (argc > 1) n = atoi(argv[1]);

    int local_n = n / size;
    int *local_data = malloc(local_n * sizeof(int));
    int *full_data = NULL;
    double t_sequential = 0.0;

    if (rank == 0) {
        full_data = malloc(n * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < n; i++) full_data[i] = rand() % 10000;

        // Calculate Sequential Time (Ts) for Speedup baseline
        int *temp_seq = malloc(n * sizeof(int));
        int *copy_seq = malloc(n * sizeof(int));
        memcpy(copy_seq, full_data, n * sizeof(int));
        
        double start_seq = MPI_Wtime();
        merge_sort(copy_seq, temp_seq, 0, n - 1);
        t_sequential = MPI_Wtime() - start_seq;

        free(temp_seq);
        free(copy_seq);
    }

    // Distribute data
    MPI_Scatter(full_data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // --- Start Parallel Timing ---
    MPI_Barrier(MPI_COMM_WORLD);
    double start_p = MPI_Wtime();

    int *temp_local = malloc(local_n * sizeof(int));
    merge_sort(local_data, temp_local, 0, local_n - 1);
    free(temp_local);

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
    double end_p = MPI_Wtime();
    // --- End Parallel Timing ---

    if (rank == 0) {
        double t_parallel = end_p - start_p;
        double speedup = t_sequential / t_parallel;
        double efficiency = speedup / size;

        printf("\n--- Results ---\n");
        printf("Size: %d | Procs: %d\n", n, size);
        printf("Parallel Time: %f s\n", t_parallel);
        printf("Speedup: %.2f | Efficiency: %.2f\n", speedup, efficiency);

        // CSV Logging
        mkdir("results", 0777); 
        FILE *fp = fopen("results/parralel_performance.csv", "a");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            if (ftell(fp) == 0) fprintf(fp, "Size,Processes,T_Sequential,T_Parallel,Speedup,Efficiency\n");
            fprintf(fp, "%d,%d,%f,%f,%f,%f\n", n, size, t_sequential, t_parallel, speedup, efficiency);
            fclose(fp);
            printf("Metrics saved to results/parralel_performance.csv\n");
        }
        free(full_data);
    }

    free(local_data);
    MPI_Finalize();
    return 0;
}
