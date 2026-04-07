#ifndef DLS_HPP
#define DLS_HPP

#include <vector>
#include <stack>
#include <map>
#include "../core/Node.hpp"
#include "../core/Graph.hpp"

class DLS {
public:
    // Recebe o objeto graph (que contém o mapa/grid) e as coordenadas de início e fim
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY, int limit);
};

#endif
