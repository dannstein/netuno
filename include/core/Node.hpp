#ifndef NODE_HPP
#define NODE_HPP

#include <vector>

// Em C++, structs e classes são quase iguais. 
// Usamos struct aqui para facilitar o acesso direto aos atributos (como no Python).
struct Node {
    // Atributos que você pediu (espelho do Python)
    Node* parent;        // self.parent
    int state;       // self.state (ex: 0 para livre, 1 para parede)
    float v1;         // self.v1 (pode ser o custo acumulado 'g')
    Node* previous;   // self.anterior (útil para listas encadeadas de busca)
    Node* next;    // self.proximo

    // Peculiaridades para o Grid
    int x, y;         // Posição na matriz
    float h;          // Heurística (distância até o destino)
    bool visited;    // Controle para não entrar em loop

    // Construtor (o seu __init__)
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
