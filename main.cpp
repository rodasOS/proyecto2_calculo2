// =============================================================================
//  CALCULO 2 - PROYECTO 2
//
//  Modulos del programa:
//    1) Visualizador de puntos en el espacio 3D (hasta 10 puntos)
//         - Si raylib esta disponible: ventana 3D con tema oscuro.
//         - Si no: lienzo de texto en la consola (solo biblioteca estandar).
//    2) Generador del Triangulo Astromash.
//
//  Todo el codigo vive en este unico archivo.
//
//  Compilacion:
//    Consola:              g++ main.cpp -o proyecto2
//    Ventana 3D (Linux):   g++ main.cpp -o proyecto2 -lraylib -lGL -lm -lpthread -ldl
//    Ventana 3D (Windows): g++ main.cpp -o proyecto2.exe -static -lraylib -lopengl32 -lgdi32 -lwinmm
//    Forzar consola aunque raylib este instalada: agregar -DSIN_VENTANA
//
//  Restricciones del enunciado: no se usan vectores, matrices (arreglos),
//  recursividad ni memoria dinamica (pilas y colas). Los datos que normalmente
//  vivirian en un arreglo se guardan en archivos de texto temporales y se
//  recorren de forma secuencial.
// =============================================================================

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#if defined(__has_include)
#if __has_include("raylib.h") && !defined(SIN_VENTANA)
#define MODO_VENTANA_RAYLIB
#endif
#endif

#ifdef MODO_VENTANA_RAYLIB
#include "raylib.h"
#elif defined(_WIN32)
#include <windows.h>
#endif

using namespace std;

// =============================================================================
//  Lectura de datos del usuario
// =============================================================================

static void descartarEntradaInvalida() {
    cin.clear();
    cin.ignore(10000, '\n');
}

static int leerEnteroEnRango(const char *mensaje, int valorMinimo, int valorMaximo) {
    int valorLeido;
    cout << mensaje;
    while (!(cin >> valorLeido) || valorLeido < valorMinimo || valorLeido > valorMaximo) {
        descartarEntradaInvalida();
        cout << "Valor fuera de rango, escribe un numero entre " << valorMinimo
             << " y " << valorMaximo << ": ";
    }
    return valorLeido;
}

// =============================================================================
//  Modulo 2: Triangulo Astromash
//
//    Filas iniciales:  1  /  2 2  /  5 4 5
//    valorBorde(n)   = suma(fila n-1) + primer valor de la fila n-2
//    T(n, k)         = T(n-1, k-1) + T(n-1, k) + T(n-2, k-1)     con 0 < k < n-1
//
//  Se usan tres archivos como almacenamiento secuencial (fila n-1, fila n-2 y
//  fila nueva). En memoria solo existen variables escalares.
// =============================================================================

static const int MAXIMO_FILAS_TRIANGULO = 20;  // con mas filas se desborda long long
static const int ANCHO_COLUMNA_TRIANGULO = 14;
static const char *ARCHIVO_FILA_ANTERIOR = "triangulo_fila_anterior.tmp";
static const char *ARCHIVO_FILA_ANTEPENULTIMA = "triangulo_fila_antepenultima.tmp";
static const char *ARCHIVO_FILA_ACTUAL = "triangulo_fila_actual.tmp";

static void imprimirMargenIzquierdo(int numeroFila, int totalFilas) {
    cout << string((totalFilas - numeroFila) * ANCHO_COLUMNA_TRIANGULO / 2, ' ');
}

// Filas 1 a 3: se escriben directamente.
static void generarFilaInicial(int numeroFila, int totalFilas) {
    ofstream archivoFilaActual(ARCHIVO_FILA_ACTUAL);
    imprimirMargenIzquierdo(numeroFila, totalFilas);
    for (int posicion = 0; posicion < numeroFila; posicion++) {
        long long valorCalculado;
        if (numeroFila == 1) valorCalculado = 1;
        else if (numeroFila == 2) valorCalculado = 2;
        else valorCalculado = (posicion == 1) ? 4 : 5;
        archivoFilaActual << valorCalculado << ' ';
        cout << setw(ANCHO_COLUMNA_TRIANGULO) << valorCalculado;
    }
    cout << '\n';
}

