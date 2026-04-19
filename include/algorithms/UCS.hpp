#ifndef UCS_HPP
#define UCS_HPP

#include <vector>
#include <queue>
#include "../core/W_Node.hpp"
#include "../core/W_Graph.hpp"

class UCS {
public:
    static std::vector<W_Node*> findPathGrid(W_Graph& graph, int startX, int startY, int endX, int endY);
};

#endif