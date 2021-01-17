#include <iostream>
#include <cmath>

//Biblioteca Pybind11 para usar código en Python.
#include <../../../AutomataCelular/venv/Lib/site-packages/pybind11/include/pybind11/pybind11.h>
#include <../../../AutomataCelular/venv/Lib/site-packages/pybind11/include/pybind11/numpy.h>

//Biblioteca para multithreading.
#include <Windows.h>

namespace py = pybind11;
using namespace std;

int potencia(int x, int p) {
    if (p == 0) return 1;
    if (p == 1) return x;

    int tmp = potencia(x, p / 2);
    if (p % 2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
}

int asignarNivel(int combinacion, int& posInicioCiclo, int* relacionEstados, int* niveles, vector<int>& pilaRegistro) {

    //Se verifica si el siguiente estado tiene un nivel superior a 0, de ser así, se le asigna el nivel siguiente a este. 
    if (niveles[relacionEstados[combinacion]] > 0) {
        niveles[combinacion] = niveles[relacionEstados[combinacion]] + 1;
        return niveles[relacionEstados[combinacion]] + 1;
    }

    int posicionEnPila = -1;
    for (int i = 0; i < pilaRegistro.size(); i++) {
        if (pilaRegistro[i] == combinacion) {
            posicionEnPila = i;
            break;
        }
    }

    if (posicionEnPila != -1) {
        if (niveles[combinacion] == -1) {
            niveles[combinacion] = 0;
        }
        posInicioCiclo = posicionEnPila;
        return 0;
    }
    else {
        pilaRegistro.push_back(combinacion);
        int anterior = asignarNivel(relacionEstados[combinacion], posInicioCiclo, relacionEstados, niveles, pilaRegistro);
        if (pilaRegistro.size() <= posInicioCiclo) {
            niveles[combinacion] = anterior + 1;
        }
        else {
            niveles[combinacion] = 0;
        }
        pilaRegistro.pop_back();
        return niveles[combinacion];
    }
}


py::list generarRelacionesArbol(int filas, int columnas, py::list B, py::list S) {
    //Se almacena memoria para el espacio del autómata celular.
    bool** grid_automata = (bool**)calloc(filas, sizeof(bool*));
    for (int i = 0; i < filas; i++) {
        grid_automata[i] = (bool*)calloc(columnas, sizeof(bool));
    }

    int combinaciones = potencia(2, filas * columnas);
    std::cout << "Combinaciones: " << combinaciones << std::endl;

    //Se almacena espacio para la relación de estados.
    //py::array_t<int> relacionEstados = py::array_t<int>(combinaciones);
    int* relacionEstados = (int*)calloc(combinaciones, sizeof(int));
    py::array_t<int> relacionesResultantes = py::array_t<int>(combinaciones);

    //Se almacena espacio para el conteo de estados de llegada a cada estado.
    int* contadoresIncidencia = (int*)calloc(combinaciones, sizeof(int));

    //Se almacena espacio para el nivel de cada estado para la representación gráfica.
    int* nivelEstado = (int*)calloc(combinaciones, sizeof(int));
    for (int i = 0; i < combinaciones; i++) {
        nivelEstado[i] = -1;
    }

    //Se almacena espacio para el nuevo estado procesado (Se almacena en una sola dimensión por simplicidad)
    bool* nuevoEstado = (bool*)calloc(filas * columnas, sizeof(bool));

    //Preparando las reglas
    int tamanioB = B.size();
    int tamanioS = S.size();

    int* BC = (int*)calloc(tamanioB, sizeof(int));
    int* SC = (int*)calloc(tamanioS, sizeof(int));

    for (int i = 0; i < tamanioB; i++) {
        BC[i] = B[i].cast<int>();
    }

    for (int i = 0; i < tamanioS; i++) {
        SC[i] = S[i].cast<int>();
    }


    //Inicia el recorrido de todas las posibles combinaciones.
    for (int numCombinacion = 0; numCombinacion < combinaciones; numCombinacion++) {

        for (int k = 0; k < filas * columnas; k++) {
            nuevoEstado[k] = false;
        }

        //Se obtiene el estado a evaluar.
        int posBit = 0;
        for (int i = filas - 1; i >= 0; i--) {
            for (int j = columnas - 1; j >= 0; j--) {
                grid_automata[i][j] = numCombinacion & (1 << posBit);
                posBit++;
            }
        }

        //Procesamiento de las celulas.
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                int indiceIzquierdo = ((j - 1) % columnas + columnas) % columnas;
                int indiceDerecho = ((j + 1) % columnas + columnas) % columnas;
                int indiceSuperior = ((i - 1) % filas + filas) % filas;
                int indiceInferior = ((i + 1) % filas + filas) % filas;

                //std::cout << "Indices calculados." << std::endl;   
                //std::cout << "izq: " << indiceIzquierdo << " der: " << indiceDerecho << " sup: " << indiceSuperior << " inf: " << indiceInferior << std::endl;   

                int sumaVecinos = grid_automata[indiceSuperior][indiceIzquierdo] + grid_automata[indiceSuperior][j] + grid_automata[indiceSuperior][indiceDerecho] +
                    grid_automata[i][indiceIzquierdo] + grid_automata[i][indiceDerecho] +
                    grid_automata[indiceInferior][indiceIzquierdo] + grid_automata[indiceInferior][j] + grid_automata[indiceInferior][indiceDerecho];

                //std::cout << "Suma vecinos calculada: " << sumaVecinos << std::endl;

                if (grid_automata[i][j]) {
                    bool encontrado = false;
                    for (int k = 0; k < tamanioS; k++) {
                        if (sumaVecinos == SC[k]) {
                            encontrado = true;
                            nuevoEstado[j + i * filas] = 1;
                            break;
                        }
                    }
                }
                else {
                    for (int k = 0; k < tamanioB; k++) {
                        if (sumaVecinos == BC[k]) {
                            nuevoEstado[j + i * filas] = 1;
                            break;
                        }
                    }
                }
            }

        }

        int nuevoEstadoValor = 0;
        for (int k = 0; k < filas * columnas; ++k) {
            if (nuevoEstado[k]) {
                nuevoEstadoValor |= 1 << filas * columnas - k - 1;
            }
        }

        relacionEstados[numCombinacion] = nuevoEstadoValor;
        contadoresIncidencia[nuevoEstadoValor]++;

    }

    /*for (int i = 0; i < combinaciones; i++) {
        std::cout << relacionEstados[i] << std::endl;
    }*/

    //relacionesResultantes.resize({ combinaciones,1 });
    //return relacionesResultantes;

    for (int i = 0; i < combinaciones; i++) {

        vector<int> pilaRegistroNiveles;
        int posicionCiclo = -1;
        asignarNivel(i, posicionCiclo, relacionEstados, nivelEstado, pilaRegistroNiveles);
    }

    /*for (int i = 0; i < combinaciones; i++) {
        cout << "Combinacion: " << i << " Siguiente: " << relacionEstados[i] << " Nivel combinacion: " << nivelEstado[i] << endl;
    }*/

    py::list resultados;
    resultados.append(py::list(py::array_t<int>(
        { combinaciones },
        { 4 },
        relacionEstados
        //free_when_done);
        )));

    resultados.append(py::list(py::array_t<int>(
        { combinaciones },
        { 4 },
        nivelEstado
        //free_when_done);
        )));

    resultados.append(py::list(py::array_t<int>(
        { combinaciones },
        { 4 },
        contadoresIncidencia
        //free_when_done);
        )));

    return resultados;

    /*return py::array_t<int>(
        { combinaciones },
        { 4 },
        relacionEstados
        //free_when_done);
        );*/

}