// Filas 4 en adelante: se calculan leyendo las dos filas previas en paralelo.
static void generarFilaPorRecurrencia(int numeroFila, int totalFilas) {
    long long sumaFilaAnterior = 0, valorLeido;
    ifstream pasadaDeSuma(ARCHIVO_FILA_ANTERIOR);
    while (pasadaDeSuma >> valorLeido) sumaFilaAnterior += valorLeido;
    pasadaDeSuma.close();

    ifstream lectorFilaAnterior(ARCHIVO_FILA_ANTERIOR);
    ifstream lectorFilaAntepenultima(ARCHIVO_FILA_ANTEPENULTIMA);
    long long primerValorAntepenultima = 0;
    long long valorIzquierdoAnterior = 0, valorDerechoAnterior = 0, valorDiagonal = 0;
    lectorFilaAntepenultima >> primerValorAntepenultima;
    lectorFilaAnterior >> valorIzquierdoAnterior;
    long long valorBorde = sumaFilaAnterior + primerValorAntepenultima;

    ofstream archivoFilaActual(ARCHIVO_FILA_ACTUAL);
    imprimirMargenIzquierdo(numeroFila, totalFilas);
    archivoFilaActual << valorBorde << ' ';
    cout << setw(ANCHO_COLUMNA_TRIANGULO) << valorBorde;
    for (int posicion = 1; posicion <= numeroFila - 2; posicion++) {
        lectorFilaAnterior >> valorDerechoAnterior;
        if (posicion == 1) valorDiagonal = primerValorAntepenultima;
        else lectorFilaAntepenultima >> valorDiagonal;
        long long valorCalculado = valorIzquierdoAnterior + valorDerechoAnterior + valorDiagonal;
        archivoFilaActual << valorCalculado << ' ';
        cout << setw(ANCHO_COLUMNA_TRIANGULO) << valorCalculado;
        valorIzquierdoAnterior = valorDerechoAnterior;
    }
    archivoFilaActual << valorBorde << ' ';
    cout << setw(ANCHO_COLUMNA_TRIANGULO) << valorBorde << '\n';
}

static void ejecutarTrianguloAstromash() {
    int totalFilas = leerEnteroEnRango("Cuantas filas del triangulo quieres generar? (1-20): ",
                                       1, MAXIMO_FILAS_TRIANGULO);
    cout << "\n  TRIANGULO ASTROMASH - " << totalFilas
         << (totalFilas == 1 ? " fila" : " filas") << "\n\n";
    for (int numeroFila = 1; numeroFila <= totalFilas; numeroFila++) {
        if (numeroFila <= 3) generarFilaInicial(numeroFila, totalFilas);
        else generarFilaPorRecurrencia(numeroFila, totalFilas);
        // Rotacion de archivos: actual -> anterior -> antepenultima
        remove(ARCHIVO_FILA_ANTEPENULTIMA);
        rename(ARCHIVO_FILA_ANTERIOR, ARCHIVO_FILA_ANTEPENULTIMA);
        rename(ARCHIVO_FILA_ACTUAL, ARCHIVO_FILA_ANTERIOR);
    }
    remove(ARCHIVO_FILA_ANTERIOR);
    remove(ARCHIVO_FILA_ANTEPENULTIMA);
}

// =============================================================================
//  Modulo 1: visualizador de puntos 3D
//
//  Los puntos capturados se guardan en un archivo temporal (una linea por
//  punto: x y z) y se leen de forma secuencial cada vez que se necesitan.
// =============================================================================

static const int MAXIMO_PUNTOS = 10;
static const char *ARCHIVO_PUNTOS_TEMPORAL = "graficador_puntos.tmp";

#ifndef MODO_VENTANA_RAYLIB
// -----------------------------------------------------------------------------
//  Variante de consola (se usa cuando raylib no esta disponible)
//
//  Proyeccion: el espacio se gira `anguloGiro` alrededor del eje Z y se inclina
//  `anguloInclinacion` hacia el observador. Cada celda del lienzo se decide
//  preguntando que hay en ella (punto, linea o eje), asi no hace falta una
//  matriz que represente la pantalla.
// -----------------------------------------------------------------------------

static const int ANCHO_LIENZO = 74;
static const int ALTO_LIENZO = 30;
static const double PI_VALOR = 3.14159265358979323846;

struct ConfiguracionVistaConsola {
    double anguloGiro;         // radianes
    double anguloInclinacion;  // radianes
    double escala;
    double origenColumna;
    double origenFila;
    double longitudEje;        // los ejes van de -longitudEje a +longitudEje
    bool usarColoresAnsi;
    bool cerrarFigura;
};

// Colores ANSI: P anaranjado, Q azul, R verde (y se repiten).
static const char *colorAnsiDelPunto(int indicePunto) {
    switch (indicePunto % 3) {
        case 0: return "\033[1;38;5;208m";
        case 1: return "\033[1;34m";
        default: return "\033[1;32m";
    }
}

static const char *nombreDelColor(int indicePunto) {
    switch (indicePunto % 3) {
        case 0: return "anaranjado";
        case 1: return "azul";
        default: return "verde";
    }
}

