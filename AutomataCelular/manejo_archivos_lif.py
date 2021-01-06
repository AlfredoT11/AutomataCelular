import re
import numpy as np


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

    return patron_cargado
