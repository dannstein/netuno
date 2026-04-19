#include <algorithms/AStar.hpp>
#include <limits>

struct CompareF {
    bool operator()(const W_Node* a, const W_Node* b) const {
        return a->getF() > b->getF();
    }
};

std::vector<W_Node*> AStar::findPathGrid(W_Graph& graph, int startX, int startY, int endX, int endY) {
    if (startX == endX && startY == endY) return {};

    graph.reset();

    for (auto& row : graph.grid)
        for (auto& node : row)
            node.g = std::numeric_limits<float>::infinity();

    W_Node* start = &graph.grid[startX][startY];
    W_Node* goal  = &graph.grid[endX][endY];

    start->g = 0.0f;
    start->h = graph.heuristic(start, goal);

    std::priority_queue<W_Node*, std::vector<W_Node*>, CompareF> pq;
    pq.push(start);

    while (!pq.empty()) {
        W_Node* current = pq.top();
        pq.pop();

        if (current->visited) continue;
        current->visited = true;

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
