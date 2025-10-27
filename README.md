##Project Name

PARALLEL TRAFFIC ACCIDENT ANALYSIS (C++ OPENMP)

----

This project analyzes large traffic accident datasets using C++ and OpenMP.
It compares serial and parallel implementations to show how parallel processing improves performance and scalability.


##What's Inside:


| File | Description |
| ----------------------------- | ------------------------------------------------------------ |
| `traffic_serial.cpp` | Regular (non-parallel) version – can process single or multiple images |
| `trafficaccidentanalysisparallel.cpp` | Parallel version using OpenMP – processes a batch of images faster |

------
## ⚙️ Requirements
* **MinGW-w64 (GCC)** — make sure it supports OpenMP (`libgomp` is included in most versions).
* **C++17** support (needed for `std::filesystem`)
* **PowerShell** (for the commands below)

----------
## How to Build & Run
Open **PowerShell** in the project folder (where your `.cpp` files and images are located).


### 1️. Compile
# Parallel version
g++ traffic_parallel.cpp -o traffic_parallel -std=c++17 -fopenmp

# Serial version
g++ traffic_serial.cpp -o traffic_serial -std=c++17 -fopenmp

## ⚡ How the Serial Version Works
*Executes the program step by step on a single CPU core.
*Each task is processed one after another, without parallelism.
*Takes more time compared to the parallel version for large data sets.

## ⚡ How the Parallel Version Works
*Uses OpenMP to split the work across multiple CPU cores.
*Each core processes a part of the data simultaneously, reducing total execution time.
*Results from all threads are combined after processing for the final output.



