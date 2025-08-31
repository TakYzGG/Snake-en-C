// -- Snake en C --

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// linux
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

// declarar el numero de filas y columnas
#define rows 25
#define columns 50

// variables globales
char cuadricula[rows][columns];
char cabeza = 'o';
int long_snake = 3; // tamano total de la snake
char direccion = 's'; // saber la direccion hacia donde se mueve la snake
int comida_x, comida_y;
int pause_time = 187500; // microsegundos que tarda el bucle principal en
                            // refrescarse

void configurar_terminal(void) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // modo sin buffer y sin eco
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    // poner stdin en no bloqueante
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void restaurar_terminal(void) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    // volver a bloqueante
    fcntl(STDIN_FILENO, F_SETFL, 0);
}

// leer las teclas de direccion (flechas)
void leer_tecla(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 27) { // ESC
            read(STDIN_FILENO, &c, 1); // [
            read(STDIN_FILENO, &c, 1);
        switch (c) {
            case 'A':
                if (direccion != 's') {
                    //pause_time = 250000;
                    direccion = 'w';
                }
                break;
            case 'B':
                if (direccion != 'w') {
                    //pause_time = 250000;
                    direccion = 's';
                }
                break;
            case 'C':
                if (direccion != 'a') {
                    //pause_time = 125000;
                    direccion = 'd';
                }
                break;
            case 'D':
                if (direccion != 'd') {
                    //pause_time = 125000;
                    direccion = 'a';
                }
                break;
            }
        }
    }
}

// llena toda la cuadricula con espacios
void rellenar_cuadricula(void) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            cuadricula[i][j] = ' ';
        }
    }
}

// dibujar la cuadricula (con bordes incluidos)
void dibujar_cuadricula(void) {
    int top_button_bd = columns + 2; // sumar el borde de las columnas

    // dibujar el borde superior
    for (int _ = 0; _ < top_button_bd; _++) {
        printf("-");
    }

    printf("\n"); // salto de linea

    // dibujar el borde de la izquierda y derecha
    for (int i = 0; i < rows; i++) {
        printf("|"); // primer caracter de la fila
        for (int j = 0; j < columns; j++){
            printf("%c", cuadricula[i][j]);
        }
        printf("|"); // ultimo caracter de la fila
        printf("\n"); // salto de linea
    }

    // dibujar el borde inferior
    for (int _ = 0; _ < top_button_bd; _++) {
        printf("-");
    }

    printf("\n"); // salto de linea
}

// colocar la snake en la cuadricula
void colocar_snake(void) {
    cuadricula[0][0] = '#';
    cuadricula[1][0] = '#';
    cuadricula[2][0] = cabeza;
}

// encontrar la posicion de la cabeza de la snake
void encontrar_cabeza(int *x, int *y) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if (cuadricula[i][j] == cabeza) {
                // guardar la posicion si es encontrada
                *x = i;
                *y = j;
            }
        }
    }
}


// buscar el final recorriendo el tablero
//
void encontrar_final(int *x, int *y) {
    int posc_x, posc_y;
    encontrar_cabeza(&posc_x, &posc_y);

    // avanzar desde la cabeza hasta el final
    int px = -1, py = -1; // posición anterior
    while (1) {
        int nx = -1, ny = -1;
        int vecinos = 0;

        // buscar el siguiente segmento (que no sea la posición anterior)
        // arriba
        if (posc_x > 0 && cuadricula[posc_x-1][posc_y] == '#' &&
           (posc_x-1 != px || posc_y != py)) {
            nx = posc_x-1; ny = posc_y; vecinos++;
        }

        // abajo
        if (posc_x < rows-1 && cuadricula[posc_x+1][posc_y] == '#' &&
           (posc_x+1 != px || posc_y != py)) {
            nx = posc_x+1; ny = posc_y; vecinos++;
        }

        // izquierda
        if (posc_y > 0 && cuadricula[posc_x][posc_y-1] == '#' &&
           (posc_x != px || posc_y-1 != py)) {
            nx = posc_x; ny = posc_y-1; vecinos++;
        }

        // derecha
        if (posc_y < columns-1 && cuadricula[posc_x][posc_y+1] == '#' &&
           (posc_x != px || posc_y+1 != py)) {
            nx = posc_x; ny = posc_y+1; vecinos++;
        }

        if (vecinos == 0) {
            // cola encontrada
            *x = posc_x;
            *y = posc_y;
            return;
        }

        // avanzar
        px = posc_x; py = posc_y;
        posc_x = nx; posc_y = ny;
    }
}

