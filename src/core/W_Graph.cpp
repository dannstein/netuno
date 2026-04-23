#include "core/W_Graph.hpp"
#include <iostream>
#include <ctime>
#include <cmath>

static constexpr float INF_WEIGHT = std::numeric_limits<float>::infinity();

W_Graph::W_Graph(int x, int y) : nx(x), ny(y), wormhole_a(nullptr) {
    grid.resize(nx, std::vector<W_Node>(ny));
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            grid[i][j] = W_Node(i, j);
}

void W_Graph::reset() {
    for (auto& row : grid)
        for (auto& node : row) {
            node.parent  = nullptr;
            node.visited = false;
            node.g       = 0.0f;
            node.h       = 0.0f;
        }
}

std::vector<W_Node*> W_Graph::sucessors_grid(W_Node* current) {
    std::vector<W_Node*> children;

    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {1, -1, 0, 0};

    for (int i = 0; i < 4; i++) {
        int xi = current->x + dx[i];
        int yi = current->y + dy[i];
        if (xi >= 0 && xi < nx && yi >= 0 && yi < ny) {
            W_Node* nb = &grid[xi][yi];
            if (nb->isPassable())
                children.push_back(nb);
        }
    }

    if (current->state == CellType::WORMHOLE && current->warp_target != nullptr)
        children.push_back(current->warp_target);

    return children;
}

float W_Graph::getEdgeCost(W_Node* from, W_Node* to) const {
    if (!to->isPassable())
        return INF_WEIGHT;

    (void)from;
    return to->weight;
}

void W_Graph::placeBlackHole(int cx, int cy) {
    for (int i = cx - 2; i <= cx + 2; i++) {
        for (int j = cy - 2; j <= cy + 2; j++) {
            if (i < 0 || i >= nx || j < 0 || j >= ny) continue;
            int dist = std::max(std::abs(i - cx), std::abs(j - cy));
            if (dist == 0) {
                grid[i][j].state  = CellType::BLACK_HOLE;
                grid[i][j].weight = 100.0f;
            } else if (dist == 1) {
                grid[i][j].state  = CellType::BLACK_HOLE_R1;
                grid[i][j].weight = 80.0f;
            } else {
                grid[i][j].state  = CellType::BLACK_HOLE_R2;
                grid[i][j].weight = 50.0f;
            }
        }
    }
}

void W_Graph::placePulsar(int cx, int cy, std::mt19937& /*rng*/) {
    if (cx < 0 || cx >= nx || cy < 0 || cy >= ny) return;
    grid[cx][cy].state  = CellType::PULSAR_CENTER;
    grid[cx][cy].weight = 75.0f;

    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {1, -1, 0, 0};
    for (int d = 0; d < 4; d++) {
        for (int len = 1; len <= 2; len++) {
            int xi = cx + dx[d] * len;
            int yi = cy + dy[d] * len;
            if (xi < 0 || xi >= nx || yi < 0 || yi >= ny) continue;
            if (grid[xi][yi].state == CellType::OPEN) {
                grid[xi][yi].state  = CellType::PULSAR_JET;
                grid[xi][yi].weight = 55.0f;
            }
        }
    }
}

void W_Graph::placeNebula(int cx, int cy, int size, std::mt19937& rng) {
    std::uniform_int_distribution<int> dirDist(0, 3);
    std::uniform_real_distribution<float> wDist(15.0f, 25.0f);

    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {1, -1, 0, 0};

    int x = cx, y = cy;
    for (int s = 0; s < size; s++) {
        if (x >= 0 && x < nx && y >= 0 && y < ny &&
            grid[x][y].state == CellType::OPEN) {
            grid[x][y].state  = CellType::NEBULA;
            grid[x][y].weight = wDist(rng);
        }
        int d = dirDist(rng);
        x += dx[d];
        y += dy[d];
    }
}

void W_Graph::placeWormhole(int ax, int ay, int bx, int by) {
    if (ax < 0 || ax >= nx || ay < 0 || ay >= ny) return;
    if (bx < 0 || bx >= nx || by < 0 || by >= ny) return;
    if (ax == bx && ay == by) return;

    grid[ax][ay].state       = CellType::WORMHOLE;
    grid[ax][ay].weight      = 1.0f;
    grid[ax][ay].warp_target = &grid[bx][by];

    grid[bx][by].state       = CellType::WORMHOLE;
    grid[bx][by].weight      = 1.0f;
    grid[bx][by].warp_target = &grid[ax][ay];

    wormhole_a = &grid[ax][ay];
}

float W_Graph::heuristic(W_Node* from, W_Node* goal) const {
    auto wdist = [](const W_Node* a, const W_Node* b) -> float {
        float dx = static_cast<float>(b->x - a->x);
        float dy = static_cast<float>(b->y - a->y);
        float c1 = (dx < 0.0f) ? 3.0f : 2.0f;
        float c2 = (dy < 0.0f) ? 7.0f : 5.0f;
        return std::sqrt(c1 * dx * dx + c2 * dy * dy);
    };

    float h = wdist(from, goal);

    if (wormhole_a != nullptr && wormhole_a->warp_target != nullptr) {
        W_Node* wb = wormhole_a->warp_target;
        float via_a = wdist(from, wormhole_a) + wdist(wb,         goal);
        float via_b = wdist(from, wb)         + wdist(wormhole_a, goal);
        h = std::min(h, std::min(via_a, via_b));
    }

    return h;
}

