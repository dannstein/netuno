#ifndef NODE_HPP
#define NODE_HPP

#include <vector>

struct Node {
    Node* parent;       
    int state;       
    float v1;         
    Node* previous;   
    Node* next;    

    int x, y;         // Posição na matriz
    float h;          // Heurística (distância até o destino)
    bool visited;    // Controle para não entrar em loop

    Node(int _x = 0, int _y = 0, int _state = 0, Node* _parent = nullptr) 
        : x(_x), y(_y), state(_state), parent(_parent) {
        
        // Inicializa o restante como nulo ou zero
        v1 = 0.0f;
        h = 0.0f;
        previous = nullptr;
        next = nullptr;
        visited = false;
    }

    // Função auxiliar (opcional) para calcular o custo total F = G + H
    float getF() const { return v1 + h; }
};

#endif
