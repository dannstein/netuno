# netuno
**Netuno project - Finding the shortest path in a blind environment**

Netuno is a C++ framework designed to simulate and visualize different Artificial Intelligence search algorithms on 2D grid maps. The project focuses on navigating "blind" or partially known environments to find the most efficient path between two points. The objective point is randomly defined, and the algorithm must find it blindly and, after found, trace the best path from the objetive to the origin.

---

## 🎓 Academic Context
This project was developed as a mandatory assignment for the **Artificial Intelligence I** course, taught by **Prof. Dr. Luis Fernando de Almeida**. 

It serves as a practical implementation of classic and modern search heuristics applied to graph theory and grid-based pathfinding.

---

## 🚀 Features
- **Dynamic Grid Generation**: Create maps of any dimensions (n x n).
- **Random Obstacle Generation**: Adjustable wall probability to test algorithm robustness.
- **Console Visualization (Phase 1)**: Real-time ASCII representation of the map and the discovered path.
- **Multiple Algorithms**: Modular structure to support up to 9 different search strategies.
- **C++ Performance**: Built with modern C++17 for high execution speed.

---

## 🧠 Search Algorithms Roadmap
The project implements a total of 9 algorithms, divided into two main categories:

### 1. Uninformed Search (Unweighted)
These algorithms explore the map without prior knowledge of the goal's location:
- [x] **BFS (Breadth-First Search)**: Guaranteed shortest path in unweighted grids.
- [x] **DFS (Depth-First Search)**: Deep exploration before backtracking.
- [x] **DLS (Depth-Limited Search)**: DFS with a maximum depth constraint.
- [x] **IDS (Iterative Deepening Search)**: Combines BFS completeness with DFS memory efficiency.
- [x] **Bidirectional Search**: Simultaneous search from start and goal.

### 2. Informed Search (Weighted/Heuristic)
These algorithms use heuristics to guide the search towards the goal more efficiently:
- [x] **UCS (Uniform Cost Search)**: Optimal pathfinding for weighted edges.
- [x] **Greedy Best-First Search**: Uses heuristics for faster, but potentially non-optimal, results.
- [x] **A* (A-Star)**: The gold standard, combining UCS and Greedy search for optimal results.
- [x] **IDA* (Iterative Deepening A*)**: Memory-efficient version of the A* algorithm.

---

## 🎨 Future Interface (GUI)
Currently, the project runs in the terminal. However, a **2D Pixel-Art Graphical Interface** is planned using the [SFML](https://sfml-dev.org) library, featuring:
- **Space Theme**: Map, nodes, and obstacles styled as celestial bodies and cosmic debris.
- **Interactive Controls**: Click to set start/end points.
- **Real-time Animation**: Watch the algorithm expand and find the path step-by-step.

---

## 🛠️ Project Structure
```text
netuno/
├── include/              # Header files (.hpp)
│   ├── core/             # Graph and Node logic
│   └── algorithms/       # AI Search classes
├── src/                  # Source files (.cpp)
│   ├── core/             # Implementation of the environment
│   ├── algorithms/       # Algorithm logic
│   └── main.cpp          # Application entry point
├── CMakeLists.txt        # Build configuration
└── README.md             # Project documentation
```

---

## ⚙️ How to Run

With the project already installed in your Linux system, follow the steps below to build and run **netuno_pathfinding**:

### 1. Prerequisites
Ensure you have the build tools and the **SFML** library installed. On Ubuntu/Debian, run:
```bash
sudo apt update
sudo apt install build-essential cmake libsfml-dev
```

### 2. Building the Project
It is recommended to perform an "out-of-source" build to keep the project structure clean. From the project root, run:

```bash
# Create and enter the build directory
mkdir -p build && cd build

# Configure the project
cmake ..

# Compile the executable
cmake --build .
```

### 3. Running the Application
Once the build is complete, you can launch the executable directly from the build folder:
```bash
./netuno_pathfinding
```

### 4. Cleaning and Rerunning
If you make structural changes to the CMakeLists.txt or need to reset the build environment entirely, use this command to clear the cache and recompile from scratch:
```bash
# Remove the build directory and start over
rm -rf build && mkdir build && cd build

# Reconfigure and Rebuild
cmake ..
cmake --build .

# Run again
./netuno_pathfinding
```

Make sure you are outside the build directory to run those commands. 

If you are already inside the build directory and just want to clear the compiled files without deleting the folder, run:
```bash
make clean
```

To wipe everything and reconfigure without leaving the build folder:
```bash
rm -rf * && cmake .. && cmake --build .
```


> **Note:** If you only modified your source code (`.cpp` or `.hpp` files), you don't need to clear the directory. Simply run `cmake --build .` inside the `build` folder to recompile only the changes.



