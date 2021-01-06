#include <iostream>
#include <../../../AutomataCelular/venv/Lib/site-packages/pybind11/include/pybind11/pybind11.h>
#include <../../../AutomataCelular/venv/Lib/site-packages/pybind11/include/pybind11/numpy.h>


namespace py = pybind11;


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
    m.doc() = "Evalua un grid de automata celular."; // optional module docstring

    m.def("evaluar", &evaluar, "Evalua un grid de automata celular");
}