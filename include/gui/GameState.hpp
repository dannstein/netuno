#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include <vector>
#include <utility>
#include <random>
#include <memory>
#include "core/Graph.hpp"
#include "core/W_Graph.hpp"

enum class AlgoID    { BFS, DFS, DLS, IDS, BDS, UCS, GREEDY, ASTAR, AIASTAR };
enum class GraphMode { UNWEIGHTED, WEIGHTED };
enum class GamePhase { EXPLORING, DONE, NO_PATH };

struct StepResult {
    GamePhase phase     = GamePhase::EXPLORING;
    int       revealIdx = 0;
};

class GameState {
public:
    GameState(int size, int wallProb, GraphMode mode, AlgoID algo,
              int dlsLimit = 0, int idsMaxLimit = 0, int fuelSteps = 0);
    ~GameState();

    StepResult advance();

    GamePhase  phase()      const { return phase_; }
    int        gridSize()   const { return size_; }
    bool       isWeighted() const { return mode_ == GraphMode::WEIGHTED; }

    int   cellState (int x, int y) const;
    int   cellType  (int x, int y) const;
    float cellWeight(int x, int y) const;

    std::pair<int,int> warpTarget(int x, int y) const;

    std::pair<int,int> origin()    const { return {ox_, oy_}; }
    std::pair<int,int> treasure()  const { return {tx_, ty_}; }
    std::pair<int,int> shipPos()   const { return {ox_, oy_}; }
    std::pair<int,int> bhCenter()  const { return {bh_cx_, bh_cy_}; }
    std::pair<int,int> wormholeA() const { return worm_a_; }
    std::pair<int,int> wormholeB() const { return worm_b_; }

    AlgoID    algoID()    const { return algo_; }
    GraphMode graphMode() const { return mode_; }

    int   revealIdx()  const { return revealIdx_; }
    int   logSize()    const { return (int)expansionLog_.size(); }
    int   pathHops()   const { return pathHops_; }
    float pathCost()   const { return pathCost_; }

    int fuelRemaining() const { return fuelLimit_ > 0 ? std::max(0, fuelLimit_ - revealIdx_) : -1; }
    int fuelTotal()     const { return fuelLimit_; }

    const std::vector<std::pair<int,int>>& expansionLog() const { return expansionLog_; }
    const std::vector<std::pair<int,int>>& finalPath()    const { return finalPath_; }

private:
    int       size_, wallProb_;
    GraphMode mode_;
    AlgoID    algo_;
    int       dlsLimit_, idsMaxLimit_, fuelLimit_;

    std::mt19937 rng_;
    GamePhase    phase_ = GamePhase::EXPLORING;

    std::unique_ptr<Graph>   ugraph_;
    std::unique_ptr<W_Graph> wgraph_;

    int ox_, oy_;
    int tx_, ty_;

    int bh_cx_ = -1, bh_cy_ = -1;
    std::pair<int,int> worm_a_ = {-1,-1};
    std::pair<int,int> worm_b_ = {-1,-1};

    std::vector<std::pair<int,int>> expansionLog_;
    std::vector<std::pair<int,int>> finalPath_;
    int   revealIdx_ = 0;
    int   pathHops_  = 0;
    float pathCost_  = 0.f;

    void buildUnweighted();
    void buildWeighted();
    void cacheSpecialCells();
};

#endif
