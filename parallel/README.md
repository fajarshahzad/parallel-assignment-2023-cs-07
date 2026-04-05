---

### 2. Parallel README (`parallel/README.md`)

```markdown
# Parallel Assignment: String Merge Sort (Parallel MPI)

## Student Information
- Name: Fajar Shahzad
- Roll Number: 2023-CS-07

## Problem Description
This is the parallel implementation of **String Merge Sort** using the **Message Passing Interface (MPI)**. 

The strategy utilizes **Point-to-Point communication** (`MPI_Send` and `MPI_Recv`) to distribute data. The problem is parallelized by:
1. Dividing the main string array into equal chunks.
2. Sending chunks from the Master (Rank 0) to Worker processes.
3. Each process sorts its local chunk using a sequential merge sort.
4. Using a **Tree-based Merge** strategy where processes pair up to merge their sorted results until the final sorted array reaches the Master.

## How to Run
### Parallel Version

**1. Compilation:**
Use the MPI compiler wrapper (`mpicc`):
```bash
mpicc -o par_sort parallel.c