 cmake_minimum_required(VERSION 3.10)
project(netuno_pathfinding)

# Definir o padrão do C++ (C++17 ou 20 é recomendado para algoritmos)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar a biblioteca gráfica (Exemplo: SFML)
# Você precisa ter a SFML instalada no sistema
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Incluir os diretórios de cabeçalho
include_directories(include)

# Listar todos os arquivos fonte
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/algorithms/*.cpp"
    "src/core/*.cpp"
    "src/gui/*.cpp"
)

# Criar o executável
add_executable(${PROJECT_NAME} ${SOURCES})

# Linkar as bibliotecas
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system)

