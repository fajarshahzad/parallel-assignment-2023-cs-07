#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
    int n = 1000, opt;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        if (opt == 'n') n = atoi(optarg);
    }

    char **data = malloc(n * sizeof(char*));
    char **temp = malloc(n * sizeof(char*));
    srand(42); 

    for (int i = 0; i < n; i++) {
        data[i] = malloc(STR_LEN);
        for (int j = 0; j < STR_LEN - 1; j++) data[i][j] = 'A' + (rand() % 26);
        data[i][STR_LEN - 1] = '\0';
    }

    clock_t start = clock();
    merge_sort(data, temp, 0, n - 1);
    clock_t end = clock();

    printf("--- Sequential Result ---\n");
    printf("Problem Size: %d\n", n);
    printf("Execution Time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    for (int i = 0; i < n; i++) free(data[i]);
    free(data); free(temp);
    return 0;
}
