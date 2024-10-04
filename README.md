# Introduction
This repository contains the codebase for a disk-based B+ tree index database system. The data is first read from games.txt (if the database file does not exist) into the storage module, after which datablocks mirroring those of physical datablocks are created, each containing the maximum of 149 records (may vary from system to system depending on how large the `recordLocations` map in the datablock header is. All datablocks are stored in the same database file. Following which, the B+ tree index is created if it does not exist. The range search query function allows users to search for records with `fgPctHome` that are between 0.5 to 0.8 (inclusive). Based on our own tests on our own machine, the results should be as follows:

```
--------------- B+ Tree Search Results ---------------
Number of index nodes accessed (internal, non-leaf node): 2
Number of data blocks accessed: 70
Number of results: 6902
Average FG3_PCT_home: 0.420801
Running time: 7342 microseconds
------------------------------------------------------

---------------- Linear Search Results ---------------
Number of data blocks accessed: 267
Number of results: 6902
Average FG3_PCT_home: 0.420801
Running time: 28026 microseconds
------------------------------------------------------
```

For the following B+ Tree:
```
----------------- B+ Tree Statistics -----------------
Order (maximum number of keys per node): 100
Height of the tree: 3
Content of root node (keys): 0.391 0.418 0.437 0.453 0.469 0.487 0.506 
B+ Tree Node Count:
Total nodes: 461
Internal nodes: 9
Leaf nodes: 452
------------------------------------------------------
```

and Storage system:
```
----------------- Storage Statistics -----------------
Total number of records: 26651
Number of datablocks: 267
Size of record: 26 bytes
Size of record (in memory): 32 bytes
Size of record (with header): 27 bytes
Size of datablock: 4096 bytes
Size of available space in datablock: 4048 bytes
Max Number of Records per Datablock: 149
------------------------------------------------------
```
The schema of a datablock stored on disk (in the database file) is as follows:
```
┌──────────────────────────────────────────────────────┐
│             Datablock Schema (4096 bytes)            │
╞══════════════════════════════════════════════════════╡
│Header                                 48 bytes       │
├────────────────┬─────────────────────────────────────┤
│id              │      unsigned short  2  bytes       │
│maxSize         │      unsigned short  2  bytes       │
│currentSize     │      unsigned short  2  bytes       │
│recordCount     │      unsigned short  2  bytes       │
│recordLocations │      unordered map   40 bytes       │
╞════════════════╧═════════════════════════════════════╡
│Record                                 1  bytes       │
╞══════════════════════════════════════════════════════╡
│Record Header                          1  bytes       │
├────────────────┬─────────────────────────────────────┤
│size            │      int             1  bytes       │
╞════════════════╪═════════════════════════════════════╡
│gameDate        │      int             4  bytes       │
│teamId          │      int             4  bytes       │
│ptsHome         │      unsigned char   1  bytes       │
│fgPctHome       │      float           4  bytes       │
│ftPctHome       │      float           4  bytes       │
│fg3PctHome      │      float           4  bytes       │
│astHome         │      unsigned char   1  bytes       │
│rebHome         │      unsigned char   1  bytes       │
│homeTeamWins    │      unsigned short  1  bytes       │
│recordId        │      unsigned short  2  bytes       │
╞════════════════╧═════════════════════════════════════╡
│                          ...                         │
│                          ...                         │
│                      Other Records                   │
│                          ...                         │
│                          ...                         │
└──────────────────────────────────────────────────────┘
```

Compiled with g++:
```
Apple clang version 15.0.0 (clang-1500.3.9.4)
Target: arm64-apple-darwin23.6.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
```

Initially, we faced some discrepancies with the timing (linear search being much faster than B+ tree search), which should not be the case. After some investigations, we found out that this was because of the extra time taken by the file `seekg()` and `read()` operations that B+ tree used but linear search did not, as our original implementation of linear search loaded the entire datablock into memory instead of reading it by offsets like B+ tree search was.

# Pre-requsites:
1. Ensure G++ compiler is installed

   **For Windows:**<br>
   https://ghost-together.medium.com/how-to-install-mingw-gcc-g-compiler-on-windows-f7c805747a00
   
   <br>**For Mac:**<br>

   Via MacPorts:
   ```bash
   sudo port install gcc9
   ```

   Via Homebrew:
   ```bash
   brew install gcc
   ```

   <br>**For Linux:**<br>
   ```bash
   sudo apt-get update && sudo apt-get install g++
   ```
   
# How to Run
1. `cd` into the root directory of this git repository
2. `cd` into the directory corresponding to your Operating System:
   
   **For Windows**<br>
   ```bash
   cd Windows
   ```
   
   **For Mac/Linux:**<br>
   ```bash
   cd Mac
   ```
4. Make Clean
   ```bash
   make clean
   ```
5. Make
   ```bash
   make
   ```
6. Run the compiled executable file
   Assuming you are in the root folder of this repository:

   **For Windows:**
   ```bash
   bin/bplustree.exe
   ```

   **For Linux / Mac:**
   ```bash
   bin/bplustree
   ```
   **Disclaimer:** No tests was run on a Linux machine and hence no guarantees can be made that the code can be executed on Linux machines. 
   
# Gotchas
1. You will need to cd to the root directory of this github repository before running the `bin/plustree` unix executable file, otherwise you will get the error `Error: Unable to open file: games.txt`
2. Always ensure you `make clean` before running the executable file for the first time
