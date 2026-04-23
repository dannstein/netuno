#include "core/Graph.hpp"
#include <random>
#include <iostream>
#include <ctime>

Graph::Graph(int x, int y) : nx(x), ny(y) {
    // Inicializa a matriz com Nodes vazios
    grid.resize(nx, std::vector<Node>(ny));
    for(int i = 0; i < nx; i++) {
        for(int j = 0; j < ny; j++) {
            grid[i][j] = Node(i, j); // Configura x e y de cada nó
        }
    }
}

std::vector<Node*> Graph::sucessors_grid(Node* current) {
    std::vector<Node*> children;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    for (int i = 0; i < 4; i++) {
        int nextX = current->x + dx[i];
        int nextY = current->y + dy[i];

        // Usa o nx e ny da própria classe Graph
        if (nextX >= 0 && nextX < nx && nextY >= 0 && nextY < ny) {
            
            // Verifica o 'state' dentro do Node na grid
            if (grid[nextX][nextY].state == 0) { 
                children.push_back(&grid[nextX][nextY]);
            }
        }
    }
    return children;
}

void Graph::reset() {
    for (auto& line : grid) {
        for (auto& node : line) {
            node.parent = nullptr;
            node.visited = false;
            node.v1 = 0; // custo g
            // ... resetar outros custos se houver
        }
    }
}

void Graph::generateRandomMap(int wallProbability) {
    // Gerador de números aleatórios moderno
    std::mt19937 generator(time(0)); 
    std::uniform_int_distribution<int> dist(0, 100);

    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            int chance = dist(generator);
            
            if (chance < wallProbability) {
                grid[i][j].state = 1; // Parede/Bloqueado
            } else {
                grid[i][j].state = 0; // Caminho livre
            }
            
            // Futuro: você pode adicionar mais condições aqui
            // else if (chance < 40) grid[i][j].state = 2; // Ex: Lama (custo alto)
        }
    }
}

void Graph::drawConsole() {
    std::cout << "\n--- MAPA ATUAL (" << nx << "x" << ny << ") ---\n";
    
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            int state = grid[i][j].state;
            
            if (state == 1) {
                std::cout << " # "; // Parede / Bloqueado
            } else if (state == 0) {
                std::cout << " . "; // Caminho Livre
            } else {
                std::cout << " " << state << " "; // Outros states futuros
            }
        }
        std::cout << "\n"; // Quebra de linha ao fim de cada linha do grid
    }
    std::cout << "---------------------------\n";
}

std::vector<Node*> Graph::showPath(Node* lastNode){
        std::vector<Node*> path;
        Node* current = lastNode;

        // Enquanto o nó não for nulo (chegou na raiz/início)
        while (current != nullptr) {
            path.push_back(current); // Adiciona o nó atual à lista
            current = current->parent;       // Sobe para o pai (igual ao Python)
        }

        // Inverte o vetor para ir do Início -> Fim
        std::reverse(path.begin(), path.end());
        
        return path;
}

void Graph::drawPath(const std::vector<Node*>& path) {
    // Cria uma cópia visual do grid para não estragar o 'state' original
    std::vector<std::vector<char>> display(nx, std::vector<char>(ny, '.'));

    // Preenche as paredes e o caminho
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            if (grid[i][j].state == 1) display[i][j] = '#';
        }
    }

    // Marca o caminho com 'X'
    for (Node* n : path) {
        display[n->x][n->y] = 'X';
    }

    std::cout << "\n--- CAMINHO ENCONTRADO ---\n";
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            std::cout << " " << display[i][j] << " ";
        }
        std::cout << "\n";
    }
}

void Graph::drawGame(const std::vector<Node*>& path, int ox, int oy, int tx, int ty) {
    std::vector<std::vector<std::string>> display(nx, std::vector<std::string>(ny));
    for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
            display[i][j] = (grid[i][j].state == 1) ? " # " : " . ";

    for (Node* n : path)
        display[n->x][n->y] = " X ";

    // S and T take priority over path markers
    display[ox][oy] = " S ";
    display[tx][ty] = " T ";

    std::cout << "\n";
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++)
            std::cout << display[i][j];
        std::cout << "\n";
    }
    std::cout << " S=Base  T=Treasure  X=Path  #=Wall\n";
}