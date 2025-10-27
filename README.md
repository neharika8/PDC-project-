## Project Name

PARALLEL TRAFFIC ACCIDENT ANALYSIS (C++ OPENMP)

----

This project analyzes large traffic accident datasets using C++ and OpenMP.
It compares serial and parallel implementations to show how parallel processing improves performance and scalability.


## What's Inside:


| File | Description |
| ----------------------------- | ------------------------------------------------------------ |
| `traffic_serial.cpp` | Regular (non-parallel) version – analyzes accident data sequentially.|
| `traffic_parallel.cpp` | Parallel version using OpenMP – analyzes data faster using multiple threads. |

------
## ⚙️ Requirements
* **MinGW-w64 (GCC)** — make sure it supports OpenMP (`libgomp` is included in most versions).
* **C++17** support (needed for `std::filesystem`)
* **PowerShell** (for the commands below)

----------
## How to Build & Run
Open **PowerShell** in the project folder (where your `.cpp` files  are located).


### 1️. Compile
#  Serial version
g++ serial_accident_analysis.cpp -o serial
./serial


# Parallel version
g++  -fopenmp parallel_accident_analysis.cpp -o parallel_traffic_analysis
./parallel_traffic_analysis


## ⚡ How the Serial Version Works
* Executes the program step by step on a single CPU core.
* Each task is processed one after another, without parallelism.
* Takes more time compared to the parallel version for large data sets.

## ⚡ How the Parallel Version Works
* Uses OpenMP to split the work across multiple CPU cores.
* Each core processes a part of the data simultaneously, reducing total execution time.
* Results from all threads are combined after processing for the final output.

## 📊Performance Results

| Dataset               | Serial Time | Parallel Time | Speedup |
| --------------------- | ----------- | ------------- | ------- |
| Original (1M records) | 0.622 sec   | 0.13 sec      | 4.78×   |
| 50K records           | 0.086 sec   | 0.05 sec      | 1.72×   |

## Time Complexity
* Serial: O(N)
* Parallel: O(N/p)

--------
## Scalability
* Scales efficiently with larger datasets and multi-core CPUs.
* Minimal overhead for thread creation.
* Handles millions of records within seconds.
---------
## Future Scope
* **MPI Integration:** For distributed execution across multiple systems.
----------  
## References 
* **Freeway Accident Analysis using Second Order Statistics (LSU Repository)**
* **SpringerOpen – Analysis of Traffic Accident Characteristics**
* **Traffic Accident Analysis using Spark and Causal Inference**
## Conclusion
 * Parallelizing the traffic accident analysis using OpenMP resulted in a ~5× speed improvement over the serial approach.
 * This demonstrates the effectiveness of parallel programming for handling large datasets and real-world analytics.

