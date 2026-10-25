// PROYECTO #2 - Calculo 2
//   1) Grafica de hasta 10 puntos en 3D
//        - con raylib instalada: ventana 3D (se activa sola si encuentra raylib.h)
//        - sin raylib: grafica en la consola (solo biblioteca estandar)
//   2) Triangulo Astromash
//
// Un solo archivo.
//   Sin ventana (consola):  g++ main.cpp -o proyecto2
//   Con ventana 3D:         g++ main.cpp -o proyecto2 -lraylib [-lGL -lm -lpthread -ldl]
//                           (en Windows con raylib/w64devkit: -lraylib -lopengl32 -lgdi32 -lwinmm)
//   Para forzar la consola aunque raylib este instalada: agregar -DSIN_VENTANA
//
// Reglas del proyecto: sin vectores, matrices (arreglos), recursividad ni
// memoria dinamica (pilas y colas). Los datos que normalmente irian en un
// arreglo se guardan en archivos de texto y se leen de forma secuencial.

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#if defined(__has_include)
#if __has_include("raylib.h") && !defined(SIN_VENTANA)
#define USAR_RAYLIB
#endif
#endif

#ifdef USAR_RAYLIB
#include "raylib.h"
#elif defined(_WIN32)
#include <windows.h>
#endif

using namespace std;

// ---------------------------------------------------------------------------
// Utilidades de entrada
// ---------------------------------------------------------------------------

static void limpiarEntrada() {
    cin.clear();
    cin.ignore(10000, '\n');
}

static int leerEntero(const char *mensaje, int minimo, int maximo) {
    int n;
    cout << mensaje;
    while (!(cin >> n) || n < minimo || n > maximo) {
        limpiarEntrada();
        cout << "Escriba un numero entre " << minimo << " y " << maximo << ": ";
    }
    return n;
}

// ---------------------------------------------------------------------------
// Parte 2: Triangulo Astromash
//   Filas base:  1 / 2 2 / 5 4 5
//   borde(n)  = suma(fila n-1) + primer elemento de la fila n-2
//   T(n,k)    = T(n-1,k-1) + T(n-1,k) + T(n-2,k-1)      (0 < k < n-1)
// Se usan tres archivos como almacenamiento secuencial: fila n-1, fila n-2
// y fila nueva. En memoria solo hay variables escalares.
// ---------------------------------------------------------------------------

static const int ASTROMASH_MAX_FILAS = 20;  // con mas filas se desborda long long
static const int ANCHO_CELDA = 14;
static const char *FILA_ANT = "astromash_ant.tmp";
static const char *FILA_ANT2 = "astromash_ant2.tmp";
static const char *FILA_NUEVA = "astromash_nueva.tmp";

static void imprimirSangria(int n, int total) {
    cout << string((total - n) * ANCHO_CELDA / 2, ' ');
}

static void escribirFilaBase(int n, int total) {
    ofstream out(FILA_NUEVA);
    imprimirSangria(n, total);
    for (int k = 0; k < n; k++) {
        long long v;
        if (n == 1) v = 1;
        else if (n == 2) v = 2;
        else v = (k == 1) ? 4 : 5;
        out << v << ' ';
        cout << setw(ANCHO_CELDA) << v;
    }
    cout << '\n';
}

static void escribirFilaGeneral(int n, int total) {
    long long suma = 0, x;
    ifstream pasada(FILA_ANT);
    while (pasada >> x) suma += x;
    pasada.close();

    ifstream a(FILA_ANT);
    ifstream b(FILA_ANT2);
    long long b0 = 0, aPrev = 0, aCur = 0, bCur = 0;
    b >> b0;
    a >> aPrev;
    long long borde = suma + b0;

    ofstream out(FILA_NUEVA);
    imprimirSangria(n, total);
    out << borde << ' ';
    cout << setw(ANCHO_CELDA) << borde;
    for (int k = 1; k <= n - 2; k++) {
        a >> aCur;
        if (k == 1) bCur = b0;
        else b >> bCur;
        long long v = aPrev + aCur + bCur;
        out << v << ' ';
        cout << setw(ANCHO_CELDA) << v;
        aPrev = aCur;
    }
    out << borde << ' ';
    cout << setw(ANCHO_CELDA) << borde << '\n';
}

