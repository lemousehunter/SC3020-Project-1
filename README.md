# Introduction
This repository contains the codebase for a disk-based B+ tree index database system. The data is first read from games.txt (if the database file does not exist) into the storage module, after which datablocks mirroring those of physical datablocks are created, each containing the maximum of 149 records (may vary from system to system depending on how large the `recordLocations` map in the datablock header is. All datablocks are stored in the same database file. Following which, the B+ tree index is created if it does not exist. The range search query function allows users to search for records with `fgPctHome` that are between 0.5 to 0.8 (inclusive). Based on our own tests on our own machine, the results should be as follows:

```
--------------- B+ Tree Search Results ---------------
Number of index nodes accessed (internal, non-leaf node): 2
Number of data blocks accessed: 55
Number of results: 6902
Average FG3_PCT_home: 0.420801
Running time: 9400 microseconds
------------------------------------------------------

---------------- Linear Search Results ---------------
Number of data blocks accessed: 209
Number of results: 6902
Average FG3_PCT_home: 0.420801
Running time: 34307 microseconds
------------------------------------------------------
```

For the following B+ Tree:
```
----------------- B+ Tree Statistics -----------------
Order (maximum number of keys per node): 100
Height of the tree: 3
Content of root node (keys): 0.393 0.419 0.438 0.453 0.469 0.488 0.506 
B+ Tree Node Count:
Total nodes: 459
Internal nodes: 9
Leaf nodes: 450
------------------------------------------------------
```

and Storage system:
```
----------------- Storage Statistics -----------------
Total number of records: 26651
Number of datablocks: 209
Size of record: 26 bytes
Size of record (in memory): 32 bytes
Size of record (with header): 27 bytes
Size of datablock: 4096 bytes
Size of datablock header: 614 bytes
Size of available space in datablock: 3482 bytes
Max Number of Records per Datablock: 128
Max used space in each Datablock: 4070 bytes
Unused space in each Datablock: 26 bytes
------------------------------------------------------
```
The schema of a datablock stored on disk (in the database file) is as follows:
```
┌───────────────────────────────────────────────────────┐
│             Datablock Schema (4096 bytes)             │
╞═══════════════════════════════════════════════════════╡
│Header                                 614 bytes       │
├────────────────┬──────────────────────────────────────┤
│id              │      unsigned short  2   bytes       │
│maxSize         │      unsigned short  2   bytes       │
│currentSize     │      unsigned short  2   bytes       │
│recordCount     │      unsigned short  2   bytes       │
│recordLocations │      unordered map   606 bytes       │
╞════════════════╧══════════════════════════════════════╡
│Record                                 27  bytes       │
╞═══════════════════════════════════════════════════════╡
│Record Header                          1   bytes       │
├────────────────┬──────────────────────────────────────┤
│size            │      int             1   bytes       │
╞════════════════╪══════════════════════════════════════╡
│gameDate        │      int             4   bytes       │
│teamId          │      int             4   bytes       │
│ptsHome         │      unsigned char   1   bytes       │
│fgPctHome       │      float           4   bytes       │
│ftPctHome       │      float           4   bytes       │
│fg3PctHome      │      float           4   bytes       │
│astHome         │      unsigned char   1   bytes       │
│rebHome         │      unsigned char   1   bytes       │
│homeTeamWins    │      bool            1   bytes       │
│recordId        │      unsigned short  2   bytes       │
╞════════════════╧══════════════════════════════════════╡
│                          ...                          │
│                          ...                          │
│                      Other Records                    │
│                          ...                          │
│                          ...                          │
╞═══════════════════════════════════════════════════════╡
│               Unused Space (26 bytes)                 │
└───────────────────────────────────────────────────────┘
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

   **For Windows:** <i>Reference: <a href = "https://code.visualstudio.com/docs/cpp/config-mingw">Using GCC with MinGW</a></i><br>
   
   a. Installing the MinGW-w64 toolchain <a href = "https://www.msys2.org/">here</a>.<br>
   
   b. In the wizard, choose your desired Installation Folder. Record this directory for later. In most cases, the recommended directory is acceptable. The same applies when you get to setting the start menu shortcuts step. When complete, ensure the <b>Run MSYS2</b> now box is checked and select <b>Finish</b>. This will open a MSYS2 terminal window for you.<br>
   
   c. In the `MSYS2 UCRT64 environment` <b>(not MinGW64 or MinGW32)</b> terminal paste this command to download the MinGW-w64 toolchain.<br>
   ```
   pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
   ```
   
   d. Accept the default number of packages in the toolchain by pressing `Enter`.<br>
   ![image](https://github.com/user-attachments/assets/8d1a8d78-5f8d-4290-a129-c88317e98d64)

   e. Press `Y` when prompted to proceed with the installation.<br>
   
   f. Add the path of your MinGW-w64 `bin` folder to the Windows `PATH` environment variable by using the following steps:<br> 
   - f.1. In the Windows search bar, type <b>Settings</b> to open your Windows Settings.<br>
   
   - f.2. Search for <b>Edit environment variables for your account</b>.<br>
   
   - f.3. In your <b>User variables</b>, select the `Path` variable and then select <b>Edit</b>.<br>
   
   - f.4. Select <b>New</b> and add the MinGW-w64 destination folder you recorded during the installation process to the list. If you used the default settings above, then this will be the path: `C:\msys64\ucrt64\bin`.<br>
   
        f.5. Select <b>OK</b>, and then select <b>OK</b> again in the <b>Environment Variables</b> window to update the `PATH` environment variable. You have to reopen any console windows for the updated `PATH` environment variable to be available<br>

    g. To check if g++, GCC, gdb has been installed correctly, open a new <b>command prompt (CMD)</b> and paste in these lines
    ```bash
    gcc --version
    g++ --version
    gdb --version
    ```




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

<b><i>Please download the correct version based on your machine OS</i></b>



File Structure:

```
.../SC3020-Lab1
│
├─────── Mac
│         ├─────── include
│         │          ├──...
│         │          │
│         │         ...
│         │
│         ├─────── src
│         │          ├──...
│         │          │
│         │         ...
│         │
│         ├─────── Makefile
│         ├─────── games.txt
│
├───────Windows
│         ├─────── include
│         │          ├──...
│         │          │
│         │         ...
│         │
│         ├─────── src
│         │          ├──...
│         │          │
│         │         ...
│         │
│         ├─────── Makefile
│         ├─────── games.txt
```

1. `cd` into the root directory of this git repository `SC3020-Lab1`
2. `cd` into the directory corresponding to your Operating System:
   
   **For Windows**<br>
   
   - First open the `MSYS2 UCRT64 environment` terminal:<br>
     <img width="290" alt="image" src="https://github.com/user-attachments/assets/a971ff81-0e26-4961-b678-60921e477484">
     
     <i>Note: If you cannot find the file just open the search bar next to the Windows logo on the taskbar and search for `MYSYS2 UCRT64`</i><br>
     <img width="282" alt="image" src="https://github.com/user-attachments/assets/2872bc15-faca-4da5-bde1-30ba6e22f3e1">

     
  
   ```bash
   cd Windows # or path to the corresponding to Windows
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
7. Run the compiled executable file
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
