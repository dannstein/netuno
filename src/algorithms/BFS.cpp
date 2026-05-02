#include <algorithms/BFS.hpp>

std::vector<Node*> BFS::findPathGrid(Graph& graph, int startX, int startY, int endX, int endY,
                                      std::vector<std::pair<int,int>>* log){
    if(startX == endX && startY == endY) return{};

    std::deque<Node*> dq;

    Node* root = &graph.grid[startX][startY];
    root->visited = true;
    dq.push_back(root);

    while(!dq.empty()){
        Node* current = dq.front();
        dq.pop_front();
        if(log) log->push_back({current->x, current->y});

        std::vector<Node*> children = graph.sucessors_grid(current);

        for(Node* child : children){
            if(!child->visited){
                child->parent = current;
                child->v1 = current->v1 + 1;
                child->visited = true;

                dq.push_back(child);

                if(child->x == endX && child->y == endY){
                    return graph.showPath(child);
                }
            }
        }
    }

    return {};
}