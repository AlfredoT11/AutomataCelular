import re
import numpy as np

def list_to_string(s):  
    
    str1 = ""  
    for ele in s:  
        str1 += ele
    return str1 

def guardar_configuracion_lif(grid, ruta, nombre):

    with open("ejemplo.lif", "w") as file:
        file.write("#Life 1.05\n")
        file.write("#D Comentario\n")
        file.write("#N\n")

        #print("Filas: ", grid.shape[0], " Columnas: ", grid.shape[1])

        posibles_columnas = int(np.floor(grid.shape[1] / 80))
        columnas_restantes = grid.shape[1] % 80

        for i in range(posibles_columnas):
            #print("Columna: ", i)
            j = 0
            while j < grid.shape[0]:
                if np.sum(grid[j, i*80: i*80 + 80]) > 0:
                    #print("Bloque inicio: {} , {}".format(j, i*80))
                    file.write("#P {} {}\n".format(i*80, j))
                    fila_inicio_bloque = j

                    temp_fila = fila_inicio_bloque + 1
                    contador_lineas_vacias = 0

                    is_fin_bloque_encontrado = False
                    while not is_fin_bloque_encontrado and temp_fila < grid.shape[0]:

                        if np.sum(grid[temp_fila, i*80: i*80 + 80]) > 0:
                            temp_fila += 1
                            contador_lineas_vacias = 0
                        else:
                            contador_lineas_vacias += 1
                            temp_fila += 1
                            if contador_lineas_vacias == 2:
                                is_fin_bloque_encontrado = True
                                temp_fila -= 2

                    fila_final_bloque = temp_fila
                    #print("Bloque final: {} , {}".format(fila_final_bloque, 80*i))

                    for fila in range(fila_inicio_bloque, fila_final_bloque+1):
                        #print("Fila: ", fila, " ", grid[fila:fila+1, i*80: i*80 + 80], "suma: ", np.sum(grid[fila:fila+1, i*80: i*80 + 80]))
                        if np.sum(grid[fila, i*80: i*80 + 80]) > 0:
                            linea_datos = []
                            ultimo_elemento_uno = np.where(grid[fila, i*80: i*80 + 80] != 0)[0][-1]
                            #print("Ultimo 1: ", np.where(grid[fila, i*80: i*80 + 80] != 0)[0])
                            for valor in grid[fila, i*80: i*80 + ultimo_elemento_uno+1]:
                                if valor == 1:
                                    linea_datos.append('*')
                                else:
                                    linea_datos.append('.')
                            linea_datos.append("\n")
                            file.write(list_to_string(linea_datos))
                        else:
                            file.write(".\n")
                    j = fila_final_bloque + 1
                else:
                    j += 1

        i = posibles_columnas
        j = 0
        while j < grid.shape[0]:
            if np.sum(grid[j, i*80: i*80 + columnas_restantes]) > 0:
                #print("Bloque inicio: {} , {}".format(j, i*80))
                file.write("#P {} {}\n".format(j, i*80))
                fila_inicio_bloque = j

                temp_fila = fila_inicio_bloque + 1
                contador_lineas_vacias = 0

                is_fin_bloque_encontrado = False
                while not is_fin_bloque_encontrado and temp_fila < grid.shape[0]:

                    if np.sum(grid[temp_fila, i*80: i*80 + columnas_restantes]) > 0:
                        temp_fila += 1
                        contador_lineas_vacias = 0
                    else:
                        contador_lineas_vacias += 1
                        temp_fila += 1
                        if contador_lineas_vacias == 2:
                            is_fin_bloque_encontrado = True
                            temp_fila -= 2

                fila_final_bloque = temp_fila
                #print("Bloque final: {} , {}".format(fila_final_bloque, 80*i))

                for fila in range(fila_inicio_bloque, fila_final_bloque+1):
                    #print("Fila: ", fila, " ", grid[fila:fila+1, i*80: i*80 + columnas_restantes], "suma: ", np.sum(grid[fila:fila+1, i*80: i*80 + columnas_restantes]))
                    if np.sum(grid[fila, i*80: i*80 + 80]) > 0:
                        linea_datos = []
                        ultimo_elemento_uno = np.where(grid[fila, i*80: i*80 + columnas_restantes] != 0)[0][-1]
                        #print("Ultimo 1: ", np.where(grid[fila, i*80: i*80 + columnas_restantes] != 0)[0])
                        for valor in grid[fila, i*80: i*80 + ultimo_elemento_uno+1]:
                            if valor == 1:
                                linea_datos.append('*')
                            else:
                                linea_datos.append('.')
                        linea_datos.append("\n")
                        file.write(list_to_string(linea_datos))
                    else:
                        file.write(".\n")
                j = fila_final_bloque + 1
            else:
                j += 1


def leer_archivo_lif(ruta_archivo_lif):

    patron_inicio_bloque = re.compile("^#P -*[0-9]+ -*[0-9]+")

    f = open(ruta_archivo_lif, 'r')

    lineas = f.readlines()

    linea = 0
    tamanio_linea_max = 0

    x_inicial = 0
    y_inicial = 0

    x_iniciales = []
    x_finales = []
    y_iniciales = []
    y_finales = []

    inicio_datos = -1
    
    num_linea = 0
    
    for i, line in enumerate(lineas):
        if patron_inicio_bloque.match(line) is not None:
            if(inicio_datos == -1):
                inicio_datos = i
            else:
                x_finales.append(x_inicial+tamanio_linea_max-1)
                y_finales.append(y_inicial+num_linea-1)
            linea_partida = line.split()
            x_inicial, y_inicial = int(linea_partida[1]), int(linea_partida[2])
            x_iniciales.append(x_inicial)
            y_iniciales.append(y_inicial)
            num_linea = 0
            tamanio_linea_max = 0
            
        elif inicio_datos != -1:
            if(len(line)-1 > tamanio_linea_max):
                tamanio_linea_max = len(line)-1
            num_linea += 1

    x_finales.append(x_inicial+tamanio_linea_max-1)
    y_finales.append(y_inicial+num_linea-1)


    tamanio_x, tamanio_y = max(x_finales) - min(x_iniciales), max(y_finales) - min(y_iniciales)

    offset_x, offset_y = min(x_iniciales), min(y_iniciales)

    patron_cargado = np.zeros((tamanio_y+1, tamanio_x+1), np.int)

    bloque = 0

    for line in lineas[inicio_datos:]:
        if patron_inicio_bloque.match(line) is not None:
            pos_inicial_x, pos_inicial_y = x_iniciales[bloque] - offset_x, y_iniciales[bloque] - offset_y
            num_linea = 0
            bloque += 1
        else:
            for i, simbolo in enumerate(line[:-1]):
                if(simbolo == "*"):
                    patron_cargado[pos_inicial_y+num_linea][pos_inicial_x+i] = 1
            num_linea += 1
            

    f.close()

    return patron_cargado.transpose()