static const char *COLOR_ANSI_EJES = "\033[90m";
static const char *COLOR_ANSI_REINICIO = "\033[0m";

static void proyectarAPlanoPantalla(const ConfiguracionVistaConsola &vista,
                                    double x, double y, double z,
                                    double &horizontal, double &vertical) {
    double xGirado = x * cos(vista.anguloGiro) - y * sin(vista.anguloGiro);
    double profundidad = x * sin(vista.anguloGiro) + y * cos(vista.anguloGiro);
    horizontal = xGirado;
    vertical = z * cos(vista.anguloInclinacion) + profundidad * sin(vista.anguloInclinacion);
}

static void convertirACeldaPantalla(const ConfiguracionVistaConsola &vista,
                                    double x, double y, double z,
                                    int &columna, int &fila) {
    double horizontal, vertical;
    proyectarAPlanoPantalla(vista, x, y, z, horizontal, vertical);
    columna = (int)floor(vista.origenColumna + horizontal * vista.escala + 0.5);
    fila = (int)floor(vista.origenFila - vertical * vista.escala / 2.0 + 0.5);
}

static bool puntoOcupaCelda(const ConfiguracionVistaConsola &vista,
                            double x, double y, double z,
                            int columnaBuscada, int filaBuscada) {
    int columna, fila;
    convertirACeldaPantalla(vista, x, y, z, columna, fila);
    return columna == columnaBuscada && fila == filaBuscada;
}

// Recorre el segmento A-B por muestreo fino y revisa si pasa por la celda.
static bool segmentoCruzaCelda(const ConfiguracionVistaConsola &vista,
                               double xInicio, double yInicio, double zInicio,
                               double xFin, double yFin, double zFin,
                               int columnaBuscada, int filaBuscada) {
    int cantidadMuestras = (int)(vista.escala * 2.0 *
                                 (fabs(xFin - xInicio) + fabs(yFin - yInicio) + fabs(zFin - zInicio))) + 2;
    for (int muestra = 0; muestra <= cantidadMuestras; muestra++) {
        double avance = (double)muestra / cantidadMuestras;
        if (puntoOcupaCelda(vista,
                            xInicio + (xFin - xInicio) * avance,
                            yInicio + (yFin - yInicio) * avance,
                            zInicio + (zFin - zInicio) * avance,
                            columnaBuscada, filaBuscada))
            return true;
    }
    return false;
}

static void ampliarLimitesProyeccion(const ConfiguracionVistaConsola &vista,
                                     double x, double y, double z,
                                     double &minimoHorizontal, double &maximoHorizontal,
                                     double &minimoVertical, double &maximoVertical) {
    double horizontal, vertical;
    proyectarAPlanoPantalla(vista, x, y, z, horizontal, vertical);
    if (horizontal < minimoHorizontal) minimoHorizontal = horizontal;
    if (horizontal > maximoHorizontal) maximoHorizontal = horizontal;
    if (vertical < minimoVertical) minimoVertical = vertical;
    if (vertical > maximoVertical) maximoVertical = vertical;
}

// Calcula escala y origen para que todo el dibujo quepa en el lienzo.
static void ajustarEscalaYOrigen(ConfiguracionVistaConsola &vista) {
    double extremoEje = vista.longitudEje * 1.15;
    double minimoHorizontal = 0, maximoHorizontal = 0, minimoVertical = 0, maximoVertical = 0;
    ampliarLimitesProyeccion(vista, extremoEje, 0, 0, minimoHorizontal, maximoHorizontal, minimoVertical, maximoVertical);
    ampliarLimitesProyeccion(vista, -extremoEje, 0, 0, minimoHorizontal, maximoHorizontal, minimoVertical, maximoVertical);
    ampliarLimitesProyeccion(vista, 0, extremoEje, 0, minimoHorizontal, maximoHorizontal, minimoVertical, maximoVertical);
    ampliarLimitesProyeccion(vista, 0, -extremoEje, 0, minimoHorizontal, maximoHorizontal, minimoVertical, maximoVertical);
    ampliarLimitesProyeccion(vista, 0, 0, extremoEje, minimoHorizontal, maximoHorizontal, minimoVertical, maximoVertical);
    ampliarLimitesProyeccion(vista, 0, 0, -extremoEje, minimoHorizontal, maximoHorizontal, minimoVertical, maximoVertical);
    ifstream archivoPuntos(ARCHIVO_PUNTOS_TEMPORAL);
    double x, y, z;
    while (archivoPuntos >> x >> y >> z)
        ampliarLimitesProyeccion(vista, x, y, z, minimoHorizontal, maximoHorizontal, minimoVertical, maximoVertical);

    double escalaPorAncho = (ANCHO_LIENZO - 1) / (maximoHorizontal - minimoHorizontal);
    double escalaPorAlto = (ALTO_LIENZO - 1) * 2.0 / (maximoVertical - minimoVertical);
    vista.escala = escalaPorAncho < escalaPorAlto ? escalaPorAncho : escalaPorAlto;
    vista.origenColumna = -minimoHorizontal * vista.escala +
                          ((ANCHO_LIENZO - 1) - (maximoHorizontal - minimoHorizontal) * vista.escala) / 2.0;
    vista.origenFila = maximoVertical * vista.escala / 2.0 +
                       ((ALTO_LIENZO - 1) - (maximoVertical - minimoVertical) * vista.escala / 2.0) / 2.0;
}

