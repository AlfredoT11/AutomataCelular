#include <iostream>
#include <../../../AutomataCelular/venv/Lib/site-packages/pybind11/include/pybind11/pybind11.h>
#include <../../../AutomataCelular/venv/Lib/site-packages/pybind11/include/pybind11/numpy.h>


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

//py::array_t<bool> evaluar(py::array_t<bool> input1, int s_min, int s_max, int r_min, int r_max) {
py::array_t<bool> evaluar(py::array_t<bool> input1, py::list B, py::list S) {
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
        }

    }


    // reshape array to match input shape
    result.resize({ X,Y });

    return result;
}



PYBIND11_MODULE(OptimizacionesC, m) {
    m.doc() = "Optimizaciones para el procesamiento de universos de un automata celular de 2 dimensiones.."; // optional module docstring

    m.def("evaluar", &evaluar, "Evalua un grid de automata celular");
    m.def("generarRelacionesArbol", &generarRelacionesArbol, "Genera las relaciones del arbol de relaciones de estados.");
}