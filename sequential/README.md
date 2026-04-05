# Parallel Assignment: String Merge Sort (Sequential)

## Student Information
- Name: Fajar Shahzad
- Roll Number: 2023-CS-07

## Problem Description
This project implements the **Merge Sort** algorithm to sort an array of random strings. Merge Sort was chosen because it is a classic "Divide and Conquer" algorithm with a stable time complexity of $O(N \log N)$. 

Sorting strings instead of integers was specifically selected for this assignment because string comparison (`strcmp`) is more computationally expensive than integer comparison. This higher computational load makes the performance gains more visible when we move to the parallel implementation.

## How to Run
### Sequential Version

**1. Compilation:**
Use the standard GCC compiler to build the executable:
```bash
gcc -o seq_sort sequential.c