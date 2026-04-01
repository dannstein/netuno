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
- [ ] **DLS (Depth-Limited Search)**: DFS with a maximum depth constraint.
- [ ] **IDS (Iterative Deepening Search)**: Combines BFS completeness with DFS memory efficiency.
- [ ] **Bidirectional Search**: Simultaneous search from start and goal.

### 2. Informed Search (Weighted/Heuristic)
These algorithms use heuristics to guide the search towards the goal more efficiently:
- [ ] **UCS (Uniform Cost Search)**: Optimal pathfinding for weighted edges.
- [ ] **Greedy Best-First Search**: Uses heuristics for faster, but potentially non-optimal, results.
- [ ] **A* (A-Star)**: The gold standard, combining UCS and Greedy search for optimal results.
- [ ] **IDA* (Iterative Deepening A*)**: Memory-efficient version of the A* algorithm.

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
├── data/                 # Nothing (yet)
├── include/              # Header files (.hpp)
│   ├── core/             # Graph and Node logic
│   └── algorithms/       # AI Search classes
├── misc/                 # Example files (initial algorithms given from the teacher)
├── src/                  # Source files (.cpp)
│   ├── core/             # Implementation of the environment
│   ├── algorithms/       # Algorithm logic
│   └── main.cpp          # Application entry point
├── CMakeLists.txt        # Build configuration
└── README.md             # Project documentation
