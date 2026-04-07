#include <algorithms/IDS.hpp>

std::vector<Node*> IDS::findPathGrid(Graph& graph, int startX, int startY, int endX, int endY, int max_limit){
    for(int i = 1; i < max_limit; i++){
        graph.reset();
        
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

            if(current->v1 < i){
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
        }

    }

    return {};
}