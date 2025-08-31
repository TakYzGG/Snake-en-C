// -- Snake_v2 en C --

// -- librerias generales --
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// -- estructura para la serpiente --
// esto va a servir para guardar las posiciones de las partes de la snake
typedef struct node {
    int x; // pos x de la parte
    int y; // pos y de la parte
} node;

// -- declarar filas y columnas --
#define rows 25
#define columns 50

// -- variables globales --
node snake[rows * columns];

// -- variables globales --
char cuadricula[rows][columns];
char cabeza = 'o';
char direccion = 'd';
int long_snake = 3;
int comida_x, comida_y;
int points = 0;

// -- codigo para linux --
#if defined(__linux__)
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define OS "linux"
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
                if (direccion != 's' && c == 'A') {
                    direccion = 'w';
                }
                if (direccion != 'a' && c == 'C') {
                    direccion = 'd';
                }
                if (direccion != 'w' && c == 'B') {
                    direccion = 's';
                }
                if (direccion != 'd' && c == 'D') {
                    direccion = 'a';
                }
            }
        }
    }

    // -- limpiar la pantalla --
    void limpiar_pantalla(void) {
        printf("\033[2J\033[H"); // limpiar pantalla (solo linux)
    }

    // -- pausar la ejecucion --
    void pausar(void) {
        usleep(175000); // microsegundos
    }
#endif

// -- librerias para windows --
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <conio.h>  // para getch() y kbhit()
    #define OS "windows"
    
    void leer_tecla(void) {
        if (_kbhit()) {  // hay una tecla presionada
            int c = _getch();  // leer la tecla
            if (c == 224) {    // teclas especiales (flechas)
                c = _getch();
                switch(c) {
                    case 72:  // flecha arriba
                        if (direccion != 's') direccion = 'w';
                        break;
                    case 80:  // flecha abajo
                        if (direccion != 'w') direccion = 's';
                        break;
                    case 75:  // flecha izquierda
                        if (direccion != 'd') direccion = 'a';
                        break;
                    case 77:  // flecha derecha
                        if (direccion != 'a') direccion = 'd';
                        break;
                }
            }
        }
    }

    // -- limpiar la pantalla --
    void limpiar_pantalla(void) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coordScreen = {0, 0};
        DWORD cCharsWritten;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD dwConSize;

        // Obtener tamaño de la consola
        if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

        // Llenar con espacios
        FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);

        // Restaurar atributos
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);

        // Mover cursor al inicio
        SetConsoleCursorPosition(hConsole, coordScreen);
    }

    // -- pausar la ejecucion --
    void pausar(void) {
        Sleep(175); // milisegundos
    }
#endif

// -- rellenar toda la cuadricula --
void rellenar_cuadricula(void) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            cuadricula[i][j] = ' ';
        }
    }
    cuadricula[comida_x][comida_y] = '@';
}

// -- colocar la cuadricula las partes de la snake --
void colocar_snake(void) {
    for (int i = 0; i < long_snake; i++) {
        cuadricula[snake[i].x][snake[i].y] = '#';
    }
}

// -- dibujar la cuadricula (con bordes incluidos) --
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

// -- agregar un elemento al inicio del array (snake) --
void add_element(node new) {
    node new_list[long_snake];
    new_list[0] = new;

    // crear el nuevo array con los elementos corridos una posicion
    for (int i = 1; i < long_snake; i++) {
        new_list[i] = snake[i -1];
    }

    // copiar el nuevo array a snake
    for (int i = 0; i < long_snake; i++) {
        snake[i] = new_list[i];
    }
}

// -- obtener la siguiente celda --
node sig_celda(void) {
    node new;
    if (direccion == 'w') {
        new.x = snake[0].x -1;
        new.y = snake[0].y;
    } else if (direccion == 'a') {
        new.x = snake[0].x;
        new.y = snake[0].y -1;
    } else if (direccion == 's') {
        new.x = snake[0].x +1;
        new.y = snake[0].y;
    } else if (direccion == 'd') {
        new.x = snake[0].x;
        new.y = snake[0].y +1;
    }

    return new;
}

// -- comprobar si perdiste --
int game_over(int x, int y) {
    // si la cabeza choca contra algun borde
    if (x == -1 || y == -1 || x == rows || y == columns) {
        return 1;
    } 

    // si la cabeza choca contra alguna parte del cuerpo
    if (cuadricula[x][y] == '#') {
        return 1;
    }
    return 0;
}


// -- mover la snake --
int mover_snake(void) {
    node new = sig_celda();

    if (game_over(new.x, new.y)) {
        return 1;
    } else {
        add_element(new); // pone a new en la posicion 0
    }

    return 0;
}

// -- generar una posicion aleatoria para la comido en la cuadricula --
void generar_comida(void) {
    while (1) {
        int x = rand() % rows;
        int y = rand() % columns;
        if (cuadricula[x][y] == ' ') {
            comida_x = x;
            comida_y = y;
            break;
        }
    }
}

// -- agregar una parte a la snake cuando come --
void comer(void) {
    node new = sig_celda();
    if (snake[0].x == comida_x && snake[0].y == comida_y) {
        snake[long_snake] = new;
        long_snake++;
        points++;
        generar_comida();
    }
}

// -- funcion principal --
int main(void) {
    node a, b, c;
    // posicion inicial de la snake
    a.x = 0, a.y = 2, snake[0] = a;
    b.x = 0, b.y = 1, snake[1] = b;
    c.x = 0, c.y = 0, snake[2] = c;

    if (OS == "linux") configurar_terminal();
    srand(time(NULL));
    rellenar_cuadricula(); // crear la cuadricula vacia
    generar_comida(); // agregar la comida
    while (1) {
        limpiar_pantalla();
        leer_tecla();
        rellenar_cuadricula();
        colocar_snake();
        dibujar_cuadricula();
        printf("Puntos: %d\n", points);
        pausar();
        comer();
        if (mover_snake()) {
            break;
        }
    }

    // mensaje de cuando pierdes
    printf("Perdiste\n");
    if (OS == "linux") restaurar_terminal();

    return 0;
}
