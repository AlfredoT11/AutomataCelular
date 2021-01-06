import pygame
import pygame.freetype
from tkinter import filedialog
from tkinter import *

import numpy as np

import os
import sys
import time
import random

#Modulos C++
import OptimizacionesC

#Modulos Python
import manejo_archivos_lif


patron = manejo_archivos_lif.leer_archivo_lif("lif/AQUA50FA.LIF")

start = time.time()

#Configuraciones iniciales de la interfaz gráfica.
pygame.init()

pygame.display.set_caption('Autómata celular')

FPS = 30
FramePerSec = pygame.time.Clock()

roboto_font = pygame.freetype.Font('Roboto/Roboto-Thin.ttf', 24)

largo_grid, ancho_grid = 890, 700
background_color = pygame.Color(64, 64, 64) #Gris.
lines_color = pygame.Color(255, 255, 255) #Blanco.
live_cells_color = pygame.Color(255, 255, 255) #Blanco.
dead_cells_color = pygame.Color(64, 64, 64) #Gris.

tamanio_celula = 10
tamanio_grid = 1000

tamanio_superficie_grid = 700
zoom_val_desplegable = 1
zoom_val = 10


#Configuraciones básicas del Automata.
s_min, s_max = 2, 3
n_min, n_max = 3, 3


grid_original = np.zeros((tamanio_grid, tamanio_grid), np.int)
grid_nuevo = np.zeros((tamanio_grid, tamanio_grid), np.int)

print("Shape: ", grid_original.shape)
superficie_principal = pygame.Surface((tamanio_superficie_grid, tamanio_superficie_grid))
superficie_principal.fill(live_cells_color)



filas_patron, columnas_patron = patron.shape
offset_inicio_patron = 100

for i in range(filas_patron):
    for j in range(columnas_patron):
        grid_original[i+offset_inicio_patron, j+offset_inicio_patron] = patron[i, j]

numero_celulas = np.sum(grid_original)
generacion = 0

seccion_x = 0
seccion_y = 0

scroll_x, scroll_y = 0, 0

def zoom(e):
    #Zoom in.
    global zoom_val
    global zoom_val_desplegable
    if (e.key == pygame.K_x):
        if(zoom_val == 10):
            zoom_val = 5
            zoom_val_desplegable = 5
        elif(zoom_val == 5):
            zoom_val = 1
            zoom_val_desplegable = 10
    elif (e.key == pygame.K_z):
        if(zoom_val == 1):
            zoom_val = 5
            zoom_val_desplegable = 5
        elif(zoom_val == 5):
            zoom_val = 10
            zoom_val_desplegable = 1

def scrolling(e):

    global seccion_x, seccion_y
    global scroll_x, scroll_y

    aumento = 10
    mostrados_pantalla = 70

    if e.key == pygame.K_LEFT:
        if scroll_x == 0:
            print("Limite izquierdo alcanzado.")
        else:
            scroll_x -= aumento
    elif e.key == pygame.K_RIGHT:
        if scroll_x + zoom_val*mostrados_pantalla + aumento > tamanio_grid:
            print("Limite derecho alcanzado.")
        else:
            scroll_x += aumento
    elif e.key == pygame.K_UP:
        if scroll_y == 0:
            print("Limite superior alcanzado.")
        else:
            scroll_y -= aumento
    elif e.key == pygame.K_DOWN:
        if scroll_y + zoom_val*mostrados_pantalla + aumento > tamanio_grid:
            print("Limite inferior alcanzado.")
        else:
            scroll_y += aumento
    
    print("{} , {}".format(seccion_x, seccion_y))


#Se dibuja el grid.
screen = pygame.display.set_mode((largo_grid, ancho_grid))
screen.set_alpha(None)
screen.fill(background_color)

#----------------------------------------------
s_min_text_surface, s_min_text_rect = roboto_font.render('S min: ', live_cells_color)
s_min_value_text_surface, s_min_value_text_rect = roboto_font.render(str(s_min), live_cells_color)

s_max_text_surface, s_max_text_rect = roboto_font.render('S max: ', live_cells_color)
s_max_value_text_surface, s_max_value_text_rect = roboto_font.render(str(s_max), live_cells_color)

r_min_text_surface, r_min_text_rect = roboto_font.render('R min: ', live_cells_color)
r_min_value_text_surface, r_min_value_text_rect = roboto_font.render(str(n_min), live_cells_color)

r_max_text_surface, r_max_text_rect = roboto_font.render('R max: ', live_cells_color)
r_max_value_text_surface, r_max_value_text_rect = roboto_font.render(str(n_max), live_cells_color)


disminuir_texto_surface, disminuir_texto_rect = roboto_font.render('', live_cells_color)
aumentar_texto_surface, aumentar_texto_rect = roboto_font.render('', live_cells_color)

pausa = False

