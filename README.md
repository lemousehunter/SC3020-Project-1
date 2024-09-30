# Introduction
This repository contains the codebase for a disk-based B+ tree index database system. The data is first read from games.txt (if datablocks does not exist) into the storage module, after which datablocks mirroring those of physical datablocks are created, each containing the pre-defined number of records. Following which, the B+ tree index is created if it does not exist. The range search query function allows users to search for records with `fgPctHome` that are between 0.5 to 0.8 (inclusive). Based on our own tests, the results should be as follows:

```
--------------- B+ Tree Search Results ---------------
Number of index nodes accessed (internal, non-leaf node): 2
Number of data blocks accessed: 70
Number of results: 6902
Average FG3_PCT_home: 0.420801
Running time: 16816 microseconds
------------------------------------------------------

---------------- Linear Search Results ---------------
Number of data blocks accessed: 267
Number of results: 6902
Average FG3_PCT_home: 0.420801
Running time: 53134 microseconds
------------------------------------------------------
======================================================
```

For the following B+ Tree:
```
----------------- B+ Tree Statistics -----------------
Order (maximum number of keys per node): 100
Height of the tree: 3
Content of root node (keys): 0.39 0.418 0.436 0.452 0.468 0.486 0.506 
B+ Tree Node Count:
Total nodes: 465
Internal nodes: 9
Leaf nodes: 456
------------------------------------------------------
```

and Storage system:
```
----------------- Storage Statistics -----------------
Total number of records: 26651
Number of datablocks: 267
Records per datablock (pre-defined): 100
------------------------------------------------------
```

Initially, we faced some discrepancies with the timing (linear search being much faster than B+ tree search), which should not be the case. After some investigations, we found out that this was because of the extra time taken by the file `seekg()` and `read()` operations that B+ tree used but linear search did not, as our original implementation of linear search loaded the entire datablock into memory instead of reading it by offsets like B+ tree search was.

# How to Run
1. Make Clean
   ```bash
   make clean
   ```
2. Make
   ```bash
   make
   ```
3. Run the compiled executable file
   Assuming you are in the root folder of this repository:
   ```bash
   bin/bplustree
   ```
   
# Gotchas
1. You will need to cd to the root directory of this github repository before running the `bin/plustree` unix executable file, otherwise you will get the error `Error: Unable to open file: games.txt`
2. Always ensure you `make clean` before running the executable file for the first time
