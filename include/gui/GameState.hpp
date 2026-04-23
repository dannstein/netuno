#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include <vector>
#include <functional>
#include <utility>
#include <random>
#include <memory>
#include <string>
#include "core/Graph.hpp"
#include "core/W_Graph.hpp"

enum class AlgoID    { BFS, DFS, DLS, IDS, BDS, UCS, GREEDY, ASTAR, AIASTAR };
enum class GraphMode { UNWEIGHTED, WEIGHTED };
enum class GamePhase { EXPLORING, TREASURE_FOUND, DONE, NO_PATH };

struct StepResult {
    std::vector<std::pair<int,int>> path;
    GamePhase phase        = GamePhase::EXPLORING;
    int       stepNum      = 0;
    float     stepCost     = 0.0f;
    float     totalCost    = 0.0f;
    bool      treasureOnRoute = false;
    bool      noPathToTarget  = false;
    bool      isReturnPath    = false;
};

class GameState {
public:
    GameState(int size, int wallProb, GraphMode mode, AlgoID algo,
              int dlsLimit = 0, int idsMaxLimit = 0, int fuelSteps = 0);
    ~GameState();

    // Advance one step of the game loop. Returns the result of that step.
    // When phase becomes DONE or NO_PATH, subsequent calls return immediately.
    StepResult advance();

    GamePhase  phase()      const { return phase_; }
    int        gridSize()   const { return size_; }
    bool       isWeighted() const { return mode_ == GraphMode::WEIGHTED; }

    // Unweighted grid query: 0=open, 1=wall
    int   cellState (int x, int y) const;

    // Weighted grid queries
    int   cellType  (int x, int y) const;
    float cellWeight(int x, int y) const;

    // Returns warp target coords, or {-1,-1} if none
    std::pair<int,int> warpTarget(int x, int y) const;

    std::pair<int,int> origin()    const { return {ox_, oy_}; }
    std::pair<int,int> treasure()  const { return {tx_, ty_}; }
    std::pair<int,int> shipPos()   const { return {cx_, cy_}; }
    std::pair<int,int> bhCenter()  const { return {bh_cx_, bh_cy_}; }
    std::pair<int,int> wormholeA() const { return worm_a_; }
    std::pair<int,int> wormholeB() const { return worm_b_; }

    AlgoID    algoID()    const { return algo_; }
    GraphMode graphMode() const { return mode_; }

    int fuelRemaining() const { return fuelLimit_ > 0 ? std::max(0, fuelLimit_ - step_) : -1; }
    int fuelTotal()     const { return fuelLimit_; }

private:
    int       size_, wallProb_;
    GraphMode mode_;
    AlgoID    algo_;
    int       dlsLimit_, idsMaxLimit_, fuelLimit_;

    std::mt19937 rng_;
    GamePhase    phase_ = GamePhase::EXPLORING;
    bool         treasureFoundFlag_ = false;

    std::unique_ptr<Graph>   ugraph_;
    std::unique_ptr<W_Graph> wgraph_;

    std::function<std::vector<Node*>  (int,int,int,int)> uAlgo_;
    std::function<std::vector<W_Node*>(int,int,int,int)> wAlgo_;

    std::vector<std::pair<int,int>> pool_;
    int   ox_, oy_;
    int   tx_, ty_;
    int   cx_, cy_;
    int   step_       = 0;
    float totalCost_  = 0.0f;

    int bh_cx_ = -1, bh_cy_ = -1;
    std::pair<int,int> worm_a_ = {-1,-1};
    std::pair<int,int> worm_b_ = {-1,-1};

    void buildUnweighted();
    void buildWeighted();
    void cacheSpecialCells();
};

#endif