static void renderizarLienzoConsola(const ConfiguracionVistaConsola &vista) {
    double longitud = vista.longitudEje;
    double posicionEtiquetaEje = vista.longitudEje * 1.12;
    for (int filaPantalla = 0; filaPantalla < ALTO_LIENZO; filaPantalla++) {
        for (int columnaPantalla = 0; columnaPantalla < ANCHO_LIENZO; columnaPantalla++) {
            char caracterCelda = ' ';
            const char *colorCelda = nullptr;
            double x, y, z;
            double xAnterior = 0, yAnterior = 0, zAnterior = 0;
            double xPrimero = 0, yPrimero = 0, zPrimero = 0;
            int indicePunto;

            // Capa 1: puntos (se muestran con su letra: P, Q, R, ...)
            ifstream archivoPuntos(ARCHIVO_PUNTOS_TEMPORAL);
            indicePunto = 0;
            while (caracterCelda == ' ' && archivoPuntos >> x >> y >> z) {
                if (puntoOcupaCelda(vista, x, y, z, columnaPantalla, filaPantalla)) {
                    caracterCelda = (char)('P' + indicePunto);
                    colorCelda = colorAnsiDelPunto(indicePunto);
                }
                indicePunto++;
            }
            archivoPuntos.close();

            // Capa 2: nombres de los ejes
            if (caracterCelda == ' ') {
                if (puntoOcupaCelda(vista, posicionEtiquetaEje, 0, 0, columnaPantalla, filaPantalla)) {
                    caracterCelda = 'X';
                    colorCelda = COLOR_ANSI_EJES;
                } else if (puntoOcupaCelda(vista, 0, posicionEtiquetaEje, 0, columnaPantalla, filaPantalla)) {
                    caracterCelda = 'Y';
                    colorCelda = COLOR_ANSI_EJES;
                } else if (puntoOcupaCelda(vista, 0, 0, posicionEtiquetaEje, columnaPantalla, filaPantalla)) {
                    caracterCelda = 'Z';
                    colorCelda = COLOR_ANSI_EJES;
                }
            }

            // Capa 3: lineas que unen los puntos
            if (caracterCelda == ' ') {
                ifstream archivoSegmentos(ARCHIVO_PUNTOS_TEMPORAL);
                indicePunto = 0;
                while (caracterCelda == ' ' && archivoSegmentos >> x >> y >> z) {
                    if (indicePunto == 0) {
                        xPrimero = x;
                        yPrimero = y;
                        zPrimero = z;
                    } else if (segmentoCruzaCelda(vista, xAnterior, yAnterior, zAnterior,
                                                  x, y, z, columnaPantalla, filaPantalla)) {
                        caracterCelda = '+';
                    }
                    xAnterior = x;
                    yAnterior = y;
                    zAnterior = z;
                    indicePunto++;
                }
                if (caracterCelda == ' ' && vista.cerrarFigura && indicePunto > 2 &&
                    segmentoCruzaCelda(vista, xAnterior, yAnterior, zAnterior,
                                       xPrimero, yPrimero, zPrimero, columnaPantalla, filaPantalla))
                    caracterCelda = '+';
            }

            // Capa 4: ejes coordenados
            if (caracterCelda == ' ') {
                if (segmentoCruzaCelda(vista, -longitud, 0, 0, longitud, 0, 0, columnaPantalla, filaPantalla))
                    caracterCelda = '-';
                else if (segmentoCruzaCelda(vista, 0, -longitud, 0, 0, longitud, 0, columnaPantalla, filaPantalla))
                    caracterCelda = '/';
                else if (segmentoCruzaCelda(vista, 0, 0, -longitud, 0, 0, longitud, columnaPantalla, filaPantalla))
                    caracterCelda = '|';
                if (caracterCelda != ' ') colorCelda = COLOR_ANSI_EJES;
            }

            if (vista.usarColoresAnsi && colorCelda != nullptr)
                cout << colorCelda << caracterCelda << COLOR_ANSI_REINICIO;
            else
                cout << caracterCelda;
        }
        cout << '\n';
    }
}