void W_Graph::generateRandomMap(int wallProbability) {
    wormhole_a = nullptr;
    std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_int_distribution<int>  prob(0, 99);
    std::uniform_real_distribution<float> openW(1.0f, 49.0f);
    std::uniform_int_distribution<int>  rx(0, nx - 1);
    std::uniform_int_distribution<int>  ry(0, ny - 1);

    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            if (prob(rng) < wallProbability) {
                grid[i][j].state  = CellType::ASTEROID;
                grid[i][j].weight = INF_WEIGHT;
            } else {
                grid[i][j].state  = CellType::OPEN;
                grid[i][j].weight = openW(rng);
            }
        }
    }

    if (prob(rng) < 20)
        placeBlackHole(rx(rng), ry(rng));

    if (prob(rng) < 20)
        placeWormhole(rx(rng), ry(rng), rx(rng), ry(rng));

    if (prob(rng) < 30) {
        std::uniform_int_distribution<int> sizeDist(5, 10);
        placeNebula(rx(rng), ry(rng), sizeDist(rng), rng);
    }

    if (prob(rng) < 20)
        placePulsar(rx(rng), ry(rng), rng);
}

void W_Graph::drawConsole() {
    std::cout << "\n--- SPACE MAP (" << nx << "x" << ny << ") ---\n";
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            switch (grid[i][j].state) {
                case CellType::ASTEROID:      std::cout << " # "; break;
                case CellType::BLACK_HOLE:    std::cout << "[O]"; break;
                case CellType::BLACK_HOLE_R1: std::cout << "(o)"; break;
                case CellType::BLACK_HOLE_R2: std::cout << " o "; break;
                case CellType::WORMHOLE:      std::cout << "{W}"; break;
                case CellType::NEBULA:        std::cout << ":::"; break;
                case CellType::PULSAR_CENTER: std::cout << "[*]"; break;
                case CellType::PULSAR_JET:    std::cout << " * "; break;
                default:                      std::cout << " . "; break;
            }
        }
        std::cout << "\n";
    }
    std::cout << "------------------------------------\n";
    std::cout << " #  Asteroid    [O] Black hole core\n";
    std::cout << "(o) BH ring 1    o  BH ring 2\n";
    std::cout << "{W} Wormhole    ::: Nebula\n";
    std::cout << "[*] Pulsar       *  Pulsar jet\n";
    std::cout << " .  Open space\n";
}

std::vector<W_Node*> W_Graph::showPath(W_Node* lastNode) {
    std::vector<W_Node*> path;
    W_Node* current = lastNode;
    while (current != nullptr) {
        path.push_back(current);
        current = current->parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

void W_Graph::drawPath(const std::vector<W_Node*>& path) {
    std::vector<std::vector<bool>> onPath(nx, std::vector<bool>(ny, false));
    for (W_Node* n : path)
        onPath[n->x][n->y] = true;

    std::cout << "\n--- PATH FOUND ---\n";
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            if (onPath[i][j]) {
                std::cout << " X ";
                continue;
            }
            switch (grid[i][j].state) {
                case CellType::ASTEROID:      std::cout << " # "; break;
                case CellType::BLACK_HOLE:    std::cout << "[O]"; break;
                case CellType::BLACK_HOLE_R1: std::cout << "(o)"; break;
                case CellType::BLACK_HOLE_R2: std::cout << " o "; break;
                case CellType::WORMHOLE:      std::cout << "{W}"; break;
                case CellType::NEBULA:        std::cout << ":::"; break;
                case CellType::PULSAR_CENTER: std::cout << "[*]"; break;
                case CellType::PULSAR_JET:    std::cout << " * "; break;
                default:                      std::cout << " . "; break;
            }
        }
        std::cout << "\n";
    }
}

void W_Graph::drawGame(const std::vector<W_Node*>& path, int ox, int oy, int tx, int ty) {
    std::vector<std::vector<bool>> onPath(nx, std::vector<bool>(ny, false));
    for (W_Node* n : path)
        onPath[n->x][n->y] = true;

    std::cout << "\n";
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            // S and T take priority over everything else
            if      (i == ox && j == oy) { std::cout << " S "; continue; }
            else if (i == tx && j == ty) { std::cout << " T "; continue; }
            else if (onPath[i][j])       { std::cout << " X "; continue; }
            switch (grid[i][j].state) {
                case CellType::ASTEROID:      std::cout << " # "; break;
                case CellType::BLACK_HOLE:    std::cout << "[O]"; break;
                case CellType::BLACK_HOLE_R1: std::cout << "(o)"; break;
                case CellType::BLACK_HOLE_R2: std::cout << " o "; break;
                case CellType::WORMHOLE:      std::cout << "{W}"; break;
                case CellType::NEBULA:        std::cout << ":::"; break;
                case CellType::PULSAR_CENTER: std::cout << "[*]"; break;
                case CellType::PULSAR_JET:    std::cout << " * "; break;
                default:                      std::cout << " . "; break;
            }
        }
        std::cout << "\n";
    }
    std::cout << " S=Base  T=Treasure  X=Path\n";
    std::cout << " #=Asteroid  [O]=BlackHole  {W}=Wormhole  ::: =Nebula  [*]=Pulsar\n";
}