static void mostrarAstromash() {
    int filas = leerEntero("Cuantas filas desea ver: ", 1, ASTROMASH_MAX_FILAS);
    cout << '\n';
    for (int n = 1; n <= filas; n++) {
        if (n <= 3) escribirFilaBase(n, filas);
        else escribirFilaGeneral(n, filas);
        // rotar archivos: nueva -> ant -> ant2
        remove(FILA_ANT2);
        rename(FILA_ANT, FILA_ANT2);
        rename(FILA_NUEVA, FILA_ANT);
    }
    remove(FILA_ANT);
    remove(FILA_ANT2);
}

// ---------------------------------------------------------------------------
// Parte 1: puntos en 3D
//   Proyeccion: se gira el espacio un angulo `az` alrededor del eje Z y se
//   inclina `el` hacia el observador. Cada celda de la pantalla se decide
//   preguntando que hay en ella (punto, linea o eje), asi no hace falta una
//   matriz de pantalla.
// ---------------------------------------------------------------------------

static const int PUNTOS_MAX = 10;
static const char *ARCHIVO_PUNTOS = "puntos3d.tmp";

#ifndef USAR_RAYLIB
// ----- Grafica en consola (respaldo cuando no hay raylib) -----

static const int COLUMNAS = 74;
static const int FILAS = 30;
static const double PI_VALOR = 3.14159265358979323846;

struct Vista {
    double az, el;     // angulos en radianes
    double s;          // escala
    double cx, cy;     // origen en pantalla
    double largo;      // los ejes van de -largo a +largo
    bool colores;
    bool cerrar;
};

// Colores ANSI: P anaranjado, Q azul, R verde (y se repiten).
static const char *colorPunto(int i) {
    switch (i % 3) {
        case 0: return "\033[1;38;5;208m";
        case 1: return "\033[1;34m";
        default: return "\033[1;32m";
    }
}
static const char *nombreColor(int i) {
    switch (i % 3) {
        case 0: return "anaranjado";
        case 1: return "azul";
        default: return "verde";
    }
}
static const char *COLOR_EJE = "\033[90m";
static const char *ANSI_RESET = "\033[0m";

static void proyectar(const Vista &v, double x, double y, double z,
                      double &u, double &w) {
    double xr = x * cos(v.az) - y * sin(v.az);
    double yr = x * sin(v.az) + y * cos(v.az);
    u = xr;
    w = z * cos(v.el) + yr * sin(v.el);
}

static void aCelda(const Vista &v, double x, double y, double z,
                   int &col, int &fila) {
    double u, w;
    proyectar(v, x, y, z, u, w);
    col = (int)floor(v.cx + u * v.s + 0.5);
    fila = (int)floor(v.cy - w * v.s / 2.0 + 0.5);
}

static bool puntoEnCelda(const Vista &v, double x, double y, double z,
                         int col, int fila) {
    int c, f;
    aCelda(v, x, y, z, c, f);
    return c == col && f == fila;
}

// Recorre el segmento A-B por muestreo fino y mira si pasa por la celda.
static bool segmentoEnCelda(const Vista &v, double x1, double y1, double z1,
                            double x2, double y2, double z2,
                            int col, int fila) {
    int pasos = (int)(v.s * 2.0 * (fabs(x2 - x1) + fabs(y2 - y1) + fabs(z2 - z1))) + 2;
    for (int i = 0; i <= pasos; i++) {
        double t = (double)i / pasos;
        if (puntoEnCelda(v, x1 + (x2 - x1) * t, y1 + (y2 - y1) * t,
                         z1 + (z2 - z1) * t, col, fila))
            return true;
    }
    return false;
}

static void extender(const Vista &v, double x, double y, double z,
                     double &minU, double &maxU, double &minW, double &maxW) {
    double u, w;
    proyectar(v, x, y, z, u, w);
    if (u < minU) minU = u;
    if (u > maxU) maxU = u;
    if (w < minW) minW = w;
    if (w > maxW) maxW = w;
}

