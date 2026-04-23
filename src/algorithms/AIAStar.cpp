#include <algorithms/AIAStar.hpp>
#include <limits>
#include <algorithm>

struct CompareF_AIA {
    bool operator()(const W_Node* a, const W_Node* b) const {
        return a->getF() > b->getF();
    }
};

std::vector<W_Node*> AIAStar::findPathGrid(W_Graph& graph, int startX, int startY, int endX, int endY) {
    if (startX == endX && startY == endY) return {};

    W_Node* goal = &graph.grid[endX][endY];

    float lim = graph.heuristic(&graph.grid[startX][startY], goal);

    while (true) {
        graph.reset();
        for (auto& row : graph.grid)
            for (auto& node : row)
                node.g = std::numeric_limits<float>::infinity();

        W_Node* start = &graph.grid[startX][startY];
        start->g = 0.0f;
        start->h = graph.heuristic(start, goal);

        std::priority_queue<W_Node*, std::vector<W_Node*>, CompareF_AIA> pq;
        pq.push(start);

        std::vector<float> novo_lim;

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
                float new_h = graph.heuristic(child, goal);
                float new_f = new_g + new_h;

                if (new_f <= lim) {
                    if (new_g < child->g) {
                        child->g      = new_g;
                        child->h      = new_h;
                        child->parent = current;
                        pq.push(child);
                    }
                } else {
                    novo_lim.push_back(new_f);
                }
            }
        }

        if (novo_lim.empty()) return {};

        lim = *std::min_element(novo_lim.begin(), novo_lim.end());
    }
}