#Posiciones de dibujo.
x_inicial_disminuir_s_min, y_inicial_disminuir_s_min = 794, 500
x_inicial_aumentar_s_min, y_inicial_aumentar_s_min = 830, 503
x_inicial_disminuir_s_max, y_inicial_disminuir_s_max = 801, 535
x_inicial_aumentar_s_max, y_inicial_aumentar_s_max = 847, 527
x_inicial_disminuir_r_min, y_inicial_disminuir_r_min = 801, 579
x_inicial_aumentar_r_min, y_inicial_aumentar_r_min = 847, 572
x_inicial_disminuir_r_max, y_inicial_disminuir_r_max = 801, 607
x_inicial_aumentar_r_max, y_inicial_aumentar_r_max = 847, 600

text_surface_x = 720
text_value_surface_x = 825
s_min_y = 500
s_max_y = 527
r_min_y = 568
r_max_y = 600

tamanio_botones_aumentar_disminuir = 20

screen.blit(s_min_text_surface, (text_surface_x, s_min_y))
screen.blit(s_min_value_text_surface, (text_value_surface_x, s_min_y))

screen.blit(s_max_text_surface, (text_surface_x, s_max_y))
screen.blit(s_max_value_text_surface, (text_value_surface_x, s_max_y))

screen.blit(r_min_text_surface, (text_surface_x, r_min_y))
screen.blit(r_min_value_text_surface, (text_value_surface_x, r_min_y))

screen.blit(r_max_text_surface, (text_surface_x, r_max_y))
screen.blit(r_max_value_text_surface, (text_value_surface_x, r_max_y))

def cambiar_regla(e):
    
    global generacion
    global numero_celulas

    if pausa:
        if e.key == pygame.K_r:
            
            root = Tk()
            canvas = Canvas(root, width = 400, height = 300)
            canvas.pack()
            #root.filename =  filedialog.askopenfilename(initialdir = os.getcwdb(),title = "Selecciona el archivo .lif.",filetypes = (("Life files","*.LIF"),("all files","*.*")))
            #print (root.filename)
            #print(type(root.filename))
            #ruta_archivo = root.filename
            #Button(root, text="Cargar archivo .lif", command=root.destroy).pack()
            
            entrada1 = Entry(root)
            canvas.create_window(200, 140, window = entrada1)
            boton1 = Button(root, text="Cambiar regla.", command=root.destroy).pack()
            canvas.create_window(200, 180, window = boton1)

            root.mainloop()
            

            """print(ruta_archivo)
            patron = manejo_archivos_lif.leer_archivo_lif(ruta_archivo)
            filas_patron, columnas_patron = patron.shape
            grid_original.fill(0)
            grid_nuevo.fill(0)            

            for i in range(filas_patron):
                for j in range(columnas_patron):
                    grid_original[i+offset_inicio_patron-1, j+offset_inicio_patron] = patron[i, j]
            
            generacion = 0
            numero_celulas = sum(grid_original) """

def cargar_archivo(e):
    
    global generacion
    global numero_celulas

    if pausa:
        if e.key == pygame.K_l:
            
            root = Tk()
            root.filename =  filedialog.askopenfilename(initialdir = os.getcwdb(),title = "Selecciona el archivo .lif.",filetypes = (("Life files","*.LIF"),("all files","*.*")))
            print (root.filename)
            print(type(root.filename))
            ruta_archivo = root.filename
            Button(root, text="Cargar archivo .lif", command=root.destroy).pack()
            root.mainloop()
            
            print(ruta_archivo)
            patron = manejo_archivos_lif.leer_archivo_lif(ruta_archivo)
            filas_patron, columnas_patron = patron.shape
            grid_original.fill(0)
            grid_nuevo.fill(0)            

            for i in range(filas_patron):
                for j in range(columnas_patron):
                    grid_original[i+offset_inicio_patron-1, j+offset_inicio_patron] = patron[i, j]
            
            generacion = 0
            numero_celulas = sum(grid_original)



