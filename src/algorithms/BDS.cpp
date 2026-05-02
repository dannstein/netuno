#include <algorithms/BDS.hpp>


struct pair_hash {
    inline std::size_t operator()(const std::pair<int, int> & v) const {
        return v.first * 31 + v.second;
    }
};

using NodeMap = std::unordered_map<std::pair<int, int>, Node*, pair_hash>;

std::vector<Node*> buildPath(NodeMap& v1, NodeMap& v2, Node* n1, Node* n2) {
    std::vector<Node*> path;
    
    Node* curr = n1;
    while (curr != nullptr) {
        path.push_back(curr);
        curr = v1[{curr->x, curr->y}];
    }
    std::reverse(path.begin(), path.end());

    curr = n2;
    while (curr != nullptr) {
        path.push_back(curr);
        curr = v2[{curr->x, curr->y}];
    }
    return path;
}

std::vector<Node*> BDS::findPathGrid(Graph& graph, int startX, int startY, int endX, int endY,
                                      std::vector<std::pair<int,int>>* log){
    if(startX == endX && startY == endY) return{};

    std::unordered_map<std::pair<int, int>, Node*, pair_hash> visited1;
    std::unordered_map<std::pair<int, int>, Node*, pair_hash> visited2;

    Node* startNode = &graph.grid[startX][startY];
    Node* endNode = &graph.grid[endX][endY];

    std::deque<Node*> dq1 = { startNode };
    std::deque<Node*> dq2 = { endNode };

    visited1[{startX, startY}] = nullptr;
    visited2[{endX, endY}] = nullptr;

    while(!dq1.empty() && !dq2.empty()){

        int size1 = dq1.size();
        while(size1--) {
            Node* current = dq1.front();
            dq1.pop_front();
            if(log) log->push_back({current->x, current->y});

            for (Node* neighbor : graph.sucessors_grid(current)) {
                std::pair<int, int> pos = {neighbor->x, neighbor->y};
                
                if (visited2.count(pos)) {
                    return buildPath(visited1, visited2, current, neighbor);
                }
                if (!visited1.count(pos)) {
                    visited1[pos] = current;
                    dq1.push_back(neighbor);
                }
            }
        }

        int size2 = dq2.size();
        while(size2--) {
            Node* current = dq2.front();
            dq2.pop_front();
            if(log) log->push_back({current->x, current->y});

            for (Node* neighbor : graph.sucessors_grid(current)) {
                std::pair<int, int> pos = {neighbor->x, neighbor->y};

                if (visited1.count(pos)) { 
                    return buildPath(visited1, visited2, neighbor, current);
                }
                if (!visited2.count(pos)) {
                    visited2[pos] = current;
                    dq2.push_back(neighbor);
                }
            }
        }
    }
    

    return {};
}