struct DatosHilo {
    
    short int filaInicio;
    short int filaFinal;
    short int columnaInicio;
    short int columnaFinal;
    
    short int filasTotales;
    short int columnasTotales;

    short int tamanioB;
    short int tamanioS;

    int* BC;
    int* SC;
    
    bool* apuntadorGridt0;
    bool* apuntadorGridt1;
};

DWORD WINAPI evaluarFragmentoGrid(LPVOID lpParameter) {
    //cout << "Hola, soy un hilo." << endl;

    //cout << "Rango filas: " << ((DatosHilo*)lpParameter)->filaInicio << ", " << ((DatosHilo*)lpParameter)->filaFinal << endl;
    //cout << "Rango columnas: " << ((DatosHilo*)lpParameter)->columnaInicio << ", " << ((DatosHilo*)lpParameter)->columnaFinal << endl;

    bool* ptr1 = ((DatosHilo*)lpParameter)->apuntadorGridt0;
    bool* ptr3 = ((DatosHilo*)lpParameter)->apuntadorGridt1;

    short int Y = ((DatosHilo*)lpParameter)->columnasTotales;
    short int X = ((DatosHilo*)lpParameter)->filasTotales;

    short int tamanioS = ((DatosHilo*)lpParameter)->tamanioS;
    short int tamanioB = ((DatosHilo*)lpParameter)->tamanioB;

    int* SC = ((DatosHilo*)lpParameter)->SC;
    int* BC = ((DatosHilo*)lpParameter)->BC;

    //cout << "Valor inicial SC: " << SC[0] << endl;

    short int filaInicial, filaFinal, columnaInicial, columnaFinal;

    if (((DatosHilo*)lpParameter)->filaInicio == 0) {
        filaInicial = 1;
        filaFinal = ((DatosHilo*)lpParameter)->filaFinal - 1;
    }
    else {
        filaInicial = ((DatosHilo*)lpParameter)->filaInicio;
        filaFinal = ((DatosHilo*)lpParameter)->filaFinal - 2;
    }

    if (((DatosHilo*)lpParameter)->columnaInicio == 0) {
        columnaInicial = 1;
        columnaFinal = ((DatosHilo*)lpParameter)->columnaFinal - 1;
    }
    else {
        columnaInicial = ((DatosHilo*)lpParameter)->columnaInicio;
        columnaFinal = ((DatosHilo*)lpParameter)->columnaFinal - 2;
    }


    //cout << " Fila: ( " << filaInicial << ", " << filaFinal << " )" << endl;
    //cout << " Columna: ( " << columnaInicial << ", " << columnaFinal << " )" << endl;

    //cout << "Apuntador 1: " << ((DatosHilo*)lpParameter)->apuntadorGridt0 << " Apuntador 3: " << ((DatosHilo*)lpParameter)->apuntadorGridt1 << endl;
    //cout << "Dato 1: " << *(((DatosHilo*)lpParameter)->apuntadorGridt0) << " Dato 3: " << *(((DatosHilo*)lpParameter)->apuntadorGridt1) << endl;


    for (short int i = filaInicial; i <= filaFinal; i++) {
        for (short int j = columnaInicial; j <= columnaFinal; j++) {
            short int sumaVecinos = 0;
            sumaVecinos = ptr1[j - 1 + Y * (i - 1)] + ptr1[j + Y * (i - 1)] + ptr1[j + 1 + Y * (i - 1)] +
                ptr1[j - 1 + i * Y] + ptr1[j + 1 + i * Y] +
                ptr1[j - 1 + Y * (i + 1)] + ptr1[j + Y * (i + 1)] + ptr1[j + 1 + Y * (i + 1)];

            if (ptr1[j + i * Y] == 1) {
                bool encontrado = false;
                for (short int k = 0; k < tamanioS; k++) {
                    if (sumaVecinos == SC[k]) {
                        encontrado = true;
                        ptr3[j + i * Y] = 1;
                        break;
                    }
                }


            }
            else {
                for (int k = 0; k < tamanioB; k++) {
                    if (sumaVecinos == BC[k]) {
                        ptr3[j + i * Y] = 1;
                        break;
                    }
                }
            }
        }
    }
    //cout << "Valor por referencia: " << ptr1[1] << endl;

    return 0;
}

