---

### 2. Parallel README (`parallel/README.md`)

```markdown
# Parallel Assignment: String Merge Sort (Parallel MPI)

## Student Information
- **Name:** Fajar Shahzad
- **Roll Number:** 2023-CS-07

---

## Problem Description
This is the parallel implementation of **String Merge Sort** using **MPI (Message Passing Interface)**. It uses a manual distribution and tree-based merging strategy.

### Parallelization Strategy
1. **Data Distribution:** Master (Rank 0) generates the data and uses **Point-to-Point** (`MPI_Send`) to send chunks to workers.
2. **Local Sorting:** Each process sorts its assigned chunk using a sequential Merge Sort.
3. **Tree-based Merge:** Processes pair up using `MPI_Send` and `MPI_Recv` to merge their results hierarchically until the final sorted array reaches Rank 0.

---

## How to Run

### 1. Compilation
Use the MPI compiler wrapper:
```bash
mpicc -o par_sort parallel.c
mpicc -o par_sort parallel.c
