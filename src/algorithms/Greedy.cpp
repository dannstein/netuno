#include <algorithms/Greedy.hpp>
#include <limits>

struct CompareH {
    bool operator()(const W_Node* a, const W_Node* b) const {
        return a->h > b->h;
    }
};

std::vector<W_Node*> Greedy::findPathGrid(W_Graph& graph, int startX, int startY, int endX, int endY,
                                           std::vector<std::pair<int,int>>* log) {
    if (startX == endX && startY == endY) return {};

    graph.reset();

    for (auto& row : graph.grid)
        for (auto& node : row)
            node.g = std::numeric_limits<float>::infinity();

    W_Node* start = &graph.grid[startX][startY];
    W_Node* goal  = &graph.grid[endX][endY];

    start->g = 0.0f;
    start->h = graph.heuristic(start, goal);

    std::priority_queue<W_Node*, std::vector<W_Node*>, CompareH> pq;
    pq.push(start);

    while (!pq.empty()) {
        W_Node* current = pq.top();
        pq.pop();

        if (current->visited) continue;
        current->visited = true;
        if (log) log->push_back({current->x, current->y});

        if (current == goal)
            return graph.showPath(current);

        for (W_Node* child : graph.sucessors_grid(current)) {
            if (child->visited) continue;

            float new_g = current->g + graph.getEdgeCost(current, child);

            if (new_g < child->g) {
                child->g      = new_g;
                child->h      = graph.heuristic(child, goal);
                child->parent = current;
                pq.push(child);
            }
        }
    }

    return {};
}