//py::array_t<bool> evaluar(py::array_t<bool> input1, int s_min, int s_max, int r_min, int r_max) {
py::list evaluar(py::array_t<bool> input1, py::list B, py::list S) {
    py::buffer_info buf1 = input1.request();

    /*  allocate the buffer */
    py::array_t<bool> result = py::array_t<bool>(buf1.size);

    py::buffer_info buf3 = result.request();

    bool* ptr1 = (bool*)buf1.ptr,
        * ptr3 = (bool*)buf3.ptr;
    int X = buf1.shape[0];
    int Y = buf1.shape[1];

    //Preparando las reglas
    int tamanioB = B.size();
    int tamanioS = S.size();

    int* BC = (int*)calloc(tamanioB, sizeof(int));
    int* SC = (int*)calloc(tamanioS, sizeof(int));

    for (int i = 0; i < tamanioB; i++) {
        BC[i] = B[i].cast<int>();
    }

    for (int i = 0; i < tamanioS; i++) {
        SC[i] = S[i].cast<int>();
    }

    /*
    std::cout << " " << std::endl;
    std::cout << " " << std::endl;
    for (int i = 0; i < tamanioB; i++) {
        std::cout << BC[i] << ", ";
    }
    std::cout << " " << std::endl;;
    for (int i = 0; i < tamanioS; i++) {
        std::cout << SC[i] << ", ";
    }
    std::cout << " " << std::endl;;
    */
    
    //printf("Altura %d Ancho %d", X, Y);
    //printf("R(%d %d %d %d)", s_min, s_max, r_min, r_max);

    /*bool** grid_automata = (bool**)calloc(X, sizeof(bool*));
    for (int i = 0; i < X; i++) {
        grid_automata[i] = (bool*)calloc(Y, sizeof(bool));
    }

    for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++) {
            grid_automata[X * i + j];
        }
    }

    //Procesamiento de las células
    // X = filas, Y = columnas
    for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++) {
            int indiceIzquierdo = ((j - 1) % Y + Y) % Y;
            int indiceDerecho = ((j + 1) % Y + Y) % Y;
            int indiceSuperior = ((i - 1) % X + X) % X;
            int indiceInferior = ((i + 1) % X + X) % X;

            int sumaVecinos = grid_automata[indiceSuperior][indiceIzquierdo] + grid_automata[indiceSuperior][j] + grid_automata[indiceSuperior][indiceDerecho] +
                grid_automata[i][indiceIzquierdo] + grid_automata[i][indiceDerecho] +
                grid_automata[indiceInferior][indiceIzquierdo] + grid_automata[indiceInferior][j] + grid_automata[indiceInferior][indiceDerecho];
        
            if (grid_automata[i][j]) {
                bool encontrado = false;
                for (int k = 0; k < tamanioS; k++) {
                    if (sumaVecinos == SC[k]) {
                        encontrado = true;
                        ptr3[j + i * X] = 1;
                        break;
                    }
                }
            }
            else {
                for (int k = 0; k < tamanioB; k++) {
                    if (sumaVecinos == BC[k]) {
                        ptr3[j + i * X] = 1;
                        break;
                    }
                }
            }
        
        }
    }*/

    // X = filas, Y = columnas
    /*int* datosHilo00 = (int*)calloc(4, sizeof(int));  // 00 | 01
    int* datosHilo01 = (int*)calloc(4, sizeof(int));  // 10 | 11
    int* datosHilo10 = (int*)calloc(4, sizeof(int));
    int* datosHilo11 = (int*)calloc(4, sizeof(int));*/

    //Configuración y creación de hilos para el proceasmiento en paralelo del universo.
    DatosHilo datosHilo00;
    DatosHilo datosHilo01;
    DatosHilo datosHilo10;
    DatosHilo datosHilo11;

    int* contadorEstados = (int*)calloc(512, sizeof(int));


    // 00 | 01
    // 10 | 11

    if (X % 2 == 0) {
        datosHilo00.filaInicio = 0;
        datosHilo00.filaFinal = X / 2;
        datosHilo01.filaInicio = 0;
        datosHilo01.filaFinal = X / 2;
        
        datosHilo10.filaInicio = X / 2;
        datosHilo10.filaFinal = X;
        datosHilo11.filaInicio = X / 2;
        datosHilo11.filaFinal = X;
    }
    else {
        datosHilo00.filaInicio = 0;
        datosHilo00.filaFinal = 1 + X / 2;
        datosHilo01.filaInicio = 0;
        datosHilo01.filaFinal = 1 + X / 2;
        
        datosHilo10.filaInicio = 1 + X / 2;
        datosHilo10.filaFinal = X;
        datosHilo11.filaInicio = 1 + X / 2;
        datosHilo11.filaFinal = X;
    }
    
    // 00 | 01
    // 10 | 11

    if (Y % 2 == 0) {
        datosHilo00.columnaInicio = 0;
        datosHilo00.columnaFinal = Y / 2;
        datosHilo10.columnaInicio = 0;
        datosHilo10.columnaFinal = Y / 2;
        
        datosHilo01.columnaInicio = Y / 2;
        datosHilo01.columnaFinal = Y;
        datosHilo11.columnaInicio = Y / 2;
        datosHilo11.columnaFinal = Y;
    }
    else {
        datosHilo00.columnaInicio = 0;
        datosHilo00.columnaFinal = 1 + Y / 2;
        datosHilo10.columnaInicio = 0;
        datosHilo10.columnaFinal = 1 + Y / 2;
        
        datosHilo01.columnaInicio = 1 + Y / 2;
        datosHilo01.columnaFinal = Y;
        datosHilo11.columnaInicio = 1 + Y / 2;
        datosHilo11.columnaFinal = Y;
    }

    datosHilo00.apuntadorGridt0 = ptr1;
    datosHilo01.apuntadorGridt0 = ptr1;
    datosHilo10.apuntadorGridt0 = ptr1;
    datosHilo11.apuntadorGridt0 = ptr1;

    datosHilo00.apuntadorGridt1 = ptr3;
    datosHilo01.apuntadorGridt1 = ptr3;
    datosHilo10.apuntadorGridt1 = ptr3;
    datosHilo11.apuntadorGridt1 = ptr3;

    datosHilo00.SC = SC;
    datosHilo01.SC = SC;
    datosHilo10.SC = SC;
    datosHilo11.SC = SC;

    datosHilo00.BC = BC;
    datosHilo01.BC = BC;
    datosHilo10.BC = BC;
    datosHilo11.BC = BC;

    datosHilo00.tamanioB = tamanioB;
    datosHilo01.tamanioB = tamanioB;
    datosHilo10.tamanioB = tamanioB;
    datosHilo11.tamanioB = tamanioB;

    datosHilo00.tamanioS = tamanioS;
    datosHilo01.tamanioS = tamanioS;
    datosHilo10.tamanioS = tamanioS;
    datosHilo11.tamanioS = tamanioS;

    datosHilo00.filasTotales = X;
    datosHilo01.filasTotales = X;
    datosHilo10.filasTotales = X;
    datosHilo11.filasTotales = X;

    datosHilo00.columnasTotales = Y;
    datosHilo01.columnasTotales = Y;
    datosHilo10.columnasTotales = Y;
    datosHilo11.columnasTotales = Y;

    DWORD hilo00;
    DWORD hilo01;
    DWORD hilo10;
    DWORD hilo11;

    HANDLE handleHilo00 = CreateThread(0, 0, evaluarFragmentoGrid, &datosHilo00, 0, &hilo00);    
    HANDLE handleHilo01 = CreateThread(0, 0, evaluarFragmentoGrid, &datosHilo01, 0, &hilo01);    
    HANDLE handleHilo10 = CreateThread(0, 0, evaluarFragmentoGrid, &datosHilo10, 0, &hilo10);    
    HANDLE handleHilo11 = CreateThread(0, 0, evaluarFragmentoGrid, &datosHilo11, 0, &hilo11);

    //El programa debe esperar hasta que los 4 hilos terminen de ejecutarse.
    
    
    WaitForSingleObject(handleHilo00, INFINITE);
    WaitForSingleObject(handleHilo01, INFINITE);
    WaitForSingleObject(handleHilo10, INFINITE);
    WaitForSingleObject(handleHilo11, INFINITE);


    //Procesamiento de todo el universo en 1 solo hilo.
    /*
    for (int i = 1; i < X - 1; i++) {
        for (int j = 1; j < Y - 1; j++) {
            int sumaVecinos = 0;
            sumaVecinos = ptr1[j - 1 + Y * (i - 1)] + ptr1[j + Y * (i - 1)] + ptr1[j + 1 + Y * (i - 1)] +
                ptr1[j - 1 + i * Y] + ptr1[j + 1 + i * Y] +
                ptr1[j - 1 + Y * (i + 1)] + ptr1[j + Y * (i + 1)] + ptr1[j + 1 + Y * (i + 1)];

            if (ptr1[j + i * Y] == 1) {
                bool encontrado = false;
                for (int k = 0; k < tamanioS; k++) {
                    if (sumaVecinos == SC[k]) {
                        encontrado = true;
                        ptr3[j + i * Y] = 1;
                        break;
                    }
                }

                
            }else{                
                for (int k = 0; k < tamanioB; k++) {
                    if (sumaVecinos == BC[k]) {
                        ptr3[j + i * Y] = 1;
                        break;
                    }
                }
            }

            /*if (sumaVecinos >= s_min && sumaVecinos <= s_max && ptr1[j + i * Y] == 1) {
                ptr3[j + i * Y] = 1;
            }
            else if (sumaVecinos >= r_min && sumaVecinos <= r_max && ptr1[j + i * Y] == 0) {
                ptr3[j + i * Y] = 1;
            }
            else {
                ptr3[j + i * Y] = 0;
            }*/
     //   }

    //}

    // X = Filas, Y = Columnas

    //cout << "Calculos especiales..." << endl;
    int offsetCelulasInferiores = Y * (X-1);
    for (int j = 1; j < Y - 1; j++) {

        int posIzquierda = j - 1;
        int posDerecha = j + 1;

        int sumaVecinos = 0;
        sumaVecinos = ptr1[posIzquierda  + offsetCelulasInferiores] + ptr1[j + offsetCelulasInferiores] + ptr1[posDerecha + offsetCelulasInferiores] +
                      ptr1[posIzquierda]                                                                + ptr1[posDerecha] +
                      ptr1[posIzquierda + Y] + ptr1[j + Y]                                              + ptr1[posDerecha + Y];

        if (ptr1[j] == 1) {
            bool encontrado = false;
            for (int k = 0; k < tamanioS; k++) {
                if (sumaVecinos == SC[k]) {
                    encontrado = true;
                    ptr3[j] = 1;
                    break;
                }
            }
        }
        else {
            for (int k = 0; k < tamanioB; k++) {
                if (sumaVecinos == BC[k]) {
                    ptr3[j] = 1;
                    break;
                }
            }
        }

    }

    int offsetCelulasSuperiores = Y * (X - 2);
    int offsetCelulasNivel = Y * (X - 1);
    for (int j = 1; j < Y - 1; j++) {

        int posIzquierda = j - 1;
        int posDerecha = j + 1;

        int sumaVecinos = 0;
        sumaVecinos = ptr1[posIzquierda + offsetCelulasSuperiores]     + ptr1[j + offsetCelulasSuperiores] + ptr1[posDerecha + offsetCelulasSuperiores] +
                      ptr1[posIzquierda + offsetCelulasNivel]                                              + ptr1[posDerecha + offsetCelulasNivel] +
                      ptr1[posIzquierda]                               + ptr1[j]                           + ptr1[posDerecha];

        if (ptr1[j + offsetCelulasNivel] == 1) {
            bool encontrado = false;
            for (int k = 0; k < tamanioS; k++) {
                if (sumaVecinos == SC[k]) {
                    encontrado = true;
                    ptr3[j + offsetCelulasNivel] = 1;
                    break;
                }
            }
        }
        else {
            for (int k = 0; k < tamanioB; k++) {
                if (sumaVecinos == BC[k]) {
                    ptr3[j + offsetCelulasNivel] = 1;
                    break;
                }
            }
        }

    }

    for (int i = 1; i < X - 1; i++) {

        int sumaVecinos = 0;
        sumaVecinos = ptr1[Y*i - 1]     + ptr1[Y*(i-1)]     + ptr1[Y*(i-1)+1] +
                      ptr1[Y*(i+1)-1]                       + ptr1[Y*i+1] +
                      ptr1[Y*(i+2)-1]   + ptr1[Y*(i+1)]     + ptr1[Y*(i+1)+1];

        if (ptr1[i*Y] == 1) {
            bool encontrado = false;
            for (int k = 0; k < tamanioS; k++) {
                if (sumaVecinos == SC[k]) {
                    encontrado = true;
                    ptr3[i * Y] = 1;
                    break;
                }
            }
        }
        else {
            for (int k = 0; k < tamanioB; k++) {
                if (sumaVecinos == BC[k]) {
                    ptr3[i * Y] = 1;
                    break;
                }
            }
        }

    }

    for (int i = 1; i < X - 1; i++) {

        int sumaVecinos = 0;
        sumaVecinos = ptr1[Y * i - 2]       + ptr1[Y * i - 1]       + ptr1[Y * (i - 1)] +
                      ptr1[Y * (i + 1)-2]                           + ptr1[Y * i]       +
                      ptr1[Y * (i + 2)-2]   + ptr1[Y * (i + 2)-1]   + ptr1[Y * (i + 1)];

        if (ptr1[Y*(i+1) - 1] == 1) {
            bool encontrado = false;
            for (int k = 0; k < tamanioS; k++) {
                if (sumaVecinos == SC[k]) {
                    encontrado = true;
                    ptr3[Y * (i + 1) - 1] = 1;
                    break;
                }
            }
        }
        else {
            for (int k = 0; k < tamanioB; k++) {
                if (sumaVecinos == BC[k]) {
                    ptr3[Y * (i + 1) - 1] = 1;
                    break;
                }
            }
        }

    }

    //Cálculo de esquinas.
    //Superior Izquierda.
    int sumaVecinos = 0;
    sumaVecinos = ptr1[X*Y-1]       + ptr1[X*(Y-1)] + ptr1[X * (Y - 1)+1] +
                  ptr1[X-1]                         + ptr1[1] +
                  ptr1[X+X+1]       + ptr1[X]       + ptr1[X+1];

    if (ptr1[0] == 1) {
        bool encontrado = false;
        for (int k = 0; k < tamanioS; k++) {
            if (sumaVecinos == SC[k]) {
                encontrado = true;
                ptr3[0] = 1;
                break;
            }
        }
    }
    else {
        for (int k = 0; k < tamanioB; k++) {
            if (sumaVecinos == BC[k]) {
                ptr3[0] = 1;
                break;
            }
        }
    }

    //Superior derecha
    sumaVecinos = 0;
    sumaVecinos = ptr1[X * Y - 2] + ptr1[X*Y-1] + ptr1[X*Y-X] +
        ptr1[X - 2] + ptr1[0] +
        ptr1[X - 2 + X] + ptr1[X-1+X] + ptr1[X];

    if (ptr1[X-1] == 1) {
        bool encontrado = false;
        for (int k = 0; k < tamanioS; k++) {
            if (sumaVecinos == SC[k]) {
                encontrado = true;
                ptr3[X-1] = 1;
                break;
            }
        }
    }
    else {
        for (int k = 0; k < tamanioB; k++) {
            if (sumaVecinos == BC[k]) {
                ptr3[X-1] = 1;
                break;
            }
        }
    }

    //Inferior izquierda.
    sumaVecinos = 0;
    sumaVecinos = ptr1[X*(Y-1)-1] + ptr1[X*(Y-2)] + ptr1[X*(Y-2)+1] +
        ptr1[X*Y-1] + ptr1[X*(Y-1)+1] +
        ptr1[X - 1] + ptr1[0] + ptr1[1];

    if (ptr1[X*(Y - 1)] == 1) {
        bool encontrado = false;
        for (int k = 0; k < tamanioS; k++) {
            if (sumaVecinos == SC[k]) {
                encontrado = true;
                ptr3[X * (Y - 1)] = 1;
                break;
            }
        }
    }
    else {
        for (int k = 0; k < tamanioB; k++) {
            if (sumaVecinos == BC[k]) {
                ptr3[X * (Y - 1)] = 1;
                break;
            }
        }
    }

    //Inferior derecha.
    sumaVecinos = 0;
    sumaVecinos = ptr1[(X*Y)-2-X] + ptr1[(X*Y)-1-X] + ptr1[X*(Y-2)] +
        ptr1[X * Y - 2] + ptr1[X*(Y-1)] +
        ptr1[X - 2] + ptr1[X-1] + ptr1[0];

    if (ptr1[X * Y - 1] == 1) {
        bool encontrado = false;
        for (int k = 0; k < tamanioS; k++) {
            if (sumaVecinos == SC[k]) {
                encontrado = true;
                ptr3[X * Y - 1] = 1;
                break;
            }
        }
    }
    else {
        for (int k = 0; k < tamanioB; k++) {
            if (sumaVecinos == BC[k]) {
                ptr3[X * Y - 1] = 1;
                break;
            }
        }
    }

    //Cálculo de entropía de Shannon.
    float totalCelulas = (float)X * (float)Y;
    float entropia = 0;

    for (int i = 1; i < X - 1; i++) {
        for (int j = 1; j < Y - 1; j++) {
            short valorDecimalEstado = 0;
            valorDecimalEstado = potencia(2, 8) * ptr3[j - 1 + Y * (i - 1)] + potencia(2, 7) * ptr3[j + Y * (i - 1)] + potencia(2, 6) * ptr3[j + 1 + Y * (i - 1)] +
                                 potencia(2, 5) * ptr3[j - 1 + i * Y]       + potencia(2, 4) * ptr3[j + 1 + i * Y]   + potencia(2, 3) * ptr3[j + 1 + i * Y] +
                                 potencia(2, 2) * ptr3[j - 1 + Y * (i + 1)] + potencia(2, 1) * ptr3[j + Y * (i + 1)] + potencia(2, 0) * ptr3[j + 1 + Y * (i + 1)];

            contadorEstados[valorDecimalEstado]++;

        }
    }

    //cout << "Calculos especiales..." << endl;
    offsetCelulasInferiores = Y * (X - 1);
    for (int j = 1; j < Y - 1; j++) {

        int posIzquierda = j - 1;
        int posDerecha = j + 1;

        short valorDecimalEstado = 0;
        valorDecimalEstado = potencia(2, 8) * ptr3[posIzquierda + offsetCelulasInferiores] + potencia(2, 7) * ptr3[j + offsetCelulasInferiores] + potencia(2, 6) * ptr3[posDerecha + offsetCelulasInferiores] +
                             potencia(2, 5) * ptr3[posIzquierda]                           + potencia(2, 4) * ptr3[j]                           + potencia(2, 3) * ptr3[posDerecha] +
                             potencia(2, 2) * ptr3[posIzquierda + Y]                       + potencia(2, 1) * ptr3[j + Y]                       + potencia(2, 0) * ptr3[posDerecha + Y];

        contadorEstados[valorDecimalEstado]++;

    }

    offsetCelulasSuperiores = Y * (X - 2);
    offsetCelulasNivel = Y * (X - 1);
    for (int j = 1; j < Y - 1; j++) {

        int posIzquierda = j - 1;
        int posDerecha = j + 1;

        int sumaVecinos = 0;
        sumaVecinos = potencia(2, 8) * ptr3[posIzquierda + offsetCelulasSuperiores]  + potencia(2, 7) * ptr3[j + offsetCelulasSuperiores] + potencia(2, 6) * ptr3[posDerecha + offsetCelulasSuperiores] +
                      potencia(2, 5) * ptr3[posIzquierda + offsetCelulasNivel]       + potencia(2, 4) * ptr3[j + offsetCelulasNivel]      + potencia(2, 3) * ptr3[posDerecha + offsetCelulasNivel] +
                      potencia(2, 2) * ptr3[posIzquierda]                            + potencia(2, 1) * ptr3[j]                           + potencia(2, 0) * ptr3[posDerecha];

    }

        
    for (int i = 1; i < X - 1; i++) {

        int sumaVecinos = 0;
        sumaVecinos = potencia(2, 8) * ptr3[Y * i - 1]       + potencia(2, 7) * ptr3[Y * (i - 1)] + potencia(2, 6) * ptr3[Y * (i - 1) + 1] +
                      potencia(2, 5) * ptr3[Y * (i + 1) - 1] + potencia(2, 4) * ptr3[i * Y]       + potencia(2, 3) * ptr3[Y * i + 1] +
                      potencia(2, 2) * ptr3[Y * (i + 2) - 1] + potencia(2, 1) * ptr3[Y * (i + 1)] + potencia(2, 0) * ptr3[Y * (i + 1) + 1];


    }

    for (int i = 1; i < X - 1; i++) {

        int sumaVecinos = 0;
        sumaVecinos = potencia(2, 8) * ptr3[Y * i - 2]       + potencia(2, 7) * ptr3[Y * i - 1]         + potencia(2, 6) * ptr3[Y * (i - 1)] +
                      potencia(2, 5) * ptr3[Y * (i + 1) - 2] + potencia(2, 4) * ptr3[Y * (i + 1) - 1]   + potencia(2, 3) * ptr3[Y * i] +
                      potencia(2, 2) * ptr3[Y * (i + 2) - 2] + potencia(2, 1) * ptr3[Y * (i + 2) - 1]   + potencia(2, 0) * ptr3[Y * (i + 1)];
    }

    for (int i = 0; i < 512; i++) {
        //cout << i << " : " << contadorEstados[i] << endl;
        if (contadorEstados[i] > 0) {
            float qn = contadorEstados[i] / totalCelulas;
            entropia += (qn)*log2(qn);
        }
    }

    //cout << "Entropia: " << entropia << endl;


    //reshape array to match input shape
    result.resize({ X,Y });

    py::list resultados;
    resultados.append(result);
    resultados.append(entropia*(-1));
    return resultados;

    //return result, entropia;
}



PYBIND11_MODULE(OptimizacionesC, m) {
    m.doc() = "Optimizaciones para el procesamiento de universos de un automata celular de 2 dimensiones.."; // optional module docstring

    m.def("evaluar", &evaluar, "Evalua un grid de automata celular");
    m.def("generarRelacionesArbol", &generarRelacionesArbol, "Genera las relaciones del arbol de relaciones de estados.");    
}