// Ajusta escala y origen para que todo quepa en la pantalla.
static void ajustarVista(Vista &v) {
    double L = v.largo * 1.15;
    double minU = 0, maxU = 0, minW = 0, maxW = 0;
    extender(v, L, 0, 0, minU, maxU, minW, maxW);
    extender(v, -L, 0, 0, minU, maxU, minW, maxW);
    extender(v, 0, L, 0, minU, maxU, minW, maxW);
    extender(v, 0, -L, 0, minU, maxU, minW, maxW);
    extender(v, 0, 0, L, minU, maxU, minW, maxW);
    extender(v, 0, 0, -L, minU, maxU, minW, maxW);
    ifstream in(ARCHIVO_PUNTOS);
    double x, y, z;
    while (in >> x >> y >> z) extender(v, x, y, z, minU, maxU, minW, maxW);

    double sx = (COLUMNAS - 1) / (maxU - minU);
    double sy = (FILAS - 1) * 2.0 / (maxW - minW);
    v.s = sx < sy ? sx : sy;
    v.cx = -minU * v.s + ((COLUMNAS - 1) - (maxU - minU) * v.s) / 2.0;
    v.cy = maxW * v.s / 2.0 + ((FILAS - 1) - (maxW - minW) * v.s / 2.0) / 2.0;
}

static void dibujar(const Vista &v) {
    double L = v.largo;
    double E = v.largo * 1.12;  // posicion de las etiquetas X, Y, Z
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLUMNAS; col++) {
            char ch = ' ';
            const char *color = 0;
            double x, y, z, xa = 0, ya = 0, za = 0, x0 = 0, y0 = 0, z0 = 0;
            int i;

            // 1) puntos
            ifstream in(ARCHIVO_PUNTOS);
            i = 0;
            while (ch == ' ' && in >> x >> y >> z) {
                if (puntoEnCelda(v, x, y, z, col, fila)) {
                    ch = (char)('P' + i);
                    color = colorPunto(i);
                }
                i++;
            }
            in.close();

            // 2) etiquetas de los ejes
            if (ch == ' ') {
                if (puntoEnCelda(v, E, 0, 0, col, fila)) { ch = 'X'; color = COLOR_EJE; }
                else if (puntoEnCelda(v, 0, E, 0, col, fila)) { ch = 'Y'; color = COLOR_EJE; }
                else if (puntoEnCelda(v, 0, 0, E, col, fila)) { ch = 'Z'; color = COLOR_EJE; }
            }

            // 3) lineas que unen los puntos
            if (ch == ' ') {
                ifstream in2(ARCHIVO_PUNTOS);
                i = 0;
                while (ch == ' ' && in2 >> x >> y >> z) {
                    if (i == 0) { x0 = x; y0 = y; z0 = z; }
                    else if (segmentoEnCelda(v, xa, ya, za, x, y, z, col, fila)) {
                        ch = '*';
                    }
                    xa = x; ya = y; za = z;
                    i++;
                }
                if (ch == ' ' && v.cerrar && i > 2 &&
                    segmentoEnCelda(v, xa, ya, za, x0, y0, z0, col, fila))
                    ch = '*';
            }

            // 4) ejes
            if (ch == ' ') {
                if (segmentoEnCelda(v, -L, 0, 0, L, 0, 0, col, fila)) ch = '-';
                else if (segmentoEnCelda(v, 0, -L, 0, 0, L, 0, col, fila)) ch = '/';
                else if (segmentoEnCelda(v, 0, 0, -L, 0, 0, L, col, fila)) ch = '|';
                if (ch != ' ') color = COLOR_EJE;
            }

            if (v.colores && color != 0) cout << color << ch << ANSI_RESET;
            else cout << ch;
        }
        cout << '\n';
    }
}

static void leyenda(const Vista &v) {
    ifstream in(ARCHIVO_PUNTOS);
    double x, y, z;
    int i = 0;
    cout << "Ejes: X = '-'   Y = '/'   Z = '|'   (de -" << v.largo << " a " << v.largo << ")\n";
    while (in >> x >> y >> z) {
        if (v.colores) cout << colorPunto(i);
        cout << (char)('P' + i) << "(" << x << ", " << y << ", " << z << ")";
        if (v.colores) cout << ANSI_RESET;
        else cout << " [" << nombreColor(i) << "]";
        cout << "   ";
        i++;
    }
    cout << '\n';
}

static void graficarEnConsola(double largo, bool cerrar) {
    Vista v;
    v.cerrar = cerrar;
    v.largo = largo;
    char c;
    cout << "Su consola muestra colores? (s/n): ";
    cin >> c;
    v.colores = (c == 's' || c == 'S');
    v.az = PI_VALOR / 4;      // 45 grados
    v.el = PI_VALOR / 6;      // 30 grados

    char opcion = ' ';
    while (opcion != 'q' && opcion != 'Q') {
        ajustarVista(v);
        cout << '\n';
        dibujar(v);
        leyenda(v);
        cout << "[a/d] girar   [w/s] inclinar   [q] volver al menu: ";
        cin >> opcion;
        if (opcion == 'a' || opcion == 'A') v.az -= PI_VALOR / 12;
        else if (opcion == 'd' || opcion == 'D') v.az += PI_VALOR / 12;
        else if (opcion == 'w' || opcion == 'W') v.el += PI_VALOR / 18;
        else if (opcion == 's' || opcion == 'S') v.el -= PI_VALOR / 18;
        if (v.el > 1.4) v.el = 1.4;
        if (v.el < -1.4) v.el = -1.4;
    }
}
#endif  // !USAR_RAYLIB

