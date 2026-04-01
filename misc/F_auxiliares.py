import numpy as np  
import random as rd
#-----------------------------------------------------------------------------
# IMPORTA O GRAFO DE ARQUIVO TEXTO
#-----------------------------------------------------------------------------
def Gera_Problema_Grafo(arquivo):
    f = open(arquivo,"r",encoding="utf-8")
    
    nos = []
    grafo = []
    for str1 in f:
        str1 = str1.strip("\n")
        str1 = str1.split(",")
        nos.append(str1[0])
        grafo.append(str1[1:])
    
    return nos, grafo
#-----------------------------------------------------------------------------
# IMPORTA O GRAFO DE ARQUIVO TEXTO
#-----------------------------------------------------------------------------
def Gera_Problema_Grafo_Ale(n,m):
    
    grafo = []
    nos = []
    for i in range(n):
        nos.append(i)
        tam = rd.randint(5,m)
        linha = []
        for j in range(tam):
            x = rd.randrange(n)
            if x not in linha:
                linha.append(x)
        grafo.append(linha)
    return nos, grafo

#-----------------------------------------------------------------------------
# GERA GRID ALEATÓRIO
#-----------------------------------------------------------------------------
def Gera_Problema_Grid_Ale(nx,ny,qtd):
    mapa = np.zeros((nx,ny),int)
    
    k = 0
    while k<qtd:
        i = rd.randrange(0,nx)
        j = rd.randrange(0,ny)
        if mapa[i][j]==0:
            mapa[i][j] = 9
            k+=1
    return mapa,nx,ny
#-----------------------------------------------------------------------------
# GERA O GRID DE ARQUIVO TEXTO
#-----------------------------------------------------------------------------
def Gera_Problema_Grid_Fixo(arquivo):
    file = open(arquivo)
    mapa = []
    for line in file:
        aux_str = line.strip("\n")
        aux_str = aux_str.split(",")
        aux_int = [int(x) for x in aux_str]
        mapa.append(aux_int)
    nx = len(mapa)
    ny = len(mapa[0])
    return mapa,nx,ny

def imprimeCaminho(texto,caminho,custo):
    print("\n",texto)
    print("Caminho: ",caminho)
    print("Custo..: ",len(caminho)-1)
