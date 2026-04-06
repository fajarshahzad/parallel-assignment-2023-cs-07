#  Parallel Assignment: String Merge Sort (Sequential)

<p align="center">
  <b>Baseline Implementation for Performance Comparison</b>
</p>

---

##  Student Information

| Field        | Details            |
|-------------|--------------------|
| Name        | Fajar Shahzad      |
| Roll Number | 2023-CS-07         |

---

##  Overview
This module implements a **Sequential Merge Sort** for sorting randomly generated strings.  
It serves as a **baseline** to evaluate the performance improvements of the parallel version.

---

##  Problem Description
The task is to sort an array of **fixed-length random strings** using Merge Sort.

###  Why Merge Sort?
- Stable sorting algorithm  
- Time complexity: **O(N log N)**  
- Based on **Divide and Conquer**  
- Efficient for large datasets  

###  Why Strings Instead of Integers?
Sorting strings using `strcmp()`:
- Involves **character-by-character comparison**
- More **CPU-intensive**
- Provides a better benchmark for parallel performance

---

##  How to Run

###  Compilation
```bash
gcc -o seq_sort sequential.c
