#include <algorithms/DFS.hpp>

std::vector<Node*> DFS::findPathGrid(Graph& graph, int startX, int startY, int endX, int endY){
    if(startX == endX && startY == endY) return{};

    std::stack<Node*> sc;

    Node* root = &graph.grid[startX][startY];
    root->visited = true;
    sc.push(root);

    while(!sc.empty()){
        Node* current = sc.top();
        sc.pop();

        if(current->x == endX && current->y == endY){
            return graph.showPath(current);
        }

        std::vector<Node*> children = graph.sucessors_grid(current);

        for(Node* child : children){
            if(!child->visited){
                child->parent = current;
                child->v1 = current->v1 + 1;
                child->visited = true;

                sc.push(child);
            }
        }
    }

    return {};
}