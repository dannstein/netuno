from BuscaNP import buscaNP
import F_auxiliares as fa
from os import system

while(True):
    system("cls")
    print("**** TIPO DE EXECUÇÃO ****\n")
    print("1. GRAFO")
    print("2. GRID")
    op = input("Sua opção:")
    
    if(op=='1'):
        #---------------- Executa Grafo -----------------------------
        arquivo = "Vale_do_Paraiba.txt"
        nos, grafo = fa.Gera_Problema_Grafo(arquivo)
        print("======== Lista de nós ========\n",nos)
        #print("\n======== Lista de Adjacência ========\n",grafo)
        origem  = input("\nOrigem......: ").upper()
        destino = input("Destino.....: ").upper()
        flag_origem  = origem in nos
        flag_destino = destino in nos
        flag = flag_origem and flag_destino
        flag_grafo = True
        #------------------------------------------------------------
    else:
        #---------------- Executa Grig ------------------------------
        arquivo = "mapa.txt"
        mapa,dx,dy = fa.Gera_Problema_Grid_Fixo(arquivo)
        # Entrada de dados para busca em grid
        origem  = tuple(map(int, input("Digite a origem (x y): ").split()))
        destino = tuple(map(int, input("Digite o destino (x y): ").split()))
        #destino_str = input("Digite as coordenadas de destino no formato x y, separadas por vírgula: ")
        #destino = [tuple(map(int, coord.split())) for coord in destino_str.split(",")]
        flag_origem  = (0<=origem[0]<dx)  and (0<=origem[1]<dy)  and (mapa[origem[0]][origem[1]]==0)
        flag_destino = (0<=destino[0]<dx) and (0<=destino[1]<dy) and (mapa[destino[0]][destino[1]]==0)
        #flag_destino = all(0<=x<dx and 0<=y<dy for x,y in destino)
        flag = flag_origem and flag_destino
        flag_grafo = False
        #------------------------------------------------------------
    if flag:
        sol = buscaNP()
        caminho = []
        # AMPLITUDE
        if flag_grafo:
            caminho = sol.amplitude_grafo(origem,destino,nos,grafo)
        else:
            caminho = sol.amplitude_grid(origem,destino,dx,dy,mapa)
        if caminho!=None:
            fa.imprimeCaminho("AMPLITUDE", caminho, len(caminho))
        else:
            print("AMPLITUDE/nCAMINHO NÃO ENCONTRADO")
        
        # PROFUNDIDADE
        caminho = []
        if flag_grafo:
            caminho = sol.profundidade_grafo(origem,destino,nos,grafo)
        else:
            caminho = sol.profundidade_grid(origem,destino,dx,dy,mapa)
        print("\n*****PROFUNDIDADE*****")
        if caminho!=None:
            fa.imprimeCaminho("PROFUNIDADE", caminho, len(caminho))
        else:
            print("PROFUNDIDADE/nCAMINHO NÃO ENCONTRADO")    
    else:
        print("Estados inválidos!")
    op2 = input("\nDeseja continuar (S/N)?").upper()
    if op2=='N':
        break
    system("cls") 
"""
    limite = 2
    caminho = sol.prof_limitada(origem,destino,nos,grafo,limite)
    print("\n*****PROFUNDIDADE LIMITADA*****")
    if caminho!=None:
        print("\n*****PROFUNDIDADE LIMITADA*****")
        print("Caminho: ",caminho)
        print("Custo..: ",len(caminho)-1)
    else:
        print("CAMINHO NÃO ENCONTRADO")
    
    limite = 3
    caminho = sol.prof_limitada(origem,destino,nos,grafo,limite)
    print("\n*****PROFUNDIDADE LIMITADA*****")
    if caminho!=None:
        print("Caminho: ",caminho)
        print("Custo..: ",len(caminho)-1)
    else:
        print("CAMINHO NÃO ENCONTRADO")
    
    limite = 4
    caminho = sol.prof_limitada(origem,destino,nos,grafo,limite)
    print("\n*****PROFUNDIDADE LIMITADA*****")
    if caminho!=None:
        print("Caminho: ",caminho)
        print("Custo..: ",len(caminho)-1)
    else:
        print("CAMINHO NÃO ENCONTRADO")

    l_max = len(nos)
    caminho = sol.aprof_iterativo(origem,destino,nos,grafo,l_max)
    if caminho!=None:
        print("\n*****APROFUNDAMENTO ITERATIVO*****")
        print("Caminho: ",caminho)
        print("Custo..: ",len(caminho)-1)
    else:
        print("CAMINHO NÃO ENCONTRADO")
       
caminho = sol.bidirecional(origem,destino,nos,grafo)
if caminho!=None:
    print("\n*****BIDIRECIONAL*****")
    print("Caminho: ",caminho)
    print("Custo..: ",len(caminho)-1)
else:
    print("CAMINHO NÃO ENCONTRADO")
""" 
