#ifndef DFS_HPP
#define DFS_HPP

#include <vector>
#include <stack>
#include <map>
#include "../core/Node.hpp"
#include "../core/Graph.hpp"

class DFS {
public:
    // Recebe o objeto graph (que contém o mapa/grid) e as coordenadas de início e fim
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY);
};

#endif
