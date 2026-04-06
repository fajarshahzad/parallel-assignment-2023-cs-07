#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

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

int main() {
    int n;
    printf("Enter array size: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 1;

    int *data = malloc(n * sizeof(int));
    int *temp = malloc(n * sizeof(int));
    srand(time(NULL));

    for (int i = 0; i < n; i++) data[i] = rand() % 1000;

    // 1. Print Original Array
    print_array(data, n, "\n--- Original Array ---\n");

    // 2. Measure Execution Time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    merge_sort(data, temp, 0, n - 1);
    
    clock_gettime(CLOCK_MONOTONIC, &end);

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Sequential baseline: Speedup = 1.0, Efficiency = 1.0 (or 100%)
    double speedup = 1.0;
    double efficiency = 1.0; 

    // 3. Print Sorted Array
    print_array(data, n, "\n--- Sorted Array ---\n");

    // 4. Print Metrics
    printf("\n--- Performance Metrics ---\n");
    printf("Execution Time : %f seconds\n", time_taken);
    printf("Speedup        : %f\n", speedup);
    printf("Efficiency     : %f (100%%)\n", efficiency);

    // --- CSV Logging in Project Root ---
    const char *dir_path = "../results";
    const char *file_path = "../results/results.csv";

    struct stat st = {0};
    if (stat(dir_path, &st) == -1) {
        mkdir(dir_path, 0777); 
    }

    FILE *fp = fopen(file_path, "a");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        if (ftell(fp) == 0) {
            fprintf(fp, "Size,Time,Speedup,Efficiency\n");
        }
        fprintf(fp, "%d,%f,%f,%f\n", n, time_taken, speedup, efficiency);
        fclose(fp);
        printf("\nData successfully saved to %s\n", file_path);
    } else {
        perror("\nCSV Error");
    }

    free(data); free(temp);
    return 0;
}