static void imprimirLeyendaPuntos(const ConfiguracionVistaConsola &vista) {
    ifstream archivoPuntos(ARCHIVO_PUNTOS_TEMPORAL);
    double x, y, z;
    int indicePunto = 0;
    cout << "Ejes: X = '-'   Y = '/'   Z = '|'   (rango de -" << vista.longitudEje
         << " a " << vista.longitudEje << ")\n";
    cout << "Puntos graficados:  ";
    while (archivoPuntos >> x >> y >> z) {
        if (vista.usarColoresAnsi) cout << colorAnsiDelPunto(indicePunto);
        cout << (char)('P' + indicePunto) << "(" << x << ", " << y << ", " << z << ")";
        if (vista.usarColoresAnsi) cout << COLOR_ANSI_REINICIO;
        else cout << " [" << nombreDelColor(indicePunto) << "]";
        cout << "   ";
        indicePunto++;
    }
    cout << '\n';
}

static void mostrarGraficaEnConsola(double longitudEje, bool cerrarFigura) {
    ConfiguracionVistaConsola vista;
    vista.cerrarFigura = cerrarFigura;
    vista.longitudEje = longitudEje;
    char respuestaColores;
    cout << "Tu terminal soporta colores? (s/n): ";
    cin >> respuestaColores;
    vista.usarColoresAnsi = (respuestaColores == 's' || respuestaColores == 'S');
    vista.anguloGiro = PI_VALOR / 4;          // 45 grados
    vista.anguloInclinacion = PI_VALOR / 6;   // 30 grados

    char teclaPresionada = ' ';
    while (teclaPresionada != 'q' && teclaPresionada != 'Q') {
        ajustarEscalaYOrigen(vista);
        cout << '\n';
        renderizarLienzoConsola(vista);
        imprimirLeyendaPuntos(vista);
        cout << "Controles -> a/d: rotar | w/s: inclinar | q: regresar al menu: ";
        cin >> teclaPresionada;
        if (teclaPresionada == 'a' || teclaPresionada == 'A') vista.anguloGiro -= PI_VALOR / 12;
        else if (teclaPresionada == 'd' || teclaPresionada == 'D') vista.anguloGiro += PI_VALOR / 12;
        else if (teclaPresionada == 'w' || teclaPresionada == 'W') vista.anguloInclinacion += PI_VALOR / 18;
        else if (teclaPresionada == 's' || teclaPresionada == 'S') vista.anguloInclinacion -= PI_VALOR / 18;
        if (vista.anguloInclinacion > 1.4) vista.anguloInclinacion = 1.4;
        if (vista.anguloInclinacion < -1.4) vista.anguloInclinacion = -1.4;
    }
}
#endif  // !MODO_VENTANA_RAYLIB

#ifdef MODO_VENTANA_RAYLIB
// -----------------------------------------------------------------------------
//  Variante con ventana 3D (raylib), tema oscuro
// -----------------------------------------------------------------------------

static const int ANCHO_VENTANA = 1000;
static const int ALTO_VENTANA = 700;
static const int CUADROS_POR_SEGUNDO = 60;
static const float SENSIBILIDAD_MOUSE = 0.005f;
static const float VELOCIDAD_ROTACION_TECLADO = 0.02f;
static const float FACTOR_ZOOM = 0.3f;
static const float LIMITE_INCLINACION = 1.5f;
static const float DISTANCIA_MINIMA_CAMARA = 2.0f;

static const Color COLOR_FONDO = {18, 22, 34, 255};
static const Color COLOR_CUADRICULA = {50, 58, 80, 255};
static const Color COLOR_EJE_X = {255, 95, 95, 255};
static const Color COLOR_EJE_Y = {110, 235, 170, 255};
static const Color COLOR_EJE_Z = {120, 175, 255, 255};
static const Color COLOR_MARCAS = {150, 160, 185, 255};
static const Color COLOR_LINEAS = {200, 205, 215, 255};
static const Color COLOR_PUNTO_P = {255, 161, 0, 255};   // anaranjado
static const Color COLOR_PUNTO_Q = {40, 120, 255, 255};  // azul
static const Color COLOR_PUNTO_R = {70, 220, 110, 255};  // verde
static const Color COLOR_PANEL = {10, 13, 22, 255};
static const Color COLOR_BORDE_PANEL = {90, 105, 150, 255};
static const Color COLOR_TEXTO = {230, 235, 245, 255};
static const Color COLOR_TEXTO_AYUDA = {170, 180, 205, 255};

