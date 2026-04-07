#include "core/Graph.hpp"
#include "algorithms/BFS.hpp"
#include "algorithms/DFS.hpp"
#include "algorithms/DLS.hpp"
#include "algorithms/IDS.hpp"
#include "algorithms/BDS.hpp"
#include <iostream>

int main() {
    int size, prob, limit = 0, max_limit = 0;
    std::cout << "Digite o tamanho do grid: ";
    std::cin >> size;
    std::cout << "Digite a probabilidade de paredes: ";
    std::cin >> prob;

    Graph myMap(size, size);
    myMap.generateRandomMap(prob);
    myMap.drawConsole();

    int sx, sy, ex, ey;
    std::cout << "Digite o Inicio (x y): ";
    std::cin >> sx >> sy;
    std::cout << "Digite o Fim (x y): ";
    std::cin >> ex >> ey;

    // --- NOVA TRAVA DE SEGURANÇA ---
    if (sx < 0 || sx >= size || sy < 0 || sy >= size || 
        ex < 0 || ex >= size || ey < 0 || ey >= size) {
        std::cout << "ERRO: Coordenadas fora do mapa (0 a " << size-1 << ")!" << std::endl;
        return 1; // Fecha o programa com erro
    }
    // -------------------------------

    myMap.grid[sx][sy].state = 0;
    myMap.grid[ex][ey].state = 0;

    while(true){
        int opt = 0;

        std::cout << "Escolha um algoritmo: " << "\n\n" ;
        std::cout << "1. Amplitude" << "\n"; 
        std::cout << "2. Profundidade" << "\n";
        std::cout << "3. Profundidade Limitada" << "\n";
        std::cout << "4. Aprofundamento Iterativo" << "\n";
        std::cout << "5. Busca Bidirecional" << "\n";
        std::cout << "9. Sair" << "\n\n";

        std::cin >> opt;

        if(opt == 9) break;

        myMap.reset();

        std::cout << "Buscando caminho..." << std::endl;

        std::vector<Node*> result;

        switch(opt){
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
        }

        //std::cout << "Buscando caminho..." << std::endl;
        //std::vector<Node*> result = BFS::findPathGrid(myMap, sx, sy, ex, ey);

        if (!result.empty()) {
            std::cout << "Caminho encontrado com " << result.size() << " nos!";
            myMap.drawPath(result); 
        } else {
            std::cout << "Nao foi possivel encontrar um caminho." << std::endl;
        }
    }

    return 0;
}


