
---

# 📁 `parallel/README.md`

```markdown
# Parallel Assignment: String Merge Sort (Parallel MPI)

## Student Information
- **Name:** Fajar Shahzad
- **Roll Number:** 2023-CS-07

---

## Problem Description
This is the parallel implementation of **String Merge Sort** using **MPI (Message Passing Interface)**. It utilizes manual data distribution and a tree-based merging strategy.

---

## Parallelization Strategy

1. **Data Distribution:**  
   The Master (Rank 0) generates the global string array and uses **Point-to-Point communication** (`MPI_Send`) to distribute equal chunks to all worker processes.

2. **Local Sorting:**  
   Each process (including Rank 0) sorts its assigned chunk using a local sequential Merge Sort.

3. **Tree-based Merge:**  
   Processes pair up using `MPI_Send` and `MPI_Recv` to merge their results hierarchically. This **tree reduction** continues until the final sorted array reaches Rank 0.

---

## How to Run

### 1. Compilation
Use the MPI compiler wrapper:

```bash
mpicc -o par_sort parallel.c
