#Bibliotecas gráficos.
import pygame
import pygame.freetype
import tkinter as tk

#Bibliotecas procesamiento.
import numpy as np
import os
import sys
import time
import random
import re

#Módulos C++
import OptimizacionesC

#Módulos Python.
import manejo_archivos_lif

class CA(object):
    """Juego de la vida implementado en Pygame."""

    background_color = pygame.Color(240, 248, 234) #Posibles colores (24, 29, 29), (1, 22, 39), (61, 8, 20), (13, 19, 33), (25, 34, 51)
    celulas_vivas_color = pygame.Color(255, 255, 255) #Blanco.
    celulas_muertas_color = pygame.Color(64, 64, 64) #Blanco.
    color_boton = pygame.Color(26, 83, 92) #Posibles (26, 83, 92)
    sombra_boton = (20, 64, 71)

    FPS = 30
    FramePerSec = pygame.time.Clock()

    def __init__(self, celulas_por_lado):

        self.largo_grid = 962
        self.ancho_grid = 600
        self.tamanio_superficie_grid = 540
        self.tamanio_superficie_desplegable = 500
        self.tamanio_celula = 10
        self.celulas_por_lado = celulas_por_lado

        self.grid_t_0 = np.zeros((celulas_por_lado, celulas_por_lado), np.int)
        self.grid_t_1 = np.zeros((celulas_por_lado, celulas_por_lado), np.int)

        self.patron = manejo_archivos_lif.leer_archivo_lif("lif/GUNSTAR.lif")
        self.offset_inicio_patron = 100
        for i in range(self.patron.shape[0]):
            for j in range(self.patron.shape[1]):
                self.grid_t_0[i+self.offset_inicio_patron, j+self.offset_inicio_patron] = self.patron[i, j]       

        self.zoom_val_desplegable = 1
        #Atributos para controlar la visualización de las células.
        self.zoom_val = 1 #1 = 1 pix por célula, 5 = 5 pix por célula, 10 = 10 pix por célula
        self.scroll_x = 0
        self.scroll_y = 0

        self.celulas_desplegadas = 500

        #    Atributos para control de scrollbar.
        self.posicion_scroll_vertical = 40
        self.posicion_scroll_horizontal = 40

        #   Atributos para control de zoom

        self.s_min = 2
        self.s_max = 3
        self.r_min = 2
        self.r_min = 2

        self.regla = "B3/S23"
        self.B = [3]
        self.S = [2, 3]
        self.generacion = 0

        self.pausa = True

    def gray(self, im):
        im = 255 * (im / im.max())
        w, h = im.shape
        ret = np.empty((w, h, 3), dtype=np.uint8)
        ret[:, :, 2] = ret[:, :, 1] = ret[:, :, 0] = im
        return ret



    def validar_regla_ingresada(self, regla_ingresada):
        print(type(regla_ingresada))
        if re.search("^B[0]?[1]?[2]?[3]?[4]?[5]?[6]?[7]?[8]?/S[0]?[1]?[2]?[3]?[4]?[5]?[6]?[7]?[8]?$", regla_ingresada):
            print("Regla válida.")
            print(regla_ingresada.split("/"))
            B_S = regla_ingresada.split("/")

            self.B = []
            if len(B_S[0]) > 1:
                for valor in B_S[0][1:]:
                    self.B.append(int(valor))

            self.S = []
            if len(B_S[1]) > 1:
                for valor in B_S[1][1:]:
                    self.S.append(int(valor))

        else:
            print("Regla inválida.")

    def cambiar_regla(self):
        if self.pausa:

            root = tk.Tk()
            canvas = tk.Canvas(root, width = 400, height = 300)
            canvas.pack()

            entrada_regla = tk.Entry(root)
            canvas.create_window(200, 140, window = entrada_regla)
            confirmar_regla_boton = tk.Button(root, text="Cambiar regla.", command=lambda: self.validar_regla_ingresada(entrada_regla.get())).pack()
            root.mainloop()

    def scrollbar(self, scrollbar_selecionada, posicion_nueva):
        if scrollbar_selecionada:
            print("Horizontal")
            if posicion_nueva > 540 - self.tamanio_grip:
                self.posicion_scroll_horizontal = 540 - self.tamanio_grip
            else:
                self.posicion_scroll_horizontal = posicion_nueva
        else:
            print("Vertical")
            if posicion_nueva > 540 - self.tamanio_grip:
                self.posicion_scroll_vertical = 540 - self.tamanio_grip
            else:
                self.posicion_scroll_vertical = posicion_nueva

        self.inicio_x = int((self.posicion_scroll_horizontal - 40) * (self.celulas_por_lado) / 500)
        self.inicio_y = int((self.posicion_scroll_vertical - 40) * (self.celulas_por_lado) / 500)

    def zoom_in(self):
        
        if self.zoom_val < 10:

            if self.zoom_val == 1:
                self.zoom_val = 5
                self.inicio_x += 125
                self.inicio_y += 125       
                self.celulas_desplegadas = 250            

            elif self.zoom_val == 5:
                self.zoom_val = 10
                self.inicio_x += 100
                self.inicio_y += 100       
                self.celulas_desplegadas = 50

            if self.inicio_x + self.celulas_desplegadas > self.celulas_por_lado:
                self.inicio_x -= self_inicio_x - self.celulas_desplegadas - self.celulas_por_lado
            if self.inicio_y + self.celulas_desplegadas > self.celulas_por_lado:
                self.inicio_y -= self_inicio_y - self.celulas_desplegadas - self.celulas_por_lado
            
            #Se modifica el tamaño del grip del scrollbar para que se visualice correctamente.
            self.tamanio_grip = (self.celulas_desplegadas/self.celulas_por_lado)*self.tamanio_superficie_grid

    def zoom_out(self):
        
        if self.zoom_val > 1:

            if self.zoom_val == 10:
                self.zoom_val = 5
                self.inicio_x -= 100
                self.inicio_y -= 100       
                self.celulas_desplegadas = 250            

            elif self.zoom_val == 5:
                self.zoom_val = 1
                self.inicio_x -= 125
                self.inicio_y -= 125       
                self.celulas_desplegadas = 500

            if self.inicio_x < 0:
                self.inicio_x = 0
            if self.inicio_y < 0:
                self.inicio_y = 0
            
            #Se modifica el tamaño del grip del scrollbar para que se visualice correctamente.
            self.tamanio_grip = (self.celulas_desplegadas/self.celulas_por_lado)*self.tamanio_superficie_grid

    def modificar_valor_celula(self, pos_mouse):
        
        if self.zoom_val == 1:           
            x = self.inicio_x + pos_mouse[0] - 40
            y = self.inicio_y + pos_mouse[1] - 40
            self.grid_t_0[x, y] = self.grid_t_0[x, y] ^ 1

        elif self.zoom_val == 5:
            x, y = int(np.floor(self.inicio_x + (pos_mouse[0]-40)/2)), int(np.floor(self.inicio_y + (pos_mouse[1]-40)/2))
            self.grid_t_0[x, y] = self.grid_t_0[x, y] ^ 1
        else:
            x, y = int(np.floor(self.inicio_x + (pos_mouse[0]-40)/10)), int(np.floor(self.inicio_y + (pos_mouse[1]-40)/10))
            self.grid_t_0[x, y] = self.grid_t_0[x, y] ^ 1

        """if self.zoom_val == 1:
            x, y = int(np.floor(self.scroll_x + pos_mouse[0]/10)), int(np.floor(self.scroll_y + pos_mouse[1]/10))
            #print("x : {} , y {}".format(np.floor(scroll_x + mouse[0]/10), np.floor(scroll_y + mouse[1]/10)))
            #print("Actual: ", grid_original[x, y])
            self.grid_t_0[x, y] = self.grid_t_0[x, y] ^ 1
            #print("Nuevo: ", grid_original[x, y])"""

    def iniciar_CA(self):

        pygame.init()
        pygame.display.set_caption('Autómata celular')
        self.font = pygame.freetype.Font('Roboto/Roboto-Light.ttf', 19)
        
        self.screen = pygame.display.set_mode((self.largo_grid, self.ancho_grid))
        #self.screen.set_alpha(128)
        self.screen.fill(self.background_color)

        #Posicion inicial de dibujado.

        self.inicio_x = 0
        self.inicio_y = 0

        #Texto y botones.

        tamanio_borde = 7
        distancia_entre_boton = 84

        #Pausa
        pygame.draw.rect(self.screen, self.sombra_boton, (720, 42, 220, 40), border_radius = tamanio_borde)
        pygame.draw.rect(self.screen, self.color_boton, (720, 37, 220, 40), border_radius = tamanio_borde)
        pausa_text_surface, pausa_text_rect = self.font.render('Pausa', (255, 255, 255))
        self.screen.blit(pausa_text_surface, (797, 50))
        

        #Cambiar regla
        pygame.draw.rect(self.screen, self.sombra_boton, (720, 42+distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        pygame.draw.rect(self.screen, self.color_boton, (720, 37+distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        cambiar_regla_text_surface, cambiar_regla_text_rect = self.font.render('Cambiar regla', (255, 255, 255))
        self.screen.blit(cambiar_regla_text_surface, (775, 50+distancia_entre_boton))

        #Cargar archivo
        pygame.draw.rect(self.screen, self.sombra_boton, (720, 42+2*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        pygame.draw.rect(self.screen, self.color_boton, (720, 37+2*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        cargar_archivo_text_surface, cargar_archivo_text_rect = self.font.render('Cargar archivo', (255, 255, 255))
        self.screen.blit(cargar_archivo_text_surface, (775, 50+2*distancia_entre_boton))

        #Guardar archivo
        pygame.draw.rect(self.screen, self.sombra_boton, (720, 42+3*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        pygame.draw.rect(self.screen, self.color_boton, (720, 37+3*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        guardar_archivo_text_surface, guardar_archivo_text_rect = self.font.render('Guardar archivo', (255, 255, 255))
        self.screen.blit(guardar_archivo_text_surface, (767, 50+3*distancia_entre_boton))

        #Generar gráfica de densidad poblacional
        pygame.draw.rect(self.screen, self.sombra_boton, (720, 42+4*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        pygame.draw.rect(self.screen, self.color_boton, (720, 37+4*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        generar_grafica_text_surface, generar_grafica_text_rect = self.font.render('Mostrar densidad', (255, 255, 255))
        self.screen.blit(generar_grafica_text_surface, (760, 50+4*distancia_entre_boton))

        #Generar atractores
        pygame.draw.rect(self.screen, self.sombra_boton, (720, 42+5*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        pygame.draw.rect(self.screen, self.color_boton, (720, 37+5*distancia_entre_boton, 220, 40), border_radius = tamanio_borde)
        generar_atractores_text_surface, generar_atractores_text_rect = self.font.render('Generar atractores', (255, 255, 255))
        self.screen.blit(generar_atractores_text_surface, (757, 50+5*distancia_entre_boton))


        #Manejo de superficie de desplegado de autómata.
        self.superficie_principal = pygame.Surface((self.tamanio_superficie_grid, self.tamanio_superficie_grid))
        self.superficie_principal.fill(self.celulas_vivas_color)

        #   Scrollbar

        self.celulas_desplegadas = 500
        self.tamanio_grip = (self.celulas_desplegadas/self.celulas_por_lado)*self.tamanio_superficie_grid

        #       Scroll vertical
        pygame.draw.rect(self.superficie_principal, self.sombra_boton, (17, 40, 18, 500))
        pygame.draw.rect(self.superficie_principal, (0, 0, 0), (19, self.posicion_scroll_vertical, 14, self.tamanio_grip))

        #       Scroll horizontal
        pygame.draw.rect(self.superficie_principal, self.sombra_boton, (40, 17, 500, 18))
        pygame.draw.rect(self.superficie_principal, (0, 0, 0), (self.posicion_scroll_horizontal, 19, self.tamanio_grip, 14))

        while(True):

            #Captura de eventos.
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.KEYDOWN:
                    print("Tecla presionada.")
                    if event.key == pygame.K_SPACE:
                        self.pausa = not self.pausa

                    if event.key == pygame.K_r:
                        self.cambiar_regla()
                        print("B: ", self.B)
                        print("S: ", self.S)

                    if event.key == pygame.K_z:
                        print("Zoom in")
                        self.zoom_in()

                    if event.key == pygame.K_x:
                        print("Zoom out")
                        self.zoom_out()

                if event.type == pygame.MOUSEBUTTONDOWN:                    
                    posicion_mouse = pygame.mouse.get_pos()
                    print("Click con el ratón en posición: ", posicion_mouse)
                    #Vertical (17, 40, 18, 500)
                    #Horizontal (40, 17, 500, 18)
                    if(posicion_mouse[0] >= 17 and posicion_mouse[0] <= 17+18 and posicion_mouse[1] >= 40 and posicion_mouse[1] <= 40 + 500):
                        #Control vertical.
                        self.scrollbar(False, posicion_mouse[1])
                                #       Scroll vertical
                        pygame.draw.rect(self.superficie_principal, self.sombra_boton, (17, 40, 18, 500))
                        pygame.draw.rect(self.superficie_principal, (0, 0, 0), (19, self.posicion_scroll_vertical, 14, self.tamanio_grip))

                    elif(posicion_mouse[0] >= 40 and posicion_mouse[0] <= 40 + 500 and posicion_mouse[1] >= 17 and posicion_mouse[1] <= 17 + 18):
                        #Control horizontal
                        self.scrollbar(True, posicion_mouse[0])
                                                #       Scroll horizontal
                        pygame.draw.rect(self.superficie_principal, self.sombra_boton, (40, 17, 500, 18))
                        pygame.draw.rect(self.superficie_principal, (0, 0, 0), (self.posicion_scroll_horizontal, 19, self.tamanio_grip, 14))
                    
                    #Verificación de cambio de estado de célula.
                    elif(posicion_mouse[0] >= 40 and posicion_mouse[0] <= 540 and posicion_mouse[0] >= 40 and posicion_mouse[0] <= 540):
                        print("Modificacion celula.")
                        self.modificar_valor_celula(posicion_mouse)


            if not self.pausa:
                # print("Juego en movimiento.")
                # self.grid_t_1 = OptimizacionesC.evaluar(self.grid_t_0, 2, 3, 3, 3).astype(np.int)    
                # print("Celulas: ", np.sum(self.grid_t_0))          
                self.grid_t_1 = OptimizacionesC.evaluar(self.grid_t_0, self.B, self.S).astype(np.int)
                self.grid_t_0 = self.grid_t_1.copy()
                numero_celulas = np.sum(self.grid_t_0)
                self.generacion += 1
                pygame.display.set_caption('Autómata celular. Generación: '+str(self.generacion)+" Número de células: "+str(numero_celulas) + " Zoom: "+str(self.zoom_val_desplegable)+" B0123456789/S0123456789")

            self.screen.blit(self.superficie_principal, (0, 0))
            
            #Redibujado del nuevo estado.
            surf_celulas = pygame.surfarray.make_surface(self.grid_t_0[self.inicio_x:self.inicio_x + self.celulas_desplegadas, self.inicio_y:self.inicio_y+self.celulas_desplegadas]*255)
            self.superficie_principal.blit(pygame.transform.scale(surf_celulas, (self.tamanio_superficie_desplegable, self.tamanio_superficie_desplegable)), (40, 40))


            pygame.display.update()
            self.FramePerSec.tick(self.FPS)

if __name__ == '__main__':
    automata_celular = CA(1500)
    automata_celular.iniciar_CA()