// Convencion matematica (z hacia arriba) -> raylib (y hacia arriba).
static Vector3 convertirACoordenadasRaylib(double x, double y, double z) {
    Vector3 posicion;
    posicion.x = (float)x;
    posicion.y = (float)z;
    posicion.z = (float)(-y);
    return posicion;
}

static Color colorSegunIndicePunto(int indicePunto) {
    switch (indicePunto % 3) {
        case 0: return COLOR_PUNTO_P;
        case 1: return COLOR_PUNTO_Q;
        default: return COLOR_PUNTO_R;
    }
}

static void dibujarEtiquetaEnPantalla(const Camera3D &camara, Vector3 posicionMundo,
                                      const char *texto, int tamanoFuente, Color color) {
    Vector2 posicionPantalla = GetWorldToScreen(posicionMundo, camara);
    DrawText(texto, (int)posicionPantalla.x + 8, (int)posicionPantalla.y - 8, tamanoFuente, color);
}

// Segmento con grosor (DrawLine3D siempre mide 1 pixel).
static void dibujarSegmentoGrueso(double xInicio, double yInicio, double zInicio,
                                  double xFin, double yFin, double zFin,
                                  float grosor, Color color) {
    DrawCylinderEx(convertirACoordenadasRaylib(xInicio, yInicio, zInicio),
                   convertirACoordenadasRaylib(xFin, yFin, zFin), grosor, grosor, 8, color);
}

// Cuadricula del plano XY, de -longitud a +longitud con paso 1.
static void dibujarCuadricula(int longitud) {
    for (int posicion = -longitud; posicion <= longitud; posicion++) {
        DrawLine3D(convertirACoordenadasRaylib(posicion, -longitud, 0),
                   convertirACoordenadasRaylib(posicion, longitud, 0), COLOR_CUADRICULA);
        DrawLine3D(convertirACoordenadasRaylib(-longitud, posicion, 0),
                   convertirACoordenadasRaylib(longitud, posicion, 0), COLOR_CUADRICULA);
    }
}

static void dibujarPanelDePuntos(int cantidadPuntos) {
    int altoPanel = 56 + cantidadPuntos * 26;
    DrawRectangle(12, 12, 270, altoPanel, Fade(COLOR_PANEL, 0.78f));
    DrawRectangleLines(12, 12, 270, altoPanel, COLOR_BORDE_PANEL);
    DrawText("Puntos graficados", 26, 22, 20, COLOR_TEXTO);
    DrawLine(26, 48, 268, 48, COLOR_BORDE_PANEL);

    ifstream archivoPuntos(ARCHIVO_PUNTOS_TEMPORAL);
    double x, y, z;
    int indicePunto = 0;
    while (archivoPuntos >> x >> y >> z) {
        DrawText(TextFormat("%c(%g, %g, %g)", 'P' + indicePunto, x, y, z),
                 26, 58 + indicePunto * 26, 20, colorSegunIndicePunto(indicePunto));
        indicePunto++;
    }
}

static void dibujarBarraDeAyuda() {
    int barraY = GetScreenHeight() - 34;
    DrawRectangle(0, barraY, GetScreenWidth(), 34, Fade(COLOR_PANEL, 0.85f));
    DrawText("Arrastra con el mouse o usa las flechas para rotar  |  Rueda: acercar o alejar  |  ESC: regresar al menu",
             14, barraY + 9, 16, COLOR_TEXTO_AYUDA);
}

