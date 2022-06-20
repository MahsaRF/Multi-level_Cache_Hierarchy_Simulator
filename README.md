# Multi-level_Cache_Hierarchy_Simulator
Multi-level Cache Hierarchy Simulator written in C++

Project Overview 

In this project, I have implemented a flexible cache hierarchy simulator and used it to study 
the performance of memory hierarchies using the SPEC benchmarks. I have designed a generic 
cache simulator module with some configurable parameters. This cache module can be instantiated 
(used) as an L1 cache, an L2 cache, or an L3 cache, and so on. The source code implements a 
flexible two level cache memory hierarchy simulator using the CACHE module designed. The 
simulator will take an input in a standard format which describes the read/write requests 
from the processor.

•	  Configurable in terms of inclusion policies (Non-Inclusive, Inclusive and Exclusive) and 
    replacement policies (‘Least Recently Used’ and ‘Least Frequently Used’) in conjunction with Write-back or Write-through, with and without allocation.  
•	  Simulator configures the cache parameters such as block size, cache size and associativity

This project was submitted as part of a graduate level course (ECE 521: Computer Design and Technology)
taught by Professor Huiyang Zhou at the North Carolina State University.
