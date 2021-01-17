#Autor: Díaz Tonatiuh
#Computer Selected Topics (Complex Systems)
#ESCOM - IPN 2021/1

#Última modificación 08/Enero/2021


#Módulos C++.
import OptimizacionesC

#Módulos procesamiento de datos.
import numpy as np
from math import sqrt
import re
import json
import os

#Módulos de procesamiento de dibujado.
from PIL import Image, ImageDraw

def list_to_string(s):  
    
    str1 = ""  
    
    for ele in s:  
        str1 += ele   
    
    return str1 

class GeneradorArboles(object):

    def __init__(self, regla):
        self.regla = regla
        self.B, self.S = self.validar_regla_ingresada(regla)
        

    def validar_regla_ingresada(self, regla_ingresada):
        print(type(regla_ingresada))
        if re.search("^B[0]?[1]?[2]?[3]?[4]?[5]?[6]?[7]?[8]?/S[0]?[1]?[2]?[3]?[4]?[5]?[6]?[7]?[8]?$", regla_ingresada):
            print(regla_ingresada.split("/"))
            B_S = regla_ingresada.split("/")

            B = []
            if len(B_S[0]) > 1:
                for valor in B_S[0][1:]:
                    B.append(int(valor))

            S = []
            if len(B_S[1]) > 1:
                for valor in B_S[1][1:]:
                    S.append(int(valor))

            return B, S

        else:
            return False, False

    def asignar_nombre_canonico(self, estado, is_ciclo = False, siguiente_elemento_ciclo = -1):
        """Cálculo de Knuth-tuples.
        Función utilizada para calcular el nombre canónico de un árbol y de esta manera determinar si existe algún árbol
        isomorfo dentro del conjunto. (AHU algorithm)
        """
    
        self.relacion_incidencias

        if estado not in self.relacion_incidencias.keys():
            return 2

        #Eliminar estados ciclados. 
        if self.relacion_incidencias[estado][0] == estado:
            hijos = self.relacion_incidencias[estado][1:].copy()
        elif is_ciclo:
            #Remover el siguiente elemento.
            hijos = self.relacion_incidencias[estado].copy()
            hijos.remove(siguiente_elemento_ciclo)
        else:
            hijos = self.relacion_incidencias[estado].copy()

        valores_canonicos_hijos = []
        for h in hijos:
            valores_canonicos_hijos.append(self.asignar_nombre_canonico(h))

        valores_canonicos_hijos.sort()
        valores_canonicos_binario = ['1']

        for valor in valores_canonicos_hijos:
            valores_canonicos_binario.append(bin(valor)[2:])

        valores_canonicos_binario.append('0')

        valor_canonico = list_to_string(valores_canonicos_binario)
        return int(valor_canonico, 2)

    def dibujar_hijos(self, estado, angulo_propio, rango_disponible, centro_estado, radio = 1000, is_ciclo = False, siguiente_elemento_ciclo = -1):
    
        #self.relacion_incidencias
        #self.draw
        #global self.centro_x 
        #global centro_y
        #global tamanio_cuadrado

        if self.relacion_incidencias[estado][0] == estado:
            hijos = self.relacion_incidencias[estado][1:]
        elif is_ciclo:
            #Remover el siguiente elemento.
            hijos = self.relacion_incidencias[estado]
            hijos.remove(siguiente_elemento_ciclo)
        else:
            hijos = self.relacion_incidencias[estado]

        num_hijos = len(hijos)

        rango_por_hijo = rango_disponible/num_hijos

        angulo_hijo_1 = angulo_propio-(rango_disponible/2) + rango_por_hijo/2
        #Dibujar hijo.
        offset_x, offset_y = radio*np.cos(angulo_hijo_1), radio*np.sin(angulo_hijo_1)
        self.draw.rectangle((self.centro_x+offset_x-self.tamanio_cuadrado, self.centro_y+offset_y-self.tamanio_cuadrado, self.centro_x+offset_x+self.tamanio_cuadrado, self.centro_y+offset_y+self.tamanio_cuadrado), fill = 0)
        self.draw.line((centro_estado[0], centro_estado[1], self.centro_x+offset_x, self.centro_y+offset_y), fill=128, width=1)

        if hijos[0] in self.relacion_incidencias:
            self.dibujar_hijos(hijos[0], angulo_hijo_1, rango_por_hijo, (self.centro_x+offset_x, self.centro_y+offset_y), radio+1000)


        if num_hijos > 1:
            for i, estado_hijo in enumerate(hijos[1:]):
                #Dibujar hijo.
                offset_x, offset_y = radio*np.cos(angulo_hijo_1+(i+1)*rango_por_hijo), radio*np.sin(angulo_hijo_1+(i+1)*rango_por_hijo)
                self.draw.rectangle((self.centro_x+offset_x-self.tamanio_cuadrado, self.centro_y+offset_y-self.tamanio_cuadrado, self.centro_x+offset_x+self.tamanio_cuadrado, self.centro_y+offset_y+self.tamanio_cuadrado), fill = 0)
                self.draw.line((centro_estado[0], centro_estado[1], self.centro_x+offset_x, self.centro_y+offset_y), fill=128, width=1)

                if estado_hijo in self.relacion_incidencias:
                    self.dibujar_hijos(estado_hijo, angulo_hijo_1+(i+1)*rango_por_hijo, rango_por_hijo, (self.centro_x+offset_x, self.centro_y+offset_y), radio+1000) 

    def dibujar_arboles(self):

        if not self.B:
            return -1        

        for n_dimension in range(1, 4):

            #Se hace el procesamiento de todas las combinaciones para el universo nxn
            #resultados[0] = siguiente estado del estado i.
            #resultados[1] = nivel del estado i.
            #resultados[2] = incidencias del estado i.

            if n_dimension > 3:
                resultados = OptimizacionesC.generarRelacionesArbolGrande(n_dimension, n_dimension, self.B, self.S)
            else:
                resultados = OptimizacionesC.generarRelacionesArbol(n_dimension, n_dimension, self.B, self.S)

            
            estados_ciclo_encontrados = []
            ciclos = []
            still_lifes = []

            for i in range(len(resultados[0])):
                if resultados[1][i] == 0:
                    if not (i in estados_ciclo_encontrados):
                        if not (resultados[2][i] == 1 and resultados[0][i] == i):
                            
                            ciclos.append([])
                            inicio_ciclo = i
                            estado_actual = resultados[0][i]
                            estados_ciclo_encontrados.append(inicio_ciclo)
                            ciclos[-1].append(inicio_ciclo)

                            while estado_actual != inicio_ciclo:
                                ciclos[-1].append(estado_actual)
                                estados_ciclo_encontrados.append(estado_actual)
                                estado_actual = resultados[0][estado_actual]
                        else:
                            still_lifes.append(i)

            self.relacion_incidencias = {}
            for estado, siguiente_estado in enumerate(resultados[0]):
                if not siguiente_estado in self.relacion_incidencias:
                    self.relacion_incidencias[siguiente_estado] = []
                    self.relacion_incidencias[siguiente_estado].append(estado)
                else:
                    self.relacion_incidencias[siguiente_estado].append(estado)
            
            #combinaciones = 2**(filas*columnas)
            valores_canonicos_ciclos = []
            
            for num_ciclo, ciclo in enumerate(ciclos):
                valores_canonicos_ciclos.append({})
                #self.centro_x, self.centro_y = 2500, 2500
                self.tamanio_cuadrado = 2

                if len(ciclo) > 1:
                    estados_ciclo = len(ciclo)
                    for i in range(estados_ciclo):
                        if ciclo[i] in self.relacion_incidencias:
                            
                            #Aquí se generan los valores canónicos de los árboles.
                            if i == 0:
                                sig_ele_ciclo = ciclo[-1]
                            else:
                                sig_ele_ciclo = ciclo[i-1]
                            valor_canonico = self.asignar_nombre_canonico(ciclo[i], True, sig_ele_ciclo)
                            if valor_canonico not in valores_canonicos_ciclos[-1]:
                                valores_canonicos_ciclos[-1][valor_canonico] = 1
                            else:
                                valores_canonicos_ciclos[-1][valor_canonico] += 1
                        else:
                            pass

            arboles_finales = {}
            for i, diction in enumerate(valores_canonicos_ciclos):
                if json.dumps(diction) not in arboles_finales.keys():
                    arboles_finales[json.dumps(diction)] = []
                    arboles_finales[json.dumps(diction)].append(i)
                else:
                    arboles_finales[json.dumps(diction)].append(i)

            print(arboles_finales)

            #Fragmento para el dibujado de árboles.
            
            #Creación del directorio para almacenar los árboles resultantes.
            nombre_directorio = "Resultados\{}\{}x{}".format(self.regla.replace("/", "_"), n_dimension, n_dimension)                     
            if not os.path.exists(nombre_directorio):
                os.makedirs(nombre_directorio)
            
            n = 4000
            m = 4000

            for num_arbol, arbol in enumerate(arboles_finales):
                ciclo = ciclos[arboles_finales[arbol][0]]
                image = Image.new("RGB", (n, m), (255, 255, 255))
                self.draw = ImageDraw.Draw(image)
                self.draw.rectangle(((image.size[0]/2)-5, (image.size[1]/2)-5, (image.size[0]/2)+5, (image.size[1]/2)+5), fill = 0)

                self.centro_x, self.centro_y = image.size[0]/2, image.size[1]/2
                self.tamanio_cuadrado = 2
                
                if len(ciclo) > 1:
                    estados_ciclo = len(ciclo)
                    tamanio_arco = 2*np.pi/estados_ciclo
                    radio_ciclo = 1000
                    diagonal = radio_ciclo*sqrt(2)

                    self.draw.ellipse((self.centro_x - radio_ciclo, self.centro_y - radio_ciclo, self.centro_x + radio_ciclo, self.centro_y + radio_ciclo), outline=128)
                    
                    for i in range(estados_ciclo):
                        offset_x, offset_y = radio_ciclo*np.cos(i*tamanio_arco), radio_ciclo*np.sin(i*tamanio_arco)
                        offset_x_sig, offset_y_sig = radio_ciclo*np.cos((i+1)*tamanio_arco), radio_ciclo*np.sin((i+1)*tamanio_arco)
                        self.draw.rectangle((self.centro_x+offset_x-self.tamanio_cuadrado, self.centro_y+offset_y-self.tamanio_cuadrado, self.centro_x+offset_x+self.tamanio_cuadrado, self.centro_y+offset_y+self.tamanio_cuadrado), fill = 0)

                        if ciclo[i] in self.relacion_incidencias:
                            if len(self.relacion_incidencias[ciclo[i]]) > 1:
                                if i == 0:
                                    sig_ele_ciclo = ciclo[-1]
                                else:
                                    sig_ele_ciclo = ciclo[i-1]

                                self.dibujar_hijos(ciclo[i], i*tamanio_arco, tamanio_arco, (self.centro_x+offset_x, self.centro_y+offset_y), is_ciclo=True, siguiente_elemento_ciclo=sig_ele_ciclo)

                else:
                    self.dibujar_hijos(0, 0, 360, (self.centro_x, self.centro_y))

                image.save( "{}\{}_{}.png".format(nombre_directorio, self.regla.replace("/", "_"), num_arbol),  "PNG")
                image.close()