def verificar_posicion_presionada():
    mouse = pygame.mouse.get_pos()

    if(mouse[0] < tamanio_superficie_grid and mouse[1] < tamanio_superficie_grid):
        if zoom_val == 1:
            x, y = int(np.floor(scroll_x + mouse[0]/10)), int(np.floor(scroll_y + mouse[1]/10))
            print("x : {} , y {}".format(np.floor(scroll_x + mouse[0]/10), np.floor(scroll_y + mouse[1]/10)))
            print("Actual: ", grid_original[x, y])
            grid_original[x, y] = grid_original[x, y] ^ 1
            print("Nuevo: ", grid_original[x, y])
            
    
    else:
        if x_inicial_disminuir_s_min <= mouse[0] <= x_inicial_disminuir_s_min+tamanio_botones_aumentar_disminuir and y_inicial_disminuir_s_min <= mouse[1] <= y_inicial_disminuir_s_min+tamanio_botones_aumentar_disminuir:
            print("Disminuir S min")
        elif x_inicial_aumentar_s_min <= mouse[0] <= x_inicial_aumentar_s_min+tamanio_botones_aumentar_disminuir and y_inicial_aumentar_s_min <= mouse[1] <= y_inicial_aumentar_s_min+tamanio_botones_aumentar_disminuir:
            print("Aumentar S min")
        elif x_inicial_disminuir_s_max <= mouse[0] <= x_inicial_disminuir_s_max+tamanio_botones_aumentar_disminuir and y_inicial_disminuir_s_max <= mouse[1] <= y_inicial_disminuir_s_max+tamanio_botones_aumentar_disminuir:
            print("Disminuir S max")
        elif x_inicial_aumentar_s_max <= mouse[0] <= x_inicial_aumentar_s_max+tamanio_botones_aumentar_disminuir and y_inicial_aumentar_s_max <= mouse[1] <= y_inicial_aumentar_s_max+tamanio_botones_aumentar_disminuir:
            print("Aumentar S max")
        if x_inicial_disminuir_r_min <= mouse[0] <= x_inicial_disminuir_r_min+tamanio_botones_aumentar_disminuir and y_inicial_disminuir_r_min <= mouse[1] <= y_inicial_disminuir_r_min+tamanio_botones_aumentar_disminuir:
            print("Disminuir R min")
        elif x_inicial_aumentar_r_min <= mouse[0] <= x_inicial_aumentar_r_min+tamanio_botones_aumentar_disminuir and y_inicial_aumentar_r_min <= mouse[1] <= y_inicial_aumentar_r_min+tamanio_botones_aumentar_disminuir:
            print("Aumentar R min")
        elif x_inicial_disminuir_s_max <= mouse[0] <= x_inicial_disminuir_r_max+tamanio_botones_aumentar_disminuir and y_inicial_disminuir_r_max <= mouse[1] <= y_inicial_disminuir_r_max+tamanio_botones_aumentar_disminuir:
            print("Disminuir R max")
        elif x_inicial_aumentar_r_max <= mouse[0] <= x_inicial_aumentar_r_max+tamanio_botones_aumentar_disminuir and y_inicial_aumentar_r_max <= mouse[1] <= y_inicial_aumentar_r_max+tamanio_botones_aumentar_disminuir:
            print("Aumentar R max")               

#Prueba scrolling
boxx = 200
boxy = 200
speed = 5

while(True):
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()
        
        mouse_pos = pygame.mouse.get_pos()


        if event.type == pygame.KEYDOWN:
            print("Tecla presionada.")
            if event.key == pygame.K_SPACE:
                pausa = not pausa
            else:
                #pass
                zoom(event)
                scrolling(event)
            
            cargar_archivo(event)
            cambiar_regla(event)
        
        if event.type == pygame.MOUSEBUTTONDOWN:
            verificar_posicion_presionada()

    if not pausa:
        
        
        grid_nuevo = OptimizacionesC.evaluar(grid_original, 2, 3, 3, 3).astype(np.int)
        grid_original = grid_nuevo.copy()
        numero_celulas = np.sum(grid_original)
        generacion += 1
        pygame.display.set_caption('Autómata celular. Generación: '+str(generacion)+" Número de células: "+str(numero_celulas) + " Zoom: "+str(zoom_val_desplegable))
        
        #end = time.time()
        #print(end - inicio_hash)

    screen.blit(superficie_principal, (0, 0))
    limite_dibujo = 70*zoom_val
    surf = pygame.surfarray.make_surface(grid_original[0+scroll_x:limite_dibujo+scroll_x, 0+scroll_y:limite_dibujo+scroll_y])
    
    
    superficie_principal.blit(pygame.transform.scale(surf, (tamanio_superficie_grid, tamanio_superficie_grid)), (0, 0))
    
    #Smin
    screen.blit(disminuir_texto_surface, (x_inicial_disminuir_s_min+7, y_inicial_disminuir_s_min+9))
    screen.blit(aumentar_texto_surface, (x_inicial_disminuir_s_min+52, y_inicial_disminuir_s_min+3))

    #Smax
    screen.blit(disminuir_texto_surface, (x_inicial_disminuir_s_max, y_inicial_disminuir_s_max))
    screen.blit(aumentar_texto_surface, (x_inicial_aumentar_s_max, y_inicial_aumentar_s_max))

    #Rmin
    screen.blit(disminuir_texto_surface, (x_inicial_disminuir_r_min, y_inicial_disminuir_r_min))
    screen.blit(aumentar_texto_surface, (x_inicial_aumentar_r_min, y_inicial_aumentar_r_min))

    #Rmax
    screen.blit(disminuir_texto_surface, (x_inicial_disminuir_r_max, y_inicial_disminuir_r_max))
    screen.blit(aumentar_texto_surface, (x_inicial_aumentar_r_max, y_inicial_aumentar_r_max))        

    pygame.display.update()
    FramePerSec.tick(FPS)