# TP – Systems and Networks

This repository contains three practical assignments (TP) written in C, each focusing on a fundamental concept: memory segmentation, memory-mapped files, and linked lists.


## 1. Segmentation

### Description

This assignment implements memory management using segmentation.
It illustrates:

* The structure of a memory segment
* Dynamic allocation and deallocation
* Logical address decomposition into segment and offset
* Common errors (segmentation fault)

### Features

* Creation of a segmented memory space
* Access to segments using pointers
* Detection of addressing errors (segmentation fault)

### File

* `segmentation.c`

---

## 2. Projection File

### Description

This assignment demonstrates how to use memory-mapped files.
It allows treating a file as if it were an array in memory.

### Features

* File opening with `open`
* Memory mapping with `mmap`
* Direct read/write access in the mapped memory
* Synchronization of changes with the file
* Unmapping and closing with `munmap` and `close`

### File

* `project_file.c`

---

## 3. Linked Lists in C

### Description

This assignment implements different types of linked lists:

* Singly linked list
* Doubly linked list
* Doubly circular linked list

### Basic Operations

* `Push(dlist **head, int val)` → add at the beginning
* `AddLast(dlist **head, int val)` → add at the end
* `Pop(dlist **head)` → remove from the beginning
* `RemoveLast(dlist **head)` → remove from the end
* `Length(dlist *head)` → return list length
* `PrintList(dlist *head)` → display elements with memory addresses

### Advanced Operations

* `ConcatLists(dlist **l1, dlist *l2)` → concatenate two lists
* `MapList(dlist *head, int (*func)(int))` → apply a function to each element
* `square(int x)` → example mapping function (square of an integer)

### Memory Management

* Allocation with `malloc`
* Deallocation with `free`
* Recommended cleanup function: `FreeList(dlist **head)`

### File

* `linked_list.c`

compilation pract : 

gcc segmentation.c -o segmentation
./segmentation

gcc project_file.c -o project_file
./project_file


gcc main.c -o linked_list
./linked_list


