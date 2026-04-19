# STARLOG — Pathfinding Algorithms Reference

> A detailed reference for every data structure and algorithm implemented in the **Netuno** project.
> Written for future study: each section explains *what*, *why*, and *how* a piece works.

---

## Table of Contents

1. [Data Structures](#1-data-structures)
   - 1.1 [Node (unweighted)](#11-node-unweighted)
   - 1.2 [W_Node (weighted)](#12-w_node-weighted)
   - 1.3 [CellType Namespace](#13-celltype-namespace)
   - 1.4 [Graph (unweighted)](#14-graph-unweighted)
   - 1.5 [W_Graph (weighted)](#15-w_graph-weighted)
2. [Unweighted Algorithms](#2-unweighted-algorithms)
   - 2.1 [BFS — Breadth-First Search](#21-bfs--breadth-first-search)
   - 2.2 [DFS — Depth-First Search](#22-dfs--depth-first-search)
   - 2.3 [DLS — Depth-Limited Search](#23-dls--depth-limited-search)
   - 2.4 [IDS — Iterative Deepening Search](#24-ids--iterative-deepening-search)
   - 2.5 [BDS — Bidirectional Search](#25-bds--bidirectional-search)
3. [Weighted Algorithms](#3-weighted-algorithms)
   - 3.1 [UCS — Uniform-Cost Search](#31-ucs--uniform-cost-search)
   - 3.2 [Greedy — Best-First Search](#32-greedy--best-first-search)
   - 3.3 [A*](#33-a)
   - 3.4 [AIA* — Incremental A*](#34-aia--incremental-a)
4. [Algorithm Comparison](#4-algorithm-comparison)

---

## 1. Data Structures

### 1.1 Node (unweighted)

**File:** `include/core/Node.hpp`

```cpp
struct Node {
    Node*  parent;
    int    state;
    float  v1;
    Node*  previous;
    Node*  next;
    int    x, y;
    float  h;
    bool   visited;
};
```

`Node` is the building block of the **unweighted** grid. Every cell in the `Graph`
is stored as a `Node` object.

| Field      | Type     | Role |
|------------|----------|------|
| `parent`   | `Node*`  | Pointer to the node this one was reached from. Used by `showPath` to reconstruct the path by walking backwards from goal to start. |
| `state`    | `int`    | Cell type: `0` = open, `1` = wall/asteroid. Used by `sucessors_grid` to skip impassable neighbours. |
| `v1`       | `float`  | Depth / step-count from the start node. Used by DLS and IDS as the "how deep are we" counter. BFS also fills it but uses `visited` as the primary expansion guard. |
| `previous` | `Node*`  | Reserved for doubly-linked list usage (not used by current algorithms). |
| `next`     | `Node*`  | Same — reserved. |
| `x`, `y`   | `int`    | Grid coordinates. `x` = column (left→right), `y` = row (top→bottom). |
| `h`        | `float`  | Heuristic value — not used by unweighted algorithms (always 0). |
| `visited`  | `bool`   | Expansion guard. Once `true`, the node is never pushed onto the frontier again, preventing loops. |
| `getF()`   | method   | Returns `v1 + h`. Not used in unweighted search but available for future informed variants. |

**Lifecycle in a search:**
1. `Graph` constructor: all nodes created with defaults (`state=0`, `v1=0`, `visited=false`).
2. Before each search: `graph.reset()` sets `parent=nullptr`, `visited=false`, `v1=0`.
3. During search: algorithm sets `parent`, increments `v1`, marks `visited=true`.
4. After goal found: `showPath` walks the `parent` chain backwards to build the result vector.

---

### 1.2 W_Node (weighted)

**File:** `include/core/W_Node.hpp`

```cpp
struct W_Node {
    W_Node* parent;
    int     state;
    float   g;
    float   h;
    int     x, y;
    float   weight;
    bool    visited;
    W_Node* warp_target;
};
```

`W_Node` is the weighted counterpart of `Node`. Each field has a precise role in
cost-based search.

| Field         | Type      | Role |
|---------------|-----------|------|
| `parent`      | `W_Node*` | Same as `Node::parent` — used to reconstruct the path. |
| `state`       | `int`     | Cell type code from the `CellType` namespace (0–8). |
| `g`           | `float`   | **Accumulated cost from start to this node.** Equivalent to `v2` in the Python source. Initialised to `∞` at the start of each weighted search so any real cost is immediately an improvement. |
| `h`           | `float`   | **Heuristic estimate from this node to the goal.** Set by calling `W_Graph::heuristic()`. Zero for UCS (not informed). |
| `x`, `y`      | `int`     | Grid coordinates, same convention as `Node`. |
| `weight`      | `float`   | **Cost to *enter* this cell.** The edge cost in the weighted graph. Open space: 1–49 (random). Asteroid: `∞`. Other cells: fixed values per `CellType`. |
| `visited`     | `bool`    | "Closed set" flag. Once a node is popped from the priority queue and expanded, it is marked `visited=true` and never re-expanded (lazy-deletion pattern). |
| `warp_target` | `W_Node*` | Non-null **only** for `WORMHOLE` cells. Points to the paired exit cell. The two wormhole cells point to each other. |
| `getF()`      | method    | Returns `g + h`. Used as the priority key in A* and AIA*. |
| `isPassable()`| method    | Returns `state != CellType::ASTEROID`. Called by `sucessors_grid` to filter out impassable neighbours. |

**Key design decision — `g = ∞` initialisation:**
Unlike the unweighted `Node` where `v1=0` means "not yet visited", `W_Node::g`
must start at `∞` so the condition `new_g < child->g` correctly identifies both
freshly discovered nodes and nodes reachable via a cheaper path.

---

### 1.3 CellType Namespace

**File:** `include/core/W_Node.hpp`

```cpp
namespace CellType {
    constexpr int OPEN           = 0;   // weight 1–49
    constexpr int ASTEROID       = 1;   // impassable (weight ∞)
    constexpr int BLACK_HOLE     = 2;   // weight 100
    constexpr int BLACK_HOLE_R1  = 3;   // weight 80
    constexpr int BLACK_HOLE_R2  = 4;   // weight 50
    constexpr int WORMHOLE       = 5;   // teleporter (weight 1)
    constexpr int NEBULA         = 6;   // weight 15–25
    constexpr int PULSAR_CENTER  = 7;   // weight 75
    constexpr int PULSAR_JET     = 8;   // weight 55
}
```

These constants are the vocabulary of the weighted grid. Using a namespace (rather
than an enum or bare `#define`s) keeps them scoped while remaining implicitly
convertible to `int` for use in `switch` statements and the `state` field.

**Cell weight summary:**

| Cell             | Weight   | Passable | Notes |
|------------------|----------|----------|-------|
| `OPEN`           | 1 – 49   | Yes      | Random per cell, simulates varying space terrain |
| `ASTEROID`       | ∞        | **No**   | Hard wall |
| `BLACK_HOLE`     | 100      | Yes      | 1 center cell, extreme gravity |
| `BLACK_HOLE_R1`  | 80       | Yes      | 8 surrounding cells (Chebyshev dist = 1) |
| `BLACK_HOLE_R2`  | 50       | Yes      | 16 outer cells (Chebyshev dist = 2) |
| `WORMHOLE`       | 1        | Yes      | Instant teleport to paired cell |
| `NEBULA`         | 15 – 25  | Yes      | Random per cell, gas cloud |
| `PULSAR_CENTER`  | 75       | Yes      | Neutron star core |
| `PULSAR_JET`     | 55       | Yes      | Radiation jet (4 cardinal directions, 2 cells each) |

---

### 1.4 Graph (unweighted)

**Files:** `include/core/Graph.hpp`, `src/core/Graph.cpp`

```cpp
class Graph {
public:
    int nx, ny;
    std::vector<std::vector<Node>> grid;
    // ...
};
```

`Graph` is a 2-D grid of `Node` objects stored in a `vector<vector<Node>>`.
`grid[x][y]` gives the node at column `x`, row `y`.

**Key methods:**

#### `Graph(int x, int y)`
Allocates an `nx × ny` grid and calls `Node(i, j)` for every cell to stamp each
node with its correct coordinates.

#### `void reset()`
Resets search-state fields (`parent`, `visited`, `v1`) without touching `state`.
This allows the same map to be searched by multiple algorithms back-to-back
without rebuilding it.

#### `std::vector<Node*> sucessors_grid(Node* current)`
Returns pointers to the (up to 4) non-wall 4-directional neighbours of `current`.
Only cells with `state == 0` are returned — walls (`state == 1`) are filtered out.
All unweighted algorithms call this to expand a node.

```
dx = {0, 0, 1, -1}
dy = {1,-1, 0,  0}   ← 4-connectivity: up, down, right, left
```

#### `void generateRandomMap(int wallProbability)`
Uses `std::mt19937` seeded with `time(0)` to set each cell to `state=1` (wall)
with probability `wallProbability/100`, otherwise `state=0` (open).

#### `std::vector<Node*> showPath(Node* lastNode)`
Walks the `parent` chain from the goal back to the start and reverses the
resulting vector. Returns an ordered start→goal path.

#### `void drawPath(const std::vector<Node*>& path)`
Renders the grid with `#` for walls and `X` for cells on the found path.

---

### 1.5 W_Graph (weighted)

**Files:** `include/core/W_Graph.hpp`, `src/core/W_Graph.cpp`

```cpp
class W_Graph {
public:
    int nx, ny;
    std::vector<std::vector<W_Node>> grid;
private:
    W_Node* wormhole_a;   // cached wormhole pointer for O(1) heuristic use
    // ...
};
```

`W_Graph` mirrors `Graph` but operates on `W_Node` and carries the full set of
space-themed obstacles.

**Key methods:**

#### `void generateRandomMap(int wallProbability)`

1. **Base pass:** fills every cell as either `ASTEROID` (weight `∞`) or `OPEN`
   (weight 1–49 uniform random).
2. **Black hole** (15% chance): calls `placeBlackHole(cx, cy)` which stamps a
   5×5 region centred at `(cx,cy)` using Chebyshev distance — dist=0 → center
   (weight 100), dist=1 → ring 1 (weight 80), dist=2 → ring 2 (weight 50).
3. **Wormhole** (15% chance): calls `placeWormhole(ax,ay,bx,by)` which stamps
   both cells as `WORMHOLE` and sets `warp_target` on each to point to the other.
   Also caches `wormhole_a` for the heuristic.
4. **Nebula** (25% chance): calls `placeNebula(cx,cy,size,rng)` which does a
   random walk of `size` steps from `(cx,cy)`, converting `OPEN` cells to
   `NEBULA` with a random weight in `[15, 25]`.
5. **Pulsar** (15% chance): calls `placePulsar(cx,cy)` which places the center
   (weight 75) and up to 8 jet cells (2 steps in each cardinal direction,
   weight 55), only overwriting `OPEN` cells.

#### `std::vector<W_Node*> sucessors_grid(W_Node* current)`

Identical 4-connectivity logic to `Graph::sucessors_grid`, filtering by
`isPassable()`. **Extra step:** if `current` is a `WORMHOLE`, `warp_target` is
appended as an additional successor. This means the wormhole teleport is
transparent to the search algorithm — it just appears as an extra cheap edge.
The algorithm's closed-set (`visited` flag) prevents the teleport from looping.

#### `float getEdgeCost(W_Node* from, W_Node* to) const`

Returns `to->weight`. This is the cost to *enter* cell `to`. The `from`
parameter is kept for API symmetry but is not currently used (edge cost depends
only on the destination). Asteroid cells return `∞`.

#### `float heuristic(W_Node* from, W_Node* goal) const`

Computes a **directional weighted Euclidean distance** between two nodes:

```
c1 = (goal.x < from.x) ? 3 : 2      ← moving left is more expensive
c2 = (goal.y < from.y) ? 7 : 5      ← moving down is more expensive
h  = sqrt(c1·dx² + c2·dy²)
```

The asymmetry models a galactic drift / stellar wind: moving right and up is
cheaper. This is an **admissible** heuristic for all open cells with weight ≥ 2
(the minimum c1). For cells with weight 1 it may slightly overestimate —
acceptable as a near-admissible heuristic in practice.

**Wormhole shortcut optimization:** if a wormhole pair exists on the board,
`wormhole_a` is non-null. The heuristic checks whether routing through either
end of the wormhole yields a lower estimate:

```
via_a = wdist(from, wormhole_a) + wdist(wormhole_b, goal)
via_b = wdist(from, wormhole_b) + wdist(wormhole_a, goal)
h     = min(h_direct, via_a, via_b)
```

This is O(1) because `wormhole_a` is a cached pointer — no grid scan is needed.

#### `void drawPath(const std::vector<W_Node*>& path)`

Unlike the unweighted `drawPath` (which used a `char` grid), this version prints
directly from `grid[i][j].state` using the same `switch` as `drawConsole`,
overlaying ` X ` for path cells. This preserves the full visual context (black
holes, nebulae, pulsars) around the found route.

---

## 2. Unweighted Algorithms

All unweighted algorithms share the same contract:
- Accept a `Graph&` and start/end coordinates.
- Return `std::vector<Node*>` — the ordered path, or `{}` if unreachable.
- Call `graph.reset()` internally (or rely on the caller) before modifying nodes.
- All edge costs are implicitly 1 (one step = one unit of depth).

---

### 2.1 BFS — Breadth-First Search

**File:** `src/algorithms/BFS.cpp`

**Core idea:** explore the graph level by level. All nodes at depth *d* are
visited before any node at depth *d+1*. Uses a **FIFO queue** (deque, push back,
pop front).

**Guarantee:** finds the **shortest path** (fewest edges) when all edge costs are
equal.

**Step-by-step:**

```
1.  Push start onto queue; mark start.visited = true.
2.  While queue not empty:
    a. Pop front node as `current`.
    b. For each unvisited neighbour `child`:
         - Set child.parent = current
         - Set child.v1     = current.v1 + 1
         - Mark child.visited = true
         - Push child onto queue
         - If child == goal → call showPath(child) and return
3.  Queue empty → return {} (no path).
```

**Key implementation detail — early exit:**
BFS checks for the goal *when a node is pushed*, not when it is popped. This
avoids expanding a full level unnecessarily once the goal is found.

**Data structure used:** `std::deque<Node*>` (`push_back` + `pop_front`).

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes (finite graph) |
| Optimal     | Yes (uniform cost = 1) |
| Time        | O(b^d) — b = branching factor, d = depth of solution |
| Space       | O(b^d) — stores entire frontier |

---

### 2.2 DFS — Depth-First Search

**File:** `src/algorithms/DFS.cpp`

**Core idea:** dive as deep as possible before backtracking. Uses a **LIFO stack**
(push and pop from top).

**Guarantee:** will find *a* path if one exists, but **not necessarily the
shortest one**.

**Step-by-step:**

```
1.  Push start onto stack; mark start.visited = true.
2.  While stack not empty:
    a. Pop top node as `current`.
    b. If current == goal → call showPath(current) and return.
    c. For each unvisited neighbour `child`:
         - Set child.parent = current
         - Set child.v1     = current.v1 + 1
         - Mark child.visited = true
         - Push child onto stack
3.  Stack empty → return {}.
```

**Why DFS checks goal on pop (not push):**
BFS can check on push because it processes nodes in order. DFS pops nodes out of
order (LIFO), so children pushed last are explored first — checking on push would
give wrong `parent` chains. The check on pop is correct but means DFS may push
the goal before visiting it.

**Data structure used:** `std::stack<Node*>`.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes (with visited tracking to avoid cycles) |
| Optimal     | **No** |
| Time        | O(b^m) — m = maximum depth of graph |
| Space       | O(b·m) — only one path in memory at a time |

---

### 2.3 DLS — Depth-Limited Search

**File:** `src/algorithms/DLS.cpp`

**Core idea:** DFS with a hard depth cap. Nodes at depth `v1 == limit` are
expanded no further. Uses the same **LIFO stack** as DFS.

**Why it exists:** unbounded DFS can loop indefinitely (even with `visited`
tracking, it may waste time in very deep branches). DLS gives the user control.

**Step-by-step:**

```
1.  Push start; mark visited.
2.  While stack not empty:
    a. Pop `current`.
    b. If current == goal → return path.
    c. If current.v1 < limit:          ← depth gate
         expand children (same as DFS)
3.  Stack empty → return {}.
```

**The depth gate:** `current.v1 < limit` means children are only generated when
the current node has NOT yet reached the limit. A node AT the limit is a leaf
even if it has unvisited neighbours.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Only if solution depth ≤ limit |
| Optimal     | **No** |
| Time        | O(b^l) — l = limit |
| Space       | O(b·l) |

---

### 2.4 IDS — Iterative Deepening Search

**File:** `src/algorithms/IDS.cpp`

**Core idea:** run DLS repeatedly with limits `1, 2, 3, ..., max_limit`. The first
run that finds the goal returns the path. Combines BFS's optimality with DFS's
low memory footprint.

**Step-by-step:**

```
For i = 1 to max_limit:
    graph.reset()                   ← clear all visited flags
    run DLS with limit = i
    if goal found → return path
return {}
```

**Why reset every iteration:** IDS re-explores nodes at every depth. Without
reset, nodes marked `visited` in iteration *i* would be silently skipped in
iteration *i+1*, breaking the search.

**Why it's optimal:** the first time the goal is found is at the minimum depth
(since every shallower depth was already exhausted). Equal to BFS on optimal
depth, but uses only O(b·d) memory.

**The apparent inefficiency:** nodes near the root are re-expanded many times
(the root is expanded `max_limit` times). In practice this overhead is dominated
by the exponential growth of the frontier — the vast majority of work is in the
last iteration, making the total cost only a constant factor above a single BFS.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes (if solution exists and max_limit is large enough) |
| Optimal     | Yes (uniform cost = 1) |
| Time        | O(b^d) |
| Space       | O(b·d) — **much better than BFS** |

---

### 2.5 BDS — Bidirectional Search

**File:** `src/algorithms/BDS.cpp`

**Core idea:** run two simultaneous BFS expansions — one forward from the start,
one backward from the goal. The search terminates when the frontiers meet.
Dramatically reduces the search space: instead of O(b^d), each frontier
only needs to reach depth d/2 → O(b^(d/2)) total.

**Data structures used:**
- Two deques (`dq1` from start, `dq2` from goal).
- Two hash maps keyed on `{x, y}` pairs storing each node's **predecessor in
  its own direction**: `visited1[pos] = who-pushed-me-from-start`,
  `visited2[pos] = who-pushed-me-from-goal`.

**Step-by-step:**

```
1.  dq1 = {start};  visited1[start] = nullptr
    dq2 = {goal};   visited2[goal]  = nullptr

2.  While both queues non-empty:
    a. Expand one full level from dq1:
         for each neighbour of current:
           if neighbour ∈ visited2 → MEET! call buildPath and return
           if not in visited1 → add to visited1, push onto dq1

    b. Expand one full level from dq2:
         for each neighbour of current:
           if neighbour ∈ visited1 → MEET! call buildPath and return
           if not in visited2 → add to visited2, push onto dq2
```

**`buildPath(v1, v2, n1, n2)` — path stitching:**
When the frontiers meet at an edge `(n1 → n2)` (n1 from the forward search,
n2 from the backward search), the function:
1. Walks `visited1` backwards from `n1` to the start (building the first half).
2. Walks `visited2` backwards from `n2` to the goal (building the second half).
3. Reverses the first half and appends the second half.

**Important:** BDS uses its own `visited` maps (not `Node::visited`) because the
two frontiers would conflict if they shared the same flag.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes |
| Optimal     | Yes (uniform cost = 1) |
| Time        | O(b^(d/2)) |
| Space       | O(b^(d/2)) |

---

## 3. Weighted Algorithms

All weighted algorithms share the same contract:
- Accept a `W_Graph&` and start/end coordinates.
- Return `std::vector<W_Node*>` — ordered path, or `{}`.
- Initialise **all** node `g` values to `∞` at the start, then set `start.g = 0`.
- Use **lazy deletion** in the priority queue: when a node is popped, check
  `current->visited`. If already true, skip it (it was re-inserted with a higher
  cost before a better path was found). Only mark `visited=true` on first pop.
- Call `graph.getEdgeCost(current, child)` which returns `child->weight`.
- Handle wormholes transparently — `sucessors_grid` adds `warp_target` as an
  extra successor.

---

### 3.1 UCS — Uniform-Cost Search

**File:** `src/algorithms/UCS.cpp`

**Core idea:** always expand the node with the **lowest accumulated cost `g`**
from the start. This is Dijkstra's algorithm adapted to a goal-directed search.
No heuristic is used — the algorithm is purely cost-driven.

**Priority queue:** min-heap on `g`. Comparator:
```cpp
struct CompareG {
    bool operator()(const W_Node* a, const W_Node* b) const {
        return a->g > b->g;   // greater-than → min-heap
    }
};
```

**Step-by-step:**

```
1.  All g = ∞. start.g = 0. Push start.
2.  While pq not empty:
    a. Pop node with lowest g as `current`.
    b. If current.visited → skip (stale entry).
    c. Mark current.visited = true.
    d. If current == goal → return showPath(current).
    e. For each passable neighbour `child`:
         new_g = current.g + getEdgeCost(current, child)
         if new_g < child.g:
             child.g      = new_g
             child.parent = current
             push child onto pq
3.  pq empty → return {}.
```

**Why `g = ∞` matters:** when a child is encountered for the first time,
`new_g < ∞` is always true, so it is correctly enqueued without any special
"first visit" case.

**Lazy deletion:** instead of updating the priority queue in-place (expensive),
when a better path is found the child is simply pushed again with its new, lower
`g`. The old entry remains in the queue but will be skipped when popped because
`visited == true` by then.

**Guarantee:** finds the **minimum-cost path**. If all weights were 1, UCS
degenerates to BFS.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes |
| Optimal     | **Yes** — always finds cheapest path |
| Time        | O((V + E) log V) |
| Space       | O(V) |

---

### 3.2 Greedy — Best-First Search

**File:** `src/algorithms/Greedy.cpp`

**Core idea:** always expand the node that *looks* closest to the goal according
to the heuristic `h`, completely ignoring the cost already paid to get there.
Fast in practice but **not optimal**.

**Priority queue:** min-heap on `h`. Comparator:
```cpp
struct CompareH {
    bool operator()(const W_Node* a, const W_Node* b) const {
        return a->h > b->h;
    }
};
```

**Step-by-step:**

```
1.  All g = ∞. start.g = 0. start.h = heuristic(start, goal). Push start.
2.  While pq not empty:
    a. Pop node with lowest h as `current`.
    b. If current.visited → skip.
    c. Mark current.visited = true.
    d. If current == goal → return showPath(current).
    e. For each passable neighbour `child`:
         new_g = current.g + getEdgeCost(current, child)
         if new_g < child.g:              ← still track g to avoid worse paths
             child.g      = new_g
             child.h      = heuristic(child, goal)
             child.parent = current
             push child onto pq
3.  pq empty → return {}.
```

**Why `g` is still tracked:** even though `g` is not the sorting key, the
condition `new_g < child.g` prevents adding a child if we already know a cheaper
way to reach it. This matches the Python source's `v2 < visitado[t].v2` guard and
avoids redundant work.

**Greedy vs UCS:** UCS guarantees the optimal cost; Greedy sacrifices that
guarantee for speed. On open grids Greedy often finds a near-optimal path very
quickly because the heuristic guides it straight to the goal.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes (with visited tracking) |
| Optimal     | **No** |
| Time        | O(b^m) worst case; much better in practice with good heuristic |
| Space       | O(b^m) |

---

### 3.3 A*

**File:** `src/algorithms/AStar.cpp`

**Core idea:** combines UCS and Greedy. The priority key is `f = g + h`:
- `g` ensures the cost paid so far is tracked (like UCS).
- `h` guides expansion toward the goal (like Greedy).
The result is optimal *and* more efficient than UCS when the heuristic is
admissible (never overestimates).

**Priority queue:** min-heap on `f = g + h`. Comparator:
```cpp
struct CompareF {
    bool operator()(const W_Node* a, const W_Node* b) const {
        return a->getF() > b->getF();
    }
};
```

**Step-by-step:**

```
1.  All g = ∞. start.g = 0. start.h = heuristic(start, goal). Push start.
2.  While pq not empty:
    a. Pop node with lowest f = g+h as `current`.
    b. If current.visited → skip.
    c. Mark current.visited = true.
    d. If current == goal → return showPath(current).
    e. For each passable neighbour `child`:
         new_g = current.g + getEdgeCost(current, child)
         if new_g < child.g:
             child.g      = new_g
             child.h      = heuristic(child, goal)
             child.parent = current
             push child onto pq
3.  pq empty → return {}.
```

The code is structurally identical to UCS and Greedy — **only the comparator
changes.** This is intentional: the three algorithms form a family where swapping
the priority key changes the search behaviour.

**The heuristic's role in A*:**
- `h = 0` everywhere → A* degenerates to UCS (pure cost expansion).
- `h` dominates (much larger than g) → A* degenerates to Greedy.
- Balanced, admissible `h` → A* expands fewer nodes than UCS while still
  guaranteeing the optimal path.

**Wormhole handling:** `heuristic()` already accounts for the wormhole pair when
computing `h`. This means A* will naturally prefer paths that use the wormhole if
the teleport genuinely shortens the route.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes |
| Optimal     | **Yes** (with admissible heuristic) |
| Time        | O(b^d) worst case; sub-exponential with good heuristic |
| Space       | O(b^d) — all nodes in open/closed sets |

---

### 3.4 AIA* — Incremental A*

**File:** `src/algorithms/AIAStar.cpp`

**Core idea:** A* bounded by a cost threshold `lim`. Nodes with `f > lim` are
pruned (not expanded). If the goal is not reached, the threshold is updated to
the **truncated average** of all pruned `f` values, and the search restarts from
scratch. This is a memory-efficient alternative to full A* — each iteration is
bounded, so the open set never grows as large.

**Relationship to IDA*:** classic IDA* updates the threshold to `min(pruned f)`.
AIA* updates to `trunc(avg(pruned f))`. The average makes the threshold jump
faster toward the optimal `f`, reducing the number of restarts on dense graphs.

**Step-by-step:**

```
lim = heuristic(start, goal)   ← initial bound: best-case f with g=0

while true:
    graph.reset();  all g = ∞
    start.g = 0;  start.h = heuristic(start, goal)
    push start onto pq
    novo_lim = []                ← f-values of pruned nodes this iteration

    while pq not empty:
        pop `current` (lowest f)
        if current.visited → skip
        mark current.visited = true
        if current == goal → return showPath(current)

        for each child of current:
            new_g = current.g + edgeCost
            new_h = heuristic(child, goal)
            new_f = new_g + new_h
            if new_f <= lim:
                if new_g < child.g:
                    update child and push
            else:
                novo_lim.append(new_f)   ← collect for next threshold

    if novo_lim is empty → return {}     ← search space exhausted, no path

    lim = trunc( sum(novo_lim) / count(novo_lim) )
    ← new threshold is the truncated average of all pruned f-values
```

**Why reset every iteration:**
AIA* restarts from scratch each time because the `visited` flags from the
previous bounded run reflect decisions made under the old (lower) threshold.
Old expansions may have taken suboptimal paths; with a higher `lim`, some
previously-pruned branches might yield better routes.

**The `trunc()` cast:**
The Python source explicitly casts to `int`: `lim = int(sum / len)`. This
truncates (rounds toward zero) rather than rounding up. In C++ this is:
```cpp
lim = static_cast<float>(static_cast<int>(sum / count));
```
This is a deliberate threshold-growth strategy: the truncation makes the limit
grow conservatively, ensuring the search does not overshoot into a very expensive
region in a single jump.

**Termination — empty `novo_lim`:**
If no node was pruned during an iteration (all reachable nodes were expanded
without finding the goal), the graph is fully exhausted — there is no path.
This is the only clean termination condition other than finding the goal.

**Memory advantage:**
Each iteration uses O(b·d_lim) space (bounded DFS-like stack). Full A* uses
O(b^d) space storing all nodes ever discovered. On large grids this difference
is significant.

**Properties:**

| Property    | Value |
|-------------|-------|
| Complete    | Yes |
| Optimal     | **Yes** (admissible heuristic + threshold eventually reaches optimal f) |
| Time        | Higher constant than A* due to restarts; fewer per-iteration nodes |
| Space       | O(b·d) per iteration — **much better than A*** |

---

## 4. Algorithm Comparison

### Unweighted algorithms at a glance

| Algorithm | Data Structure | Optimal | Complete | Space    | Key use case |
|-----------|---------------|---------|----------|----------|--------------|
| BFS       | Queue (FIFO)   | Yes     | Yes      | O(b^d)   | Shortest hop-count path |
| DFS       | Stack (LIFO)   | No      | Yes      | O(b·m)   | Any path, minimum memory |
| DLS       | Stack          | No      | Partial  | O(b·l)   | Bounded exploration |
| IDS       | Stack (reset)  | Yes     | Yes      | O(b·d)   | BFS quality + DFS memory |
| BDS       | Two queues     | Yes     | Yes      | O(b^d/2) | Large graphs, both ends known |

### Weighted algorithms at a glance

| Algorithm | Priority key | Optimal | Uses heuristic | Key characteristic |
|-----------|-------------|---------|----------------|-------------------|
| UCS       | `g`         | Yes     | No             | Guarantees minimum cost |
| Greedy    | `h`         | No      | Yes            | Fast, goal-directed |
| A*        | `g + h`     | Yes     | Yes            | Best balance of speed and optimality |
| AIA*      | `g + h` + threshold | Yes | Yes       | A* quality with bounded memory per iteration |

### Choosing an algorithm

```
Need the shortest path (unweighted)?
    ├─ Memory constrained?            → IDS
    ├─ Both endpoints known?          → BDS
    └─ Otherwise                      → BFS

Need the cheapest path (weighted)?
    ├─ No heuristic available?        → UCS
    ├─ Speed > optimality?            → Greedy
    ├─ Optimal + fast (normal graph)? → A*
    └─ Optimal + large graph?         → AIA*
```

---

*STARLOG — maintained alongside the Netuno project source.*
