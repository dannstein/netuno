#ifndef BFS_HPP
#define BFS_HPP

#include <vector>
#include <deque>
#include <utility>
#include "../core/Node.hpp"
#include "../core/Graph.hpp"

class BFS {
public:
    static std::vector<Node*> findPathGrid(Graph& graph, int startX, int startY, int endX, int endY,
                                           std::vector<std::pair<int,int>>* log = nullptr);
};

#endif