static void mostrarGraficaEnVentana(double longitudEje, bool cerrarFigura, int cantidadPuntos) {
    float longitud = (float)longitudEje;
    float radioPunto = longitud * 0.03f < 0.12f ? 0.12f : longitud * 0.03f;
    float grosorLinea = radioPunto * 0.25f;
    int pasoEntreMarcas = (int)ceil(longitudEje / 10.0);

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(ANCHO_VENTANA, ALTO_VENTANA, "Calculo 2 - Visualizador de puntos 3D");
    SetTargetFPS(CUADROS_POR_SEGUNDO);

    float anguloGiro = 0.8f, anguloInclinacion = 0.5f;
    float distanciaCamara = longitud * 3.2f + 4.0f;
    Camera3D camara;
    camara.target = convertirACoordenadasRaylib(0, 0, 0);
    camara.up.x = 0.0f;
    camara.up.y = 1.0f;
    camara.up.z = 0.0f;
    camara.fovy = 45.0f;
    camara.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        // Entrada del usuario
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 desplazamiento = GetMouseDelta();
            anguloGiro -= desplazamiento.x * SENSIBILIDAD_MOUSE;
            anguloInclinacion += desplazamiento.y * SENSIBILIDAD_MOUSE;
        }
        if (IsKeyDown(KEY_LEFT)) anguloGiro -= VELOCIDAD_ROTACION_TECLADO;
        if (IsKeyDown(KEY_RIGHT)) anguloGiro += VELOCIDAD_ROTACION_TECLADO;
        if (IsKeyDown(KEY_UP)) anguloInclinacion += VELOCIDAD_ROTACION_TECLADO;
        if (IsKeyDown(KEY_DOWN)) anguloInclinacion -= VELOCIDAD_ROTACION_TECLADO;
        distanciaCamara -= GetMouseWheelMove() * longitud * FACTOR_ZOOM;
        if (anguloInclinacion > LIMITE_INCLINACION) anguloInclinacion = LIMITE_INCLINACION;
        if (anguloInclinacion < -LIMITE_INCLINACION) anguloInclinacion = -LIMITE_INCLINACION;
        if (distanciaCamara < DISTANCIA_MINIMA_CAMARA) distanciaCamara = DISTANCIA_MINIMA_CAMARA;
        camara.position.x = distanciaCamara * cosf(anguloInclinacion) * sinf(anguloGiro);
        camara.position.y = distanciaCamara * sinf(anguloInclinacion);
        camara.position.z = distanciaCamara * cosf(anguloInclinacion) * cosf(anguloGiro);

        BeginDrawing();
        ClearBackground(COLOR_FONDO);
        BeginMode3D(camara);

        dibujarCuadricula((int)longitud);
        dibujarSegmentoGrueso(-longitudEje, 0, 0, longitudEje, 0, 0, grosorLinea, COLOR_EJE_X);
        dibujarSegmentoGrueso(0, -longitudEje, 0, 0, longitudEje, 0, grosorLinea, COLOR_EJE_Y);
        dibujarSegmentoGrueso(0, 0, -longitudEje, 0, 0, longitudEje, grosorLinea, COLOR_EJE_Z);

        // Lectura secuencial: solo se guardan el punto anterior y el primero.
        ifstream archivoPuntos(ARCHIVO_PUNTOS_TEMPORAL);
        double x, y, z;
        double xAnterior = 0, yAnterior = 0, zAnterior = 0;
        double xPrimero = 0, yPrimero = 0, zPrimero = 0;
        int indicePunto = 0;
        while (archivoPuntos >> x >> y >> z) {
            if (indicePunto == 0) {
                xPrimero = x;
                yPrimero = y;
                zPrimero = z;
            } else {
                dibujarSegmentoGrueso(xAnterior, yAnterior, zAnterior, x, y, z, grosorLinea, COLOR_LINEAS);
            }
            Vector3 posicionPunto = convertirACoordenadasRaylib(x, y, z);
            DrawSphere(posicionPunto, radioPunto, colorSegunIndicePunto(indicePunto));
            DrawSphereWires(posicionPunto, radioPunto * 1.7f, 8, 8,
                            Fade(colorSegunIndicePunto(indicePunto), 0.45f));
            xAnterior = x;
            yAnterior = y;
            zAnterior = z;
            indicePunto++;
        }
        if (cerrarFigura && indicePunto > 2)
            dibujarSegmentoGrueso(xAnterior, yAnterior, zAnterior, xPrimero, yPrimero, zPrimero,
                                  grosorLinea, COLOR_LINEAS);
        EndMode3D();

        // Nombres de los ejes y numeros de referencia
        dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(longitud * 1.05f, 0, 0), "X", 24, COLOR_EJE_X);
        dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(0, longitud * 1.05f, 0), "Y", 24, COLOR_EJE_Y);
        dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(0, 0, longitud * 1.05f), "Z", 24, COLOR_EJE_Z);
        for (int marca = pasoEntreMarcas; marca <= (int)longitudEje; marca += pasoEntreMarcas) {
            dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(marca, 0, 0), TextFormat("%d", marca), 14, COLOR_MARCAS);
            dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(-marca, 0, 0), TextFormat("%d", -marca), 14, COLOR_MARCAS);
            dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(0, marca, 0), TextFormat("%d", marca), 14, COLOR_MARCAS);
            dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(0, -marca, 0), TextFormat("%d", -marca), 14, COLOR_MARCAS);
            dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(0, 0, marca), TextFormat("%d", marca), 14, COLOR_MARCAS);
            dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(0, 0, -marca), TextFormat("%d", -marca), 14, COLOR_MARCAS);
        }

        // Nombre y coordenadas junto a cada punto
        ifstream archivoEtiquetas(ARCHIVO_PUNTOS_TEMPORAL);
        indicePunto = 0;
        while (archivoEtiquetas >> x >> y >> z) {
            dibujarEtiquetaEnPantalla(camara, convertirACoordenadasRaylib(x, y, z),
                                      TextFormat("%c(%g, %g, %g)", 'P' + indicePunto, x, y, z),
                                      18, colorSegunIndicePunto(indicePunto));
            indicePunto++;
        }

        dibujarPanelDePuntos(cantidadPuntos);
        dibujarBarraDeAyuda();
        EndDrawing();
    }
    CloseWindow();
}
#endif  // MODO_VENTANA_RAYLIB

