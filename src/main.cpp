#include "core/Graph.hpp"
#include "core/W_Graph.hpp"
#include "algorithms/BFS.hpp"
#include "algorithms/DFS.hpp"
#include "algorithms/DLS.hpp"
#include "algorithms/IDS.hpp"
#include "algorithms/BDS.hpp"

#include "algorithms/UCS.hpp"
#include "algorithms/Greedy.hpp"
#include "algorithms/AStar.hpp"
#include "algorithms/AIAStar.hpp"

#include <iostream>

static void runUnweighted(int size, int prob) {
    Graph myMap(size, size);
    myMap.generateRandomMap(prob);
    myMap.drawConsole();

    int sx, sy, ex, ey;
    std::cout << "Digite o Inicio (x y): ";
    std::cin >> sx >> sy;
    std::cout << "Digite o Fim (x y): ";
    std::cin >> ex >> ey;

    if (sx < 0 || sx >= size || sy < 0 || sy >= size ||
        ex < 0 || ex >= size || ey < 0 || ey >= size) {
        std::cout << "ERRO: Coordenadas fora do mapa (0 a " << size - 1 << ")!\n";
        return;
    }

    myMap.grid[sx][sy].state = 0;
    myMap.grid[ex][ey].state = 0;

    int limit = 0, max_limit = 0;

    while (true) {
        int opt = 0;
        std::cout << "\nEscolha um algoritmo:\n\n";
        std::cout << "1. Amplitude (BFS)\n";
        std::cout << "2. Profundidade (DFS)\n";
        std::cout << "3. Profundidade Limitada (DLS)\n";
        std::cout << "4. Aprofundamento Iterativo (IDS)\n";
        std::cout << "5. Busca Bidirecional (BDS)\n";
        std::cout << "9. Voltar\n\n";
        std::cin >> opt;

        if (opt == 9) break;

        myMap.reset();
        std::cout << "Buscando caminho...\n";

        std::vector<Node*> result;

        switch (opt) {
            case 1:
                result = BFS::findPathGrid(myMap, sx, sy, ex, ey);
                break;
            case 2:
                result = DFS::findPathGrid(myMap, sx, sy, ex, ey);
                break;
            case 3:
                std::cout << "Insira o limite desejado: ";
                std::cin >> limit;
                result = DLS::findPathGrid(myMap, sx, sy, ex, ey, limit);
                break;
            case 4:
                std::cout << "Insira o limite maximo: ";
                std::cin >> max_limit;
                result = IDS::findPathGrid(myMap, sx, sy, ex, ey, max_limit);
                break;
            case 5:
                result = BDS::findPathGrid(myMap, sx, sy, ex, ey);
                break;
            default:
                std::cout << "Opcao invalida.\n";
                continue;
        }

        if (!result.empty()) {
            std::cout << "Caminho encontrado com " << result.size() << " nos!\n";
            myMap.drawPath(result);
        } else {
            std::cout << "Nao foi possivel encontrar um caminho.\n";
        }
    }
}

static void runWeighted(int size, int prob) {
    W_Graph myMap(size, size);
    myMap.generateRandomMap(prob);
    myMap.drawConsole();

    int sx, sy, ex, ey;
    std::cout << "Digite o Inicio (x y): ";
    std::cin >> sx >> sy;
    std::cout << "Digite o Fim (x y): ";
    std::cin >> ex >> ey;

    if (sx < 0 || sx >= size || sy < 0 || sy >= size ||
        ex < 0 || ex >= size || ey < 0 || ey >= size) {
        std::cout << "ERRO: Coordenadas fora do mapa (0 a " << size - 1 << ")!\n";
        return;
    }

    myMap.grid[sx][sy].state  = CellType::OPEN;
    myMap.grid[sx][sy].weight = 1.0f;
    myMap.grid[ex][ey].state  = CellType::OPEN;
    myMap.grid[ex][ey].weight = 1.0f;

    while (true) {
        int opt = 0;
        std::cout << "\nEscolha um algoritmo:\n\n";
        std::cout << "1. Custo Uniforme (UCS)\n";
        std::cout << "2. Guloso (Greedy)\n";
        std::cout << "3. A*\n";
        std::cout << "4. A* Incremental (AIA*)\n";
        std::cout << "9. Voltar\n\n";
        std::cin >> opt;

        if (opt == 9) break;

        std::cout << "Buscando caminho...\n";

        std::vector<W_Node*> result;

        switch (opt) {
            case 1:
                result = UCS::findPathGrid(myMap, sx, sy, ex, ey);
                break;
            case 2:
                result = Greedy::findPathGrid(myMap, sx, sy, ex, ey);
                break;
            case 3:
                result = AStar::findPathGrid(myMap, sx, sy, ex, ey);
                break;
            case 4:
                result = AIAStar::findPathGrid(myMap, sx, sy, ex, ey);
                break;
            default:
                std::cout << "Opcao invalida.\n";
                continue;
        }

        if (!result.empty()) {
            float cost = result.back()->g;
            std::cout << "Caminho encontrado com " << result.size()
                      << " nos! Custo total: " << cost << "\n";
            myMap.drawPath(result);
        } else {
            std::cout << "Nao foi possivel encontrar um caminho.\n";
        }
    }
}

int main() {
    int size, prob;
    std::cout << "Digite o tamanho do grid: ";
    std::cin >> size;
    std::cout << "Digite a probabilidade de paredes (0-100): ";
    std::cin >> prob;

    while (true) {
        int mode = 0;
        std::cout << "\n=== MENU PRINCIPAL ===\n\n";
        std::cout << "1. Busca nao-ponderada  (BFS / DFS / DLS / IDS / BDS)\n";
        std::cout << "2. Busca ponderada      (UCS / Greedy / A* / AIA*)\n";
        std::cout << "9. Sair\n\n";
        std::cin >> mode;

        if (mode == 9) break;

        switch (mode) {
            case 1: runUnweighted(size, prob); break;
            case 2: runWeighted(size, prob);   break;
            default: std::cout << "Opcao invalida.\n"; break;
        }
    }

    return 0;
}
