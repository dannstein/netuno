# Netuno
**Netuno project - Finding the shortest path in a blind environment**

Netuno is a C++ framework designed to simulate and visualize different Artificial Intelligence search algorithms on 2D grid maps. The project focuses on navigating "blind" or partially known environments to find the most efficient path between two points. The objective point is randomly defined, and the algorithm must find it blindly and, after found, trace the best path from the objective back to the origin.

---

## Academic Context
This project was developed as a mandatory assignment for the **Artificial Intelligence I** course, taught by **Prof. Dr. Luis Fernando de Almeida**.

It serves as a practical implementation of classic and modern search heuristics applied to graph theory and grid-based pathfinding.

> **Important:** All search algorithms in this project were **originally provided in Python by Prof. Dr. Luis Fernando de Almeida** as part of the course material. The project authors **translated them from Python to C++** — no algorithmic authorship is claimed by the students.
>
> The **graphical interface (GUI)** — including the SFML window, sprite rendering, animation system, HUD, camera controls, game state machine, and tutorial screen — was **vibe coded** (AI-assisted development using Claude) by the project authors. The only thing not vibe coded from the interface are the 2d pixel sprites.

---

## Features

### Algorithms (provided by Prof. Dr. Luis Fernando de Almeida)
- 9 search algorithms across two categories (uninformed and informed)
- Weighted and unweighted graph support
- Custom heuristic function with directional bias and wormhole-aware path estimation

### Graphical Interface (vibe coded)
- **Space-themed pixel-art GUI** built with [SFML 2.5](https://sfml-dev.org)
- **Zoomable, pannable grid** — scroll wheel to zoom, click-drag to pan
- **Animated ship** that follows the path with smooth interpolation and directional rotation
- **Two exploration phases**: blue path overlay for exploration, gold for the return trip to base
- **Rich weighted terrain** with 8 distinct cell types (asteroids, black holes, nebulas, pulsars, wormholes)
- **HUD sidebar** showing algorithm, mode, ship position, total cost, and fuel gauge
- **Fuel mechanic** — configurable max steps to prevent unbounded search on blocked maps
- **In-game tutorial** explaining every cell type, weight, and algorithm

---

## Search Algorithms

The project implements 9 algorithms in two categories:

### Uninformed Search (Unweighted mode)
These algorithms explore the map without prior knowledge of the goal's location:

| Algorithm | Description |
|---|---|
| **BFS** (Breadth-First Search) | Explores level by level; guarantees shortest path by hop count |
| **DFS** (Depth-First Search) | Goes as deep as possible before backtracking; not optimal |
| **DLS** (Depth-Limited Search) | DFS with a configurable maximum depth cap |
| **IDS** (Iterative Deepening Search) | Repeats DLS with increasing depth; optimal like BFS, low memory |
| **BDS** (Bidirectional Search) | Searches simultaneously from start and goal, meets in the middle |

### Informed Search (Weighted mode)
These algorithms use a heuristic to guide the search more efficiently:

| Algorithm | Description |
|---|---|
| **UCS** (Uniform Cost Search) | Expands cheapest paths first; guarantees optimal cost |
| **Greedy Best-First** | Follows heuristic only; fast but not always optimal |
| **A\*** (A-Star) | Combines UCS and heuristic; optimal and faster than UCS |
| **AIA\*** (Anytime Incremental A\*) | Tightens cost bound iteratively; finds optimal path progressively |

---

## Weighted Terrain

In weighted mode the grid is procedurally generated with the following cell types:

| Cell | Weight | Description |
|---|---|---|
| Open Space | 1–49 | Random passable terrain, most common |
| Asteroid | Blocked | Impassable wall |
| Nebula | 15–25 | Interstellar gas cloud, moderate slowdown |
| Wormhole | 1 | Teleports to its paired wormhole instantly |
| Pulsar Jet | 55 | Radiation stream extending 2 cells from the core |
| Pulsar Center | 75 | Neutron star core |
| Black Hole Ring 2 | 50 | Outer gravitational field (16 surrounding cells) |
| Black Hole Ring 1 | 80 | Strong gravitational pull (8 cells adjacent to core) |
| Black Hole Core | 100 | Singularity — highest possible movement cost |

---

## Controls (in-game)

| Key / Action | Effect |
|---|---|
| `SPACE` | Pause / resume auto-advance |
| `RIGHT ARROW` | Advance one step manually |
| `ENTER` | Confirm / proceed to results when done |
| `ESC` | Return to main menu |
| Scroll wheel | Zoom in / out on the grid |
| Click + drag | Pan the camera |

---

## Project Structure
```text
netuno/
├── include/
│   ├── core/             # Graph, W_Graph, Node, W_Node
│   ├── algorithms/       # One header per algorithm
│   └── gui/              # GameState, Renderer
├── src/
│   ├── core/             # Graph and W_Graph 
│   ├── algorithms/       # Algorithm implementations
│   ├── gui/          
│   └── main.cpp          # SFML window, state machine, 
├── CMakeLists.txt
└── README.md
```

---

## How to Run

### Prerequisites
Ensure you have the build tools and SFML installed. On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake libsfml-dev
```

### Build
```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

### Run
```bash
./netuno_pathfinding
```

The executable resolves the `sprites/` directory relative to its own location, so it can be launched from any working directory.

### Cleaning and Rerunning
If you make structural changes to the `CMakeLists.txt` or need to reset the build environment entirely, use this command to clear the cache and recompile from scratch:
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