// -----------------------------------------------------------------------------
//  Captura de puntos y flujo principal del modulo 1
// -----------------------------------------------------------------------------

static bool capturarPuntosDelUsuario(int cantidadPuntos) {
    ofstream archivoPuntos(ARCHIVO_PUNTOS_TEMPORAL);
    if (!archivoPuntos) {
        cout << "No se pudo crear el archivo temporal (revisa los permisos de la carpeta).\n";
        return false;
    }
    for (int indicePunto = 0; indicePunto < cantidadPuntos; indicePunto++) {
        double x, y, z;
        cout << "Coordenadas del punto " << (char)('P' + indicePunto) << " (x y z): ";
        while (!(cin >> x >> y >> z)) {
            descartarEntradaInvalida();
            cout << "Formato no valido, escribe tres numeros separados por espacios: ";
        }
        archivoPuntos << x << ' ' << y << ' ' << z << '\n';
    }
    return true;
}

static double obtenerMayorCoordenadaAbsoluta() {
    ifstream archivoPuntos(ARCHIVO_PUNTOS_TEMPORAL);
    double x, y, z, mayorValor = 0;
    while (archivoPuntos >> x >> y >> z) {
        if (fabs(x) > mayorValor) mayorValor = fabs(x);
        if (fabs(y) > mayorValor) mayorValor = fabs(y);
        if (fabs(z) > mayorValor) mayorValor = fabs(z);
    }
    return mayorValor;
}

static void ejecutarGraficadorPuntos3D() {
    int cantidadPuntos = leerEnteroEnRango("Cuantos puntos vas a ingresar? (1-10): ", 1, MAXIMO_PUNTOS);
    if (!capturarPuntosDelUsuario(cantidadPuntos)) return;

    bool cerrarFigura = false;
    if (cantidadPuntos > 2) {
        char respuestaCierre;
        cout << "Quieres unir el ultimo punto con el primero? (s/n): ";
        cin >> respuestaCierre;
        cerrarFigura = (respuestaCierre == 's' || respuestaCierre == 'S');
    }

    double longitudEje = ceil(obtenerMayorCoordenadaAbsoluta());
    if (longitudEje < 3) longitudEje = 3;

#ifdef MODO_VENTANA_RAYLIB
    cout << "Abriendo la ventana 3D... (ESC para regresar al menu)\n";
    mostrarGraficaEnVentana(longitudEje, cerrarFigura, cantidadPuntos);
#else
    mostrarGraficaEnConsola(longitudEje, cerrarFigura);
#endif
    remove(ARCHIVO_PUNTOS_TEMPORAL);
}

// =============================================================================
//  Menu principal
// =============================================================================

static void imprimirMenuPrincipal() {
    cout << "\n"
         << "  +--------------------------------------------+\n"
         << "  |        CALCULO 2  -  PROYECTO 2            |\n"
         << "  +--------------------------------------------+\n"
         << "  |  [1] Visualizar puntos en el espacio 3D    |\n"
         << "  |  [2] Generar el Triangulo Astromash        |\n"
         << "  |  [3] Terminar el programa                  |\n"
         << "  +--------------------------------------------+\n";
}

int main() {
#if defined(_WIN32) && !defined(MODO_VENTANA_RAYLIB)
    // Activa los colores ANSI en consolas modernas de Windows.
    HANDLE manejadorConsola = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modoConsola;
    if (GetConsoleMode(manejadorConsola, &modoConsola))
        SetConsoleMode(manejadorConsola, modoConsola | 0x0004);
#endif

    int opcionElegida = 0;
    while (opcionElegida != 3) {
        imprimirMenuPrincipal();
        opcionElegida = leerEnteroEnRango("  Elige una opcion (1-3): ", 1, 3);
        if (opcionElegida == 1) ejecutarGraficadorPuntos3D();
        else if (opcionElegida == 2) ejecutarTrianguloAstromash();
    }
    return 0;
}