#ifdef USAR_RAYLIB
// ----- Ventana 3D con raylib -----

// Convencion matematica (z hacia arriba) -> raylib (y hacia arriba).
static Vector3 aMundo(double x, double y, double z) {
    Vector3 p;
    p.x = (float)x;
    p.y = (float)z;
    p.z = (float)(-y);
    return p;
}

static Color colorRaylib(int i) {
    switch (i % 3) {
        case 0: return ORANGE;  // P
        case 1: return BLUE;    // Q
        default: return GREEN;  // R
    }
}

static void etiquetaPantalla(const Camera3D &cam, Vector3 p, const char *txt,
                             int tam, Color c) {
    Vector2 s = GetWorldToScreen(p, cam);
    DrawText(txt, (int)s.x + 8, (int)s.y - 8, tam, c);
}

// Linea con grosor (DrawLine3D siempre mide 1 pixel).
static void linea(double x1, double y1, double z1, double x2, double y2, double z2,
                  float grosor, Color c) {
    DrawCylinderEx(aMundo(x1, y1, z1), aMundo(x2, y2, z2), grosor, grosor, 8, c);
}

static void graficarEnVentana(double largo, bool cerrar) {
    float L = (float)largo;
    float radioPunto = L * 0.03f < 0.12f ? 0.12f : L * 0.03f;
    float grosor = radioPunto * 0.25f;
    int paso = (int)ceil(largo / 10.0);

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1000, 700, "Proyecto 2 - Puntos en 3D");
    SetTargetFPS(60);

    float az = 0.8f, el = 0.5f, distancia = L * 3.2f + 4.0f;
    Camera3D cam;
    cam.target = aMundo(0, 0, 0);
    cam.up.x = 0.0f;
    cam.up.y = 1.0f;
    cam.up.z = 0.0f;
    cam.fovy = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 d = GetMouseDelta();
            az -= d.x * 0.005f;
            el += d.y * 0.005f;
        }
        if (IsKeyDown(KEY_LEFT)) az -= 0.02f;
        if (IsKeyDown(KEY_RIGHT)) az += 0.02f;
        if (IsKeyDown(KEY_UP)) el += 0.02f;
        if (IsKeyDown(KEY_DOWN)) el -= 0.02f;
        distancia -= GetMouseWheelMove() * L * 0.3f;
        if (el > 1.5f) el = 1.5f;
        if (el < -1.5f) el = -1.5f;
        if (distancia < 2.0f) distancia = 2.0f;
        cam.position.x = distancia * cosf(el) * sinf(az);
        cam.position.y = distancia * sinf(el);
        cam.position.z = distancia * cosf(el) * cosf(az);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(cam);

        DrawGrid((int)(L * 2), 1.0f);
        linea(-largo, 0, 0, largo, 0, 0, grosor, RED);
        linea(0, -largo, 0, 0, largo, 0, grosor, DARKGREEN);
        linea(0, 0, -largo, 0, 0, largo, grosor, DARKBLUE);

        // Lectura secuencial: solo se guardan el punto anterior y el primero.
        ifstream in(ARCHIVO_PUNTOS);
        double x, y, z, xa = 0, ya = 0, za = 0, x0 = 0, y0 = 0, z0 = 0;
        int i = 0;
        while (in >> x >> y >> z) {
            if (i == 0) {
                x0 = x; y0 = y; z0 = z;
            } else {
                linea(xa, ya, za, x, y, z, grosor, BLACK);
            }
            DrawSphere(aMundo(x, y, z), radioPunto, colorRaylib(i));
            xa = x; ya = y; za = z;
            i++;
        }
        if (cerrar && i > 2) linea(xa, ya, za, x0, y0, z0, grosor, BLACK);
        EndMode3D();

        // Nombres de los ejes y numeros en cada eje
        etiquetaPantalla(cam, aMundo(L * 1.05f, 0, 0), "X", 20, RED);
        etiquetaPantalla(cam, aMundo(0, L * 1.05f, 0), "Y", 20, DARKGREEN);
        etiquetaPantalla(cam, aMundo(0, 0, L * 1.05f), "Z", 20, DARKBLUE);
        for (int t = paso; t <= (int)largo; t += paso) {
            etiquetaPantalla(cam, aMundo(t, 0, 0), TextFormat("%d", t), 14, GRAY);
            etiquetaPantalla(cam, aMundo(-t, 0, 0), TextFormat("%d", -t), 14, GRAY);
            etiquetaPantalla(cam, aMundo(0, t, 0), TextFormat("%d", t), 14, GRAY);
            etiquetaPantalla(cam, aMundo(0, -t, 0), TextFormat("%d", -t), 14, GRAY);
            etiquetaPantalla(cam, aMundo(0, 0, t), TextFormat("%d", t), 14, GRAY);
            etiquetaPantalla(cam, aMundo(0, 0, -t), TextFormat("%d", -t), 14, GRAY);
        }

        // Nombre y coordenadas de cada punto, y leyenda en la esquina
        ifstream in2(ARCHIVO_PUNTOS);
        i = 0;
        while (in2 >> x >> y >> z) {
            const char *txt = TextFormat("%c(%g, %g, %g)", 'P' + i, x, y, z);
            etiquetaPantalla(cam, aMundo(x, y, z), txt, 18, colorRaylib(i));
            DrawText(txt, 12, 40 + i * 24, 20, colorRaylib(i));
            i++;
        }
        DrawText("Arrastre con el mouse o use las flechas para girar. Rueda: zoom. ESC: volver al menu",
                 12, 10, 16, DARKGRAY);
        EndDrawing();
    }
    CloseWindow();
}
#endif  // USAR_RAYLIB