// mover la snake hacia delante
int mover_snake(void) {
    int posc_x, posc_y; // cabeza
    int posf_x, posf_y; // final
    int new_x, new_y; // nueva posicion para la cabeza

    // tener la posicion de la cabeza
    encontrar_cabeza(&posc_x, &posc_y);

    // calcular nueva posicion para la cabeza
    if (direccion == 'w') {
        // mover la snake hacia arriba
        new_x = posc_x - 1;
        new_y = posc_y;
    } else if (direccion == 'a') {
        // mover la snake hacia la izquierda
        new_x = posc_x;
        new_y = posc_y - 1;
    } else if (direccion == 's') {
        // mover la snake hacia abajo
        new_x = posc_x + 1;
        new_y = posc_y;
    } else if (direccion == 'd') {
        // mover la snake hacia la derecha
        new_x = posc_x;
        new_y = posc_y + 1;
    } else {
        printf("ERROR SUPREMO");
    }

    // mover la cabeza de la snake
    if (perder(new_x, new_y)){
        return 1;
    } else {
        cuadricula[posc_x][posc_y] = '#';
        cuadricula[new_x][new_y] = cabeza;
    }

    // encontrar el final de la snake
    encontrar_final(&posf_x, &posf_y);

    // borrar ultimo caracter
    cuadricula[posf_x][posf_y] = ' ';
    return 0;
}

/* colocar la comida en una posicion aleatoria de la cuadricula mientras este
   vacia */
void colocar_comida(void) {
    while (1) {
        int x = rand() % rows;
        int y = rand() % columns;
        if (cuadricula[x][y] == ' ') {
            cuadricula[x][y] = '@';
            comida_x = x;
            comida_y = y;
            break;
        }
    }
}

// comprobar si la cabeza de la snake coincide con la posicion de la comida
void comer_comida(void) {
    int posc_x, posc_y;
    int posf_x, posf_y;
    encontrar_cabeza(&posc_x, &posc_y);

    if (posc_x == comida_x && posc_y == comida_y) {
        cuadricula[posc_x][posc_y] = '#';

        if (direccion == 'w') {
            cuadricula[posc_x -1][posc_y] = cabeza;
        } else if (direccion == 'a') {
            cuadricula[posc_x][posc_y -1] = cabeza;
        } else if (direccion == 'd') {
            cuadricula[posc_x][posc_y +1] = cabeza;
        } else if (direccion == 's') {
            cuadricula[posc_x +1][posc_y] = cabeza;
        }
        colocar_comida();
    }
}

// comprobar si la snake choca con alguna pared
int perder(int x, int y) {
    if (x == -1 || x == rows || y == -1 || y == columns) {
        return 1;
    } else if (cuadricula[x][y] == '#') {
        return 1;
    }
    return 0;
}

// funcion principal
int main(void) {
    configurar_terminal();
    srand(time(NULL));
    rellenar_cuadricula();
    colocar_snake();
    colocar_comida();
    while (1) {
        printf("\033[2J\033[H"); // limpiar pantalla (solo linux)
        leer_tecla();
        dibujar_cuadricula();
        //sleep(1); // segundos
        usleep(pause_time); // microsegundos
        if (mover_snake()) {
            break;
        }
        comer_comida();
    }
    printf("Perdiste\n");
    restaurar_terminal();
    return 0;
}