static bool pedirPuntos(int cantidad) {
    ofstream out(ARCHIVO_PUNTOS);
    if (!out) {
        cout << "No se pudo crear el archivo temporal (revise permisos de la carpeta).\n";
        return false;
    }
    for (int i = 0; i < cantidad; i++) {
        double x, y, z;
        cout << "Punto " << (char)('P' + i) << " (x y z): ";
        while (!(cin >> x >> y >> z)) {
            limpiarEntrada();
            cout << "Entrada invalida, escriba tres numeros separados por espacio: ";
        }
        out << x << ' ' << y << ' ' << z << '\n';
    }
    return true;
}

static double mayorCoordenada() {
    ifstream in(ARCHIVO_PUNTOS);
    double x, y, z, m = 0;
    while (in >> x >> y >> z) {
        if (fabs(x) > m) m = fabs(x);
        if (fabs(y) > m) m = fabs(y);
        if (fabs(z) > m) m = fabs(z);
    }
    return m;
}

static void graficarPuntos() {
    int cantidad = leerEntero("Cuantos puntos desea graficar (1-10): ", 1, PUNTOS_MAX);
    if (!pedirPuntos(cantidad)) return;

    bool cerrar = false;
    if (cantidad > 2) {
        char r;
        cout << "Cerrar la figura (unir el ultimo con el primero)? (s/n): ";
        cin >> r;
        cerrar = (r == 's' || r == 'S');
    }

    double largo = ceil(mayorCoordenada());
    if (largo < 3) largo = 3;

#ifdef USAR_RAYLIB
    cout << "Abriendo la ventana 3D... (ESC para volver al menu)\n";
    graficarEnVentana(largo, cerrar);
#else
    graficarEnConsola(largo, cerrar);
#endif
    remove(ARCHIVO_PUNTOS);
}

// ---------------------------------------------------------------------------

int main() {
#if defined(_WIN32) && !defined(USAR_RAYLIB)
    // Activa los colores ANSI en consolas modernas de Windows.
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo;
    if (GetConsoleMode(h, &modo)) SetConsoleMode(h, modo | 0x0004);
#endif

    int opcion = 0;
    while (opcion != 3) {
        cout << "\n=== PROYECTO 2 ===\n"
             << "1) Graficar puntos en 3D\n"
             << "2) Triangulo Astromash\n"
             << "3) Salir\n";
        opcion = leerEntero("Opcion: ", 1, 3);
        if (opcion == 1) graficarPuntos();
        else if (opcion == 2) mostrarAstromash();
    }
    return 0;
}
