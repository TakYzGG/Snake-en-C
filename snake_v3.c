// -- Snake_v3 en C --

// -- librerias generales --
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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
FILE *archivo;
const char *name_file = ".highscore.txt"; // archivo de guardado de los puntos
char cuadricula[rows][columns];
char cabeza = 'o';
char direccion = 'd';
int long_snake = 3;
int comida_x, comida_y;
char comida_char;
int points = 0;
int count = 0; // sirve para saber cuando aumentar la velocidad del juego

// -- codigo para linux --
#if defined(__linux__)
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define OS "linux"
    int sleep_time = 175000;
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
        usleep(sleep_time); // microsegundos
    }

    // -- sumar tiempo a la pausa (mas lento) --
    void sumar_tiempo(void) {
        sleep_time += 5000;
    }

    // -- restar tiempo a la pausa (mas rapido) --
    void restar_tiempo(void) {
        if (sleep_time > 100000 && count == 5) {
            sleep_time -= 5000;
            count = 0;
        }
    }
#endif

// -- librerias para windows --
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <conio.h>  // para getch() y kbhit()
    #define OS "windows"
    int sleep_time = 175;
    
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
        Sleep(sleep_time); // milisegundos
    }

    // -- sumar tiempo a la pausa (mas lento) --
    void sumar_tiempo(void) {
        sleep_time += 5;
    }

    // -- restar tiempo a la pausa (mas rapido) --
    void restar_tiempo(void) {
        if (sleep_time > 100 && count == 5) {
            sleep_time -= 5;
            count = 0;
        }
    }
#endif

// -- continuar la ejecucion despues de precionar enter --
void continuar(void) {
    getchar();
    printf("\nPresiona ENTER para continuar");
    getchar();
}

// -- comprobar que esten ordenados --
int comprobar_orden(int *array, int n) {
    for (int i = 0; i < n -1; i++) {
        if (array[i] > array[i + 1]) {
            return 0;
        }
    }
    return 1;
}

// -- ordenamiento burbuja --
int ordenamiento_burbuja(int *array, int n) {
    int continuar = 1;
    while (continuar) {
        for (int i = 0; i < n -1; i++) {
            if (array[i] > array[i + 1]) {
                int temp = array[i + 1];
                array[i + 1] = array[i];
                array[i] = temp;
            }
        }
        // terminar el bucle
        if (comprobar_orden(array, n)) {
            continuar = 0;
        }
    }
}

// -- crear archivo para guardar los puntos --
void create_highscore(char *player) {
    archivo = fopen(name_file, "w");
    fprintf(archivo, "Jugador: Puntos\n"); // linea 1
    fprintf(archivo, "%s: %d\n", player, points);
}

// -- agregar mas puntuaciones si ya hay una existente --
void add_highscore(char *player) {
    archivo = fopen(name_file, "a");
    fprintf(archivo, "%s: %d\n", player, points);
}

// -- escribir el archivo de guardado con las puntuaciones ordenadas
void reescribir_guardado(node *puntuaciones, int n) {
    archivo = fopen(name_file, "w");
}

// -- ordenar puntuacion --
int ordenar_puntuacion(void) {
    archivo = fopen(name_file, "r");
    int inicio = 0; // saltar la primer linea
    int i;
    char linea[128];
    char *separador;
    node puntuaciones[128]; // maximo 128 jugadores

    // comprobar si el archivo de guardado no existe
    if (archivo == NULL) {
        return 0;
    }

    // leer linea por linea
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        // saltar la primer linea
        if (inicio < 1) {
            inicio++;
            continue;
        }

        // buscar donde estan los :
        linea[strcspn(linea, "\n")] = '\0';
        separador = strchr(linea, ':') ;

        // si existen :
        if (separador != NULL) {
            *separador = '\0'; // cortar la linea
            char *usuario = linea;
            int points = atoi(separador +1);
            node x;
            x.x = usuario; // guardar usuario
            x.y = points; // guardar puntos
            puntuaciones[i] = x;
            i++;
        }
    }
    ordenamiento_burbuja(puntuaciones, i);
    return 0;
}

// -- cerrar el archivo --
void close_file(void) {
    fclose(archivo);
}

// -- rellenar toda la cuadricula --
void rellenar_cuadricula(void) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            cuadricula[i][j] = ' ';
        }
    }
    cuadricula[comida_x][comida_y] = comida_char;
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

// -- generar la comida --
char generar_comida(int r) { // r = retorno
    // @: comida normal (+1 punto) 45%
    // +: da mas puntos (+2 puntos) 15%
    // $: da mas puntos (+3 puntos) 10%
    // -: resta tamano a la snake (-1 de tamano) %5
    // !: reduce la velocidad %5
    // ?: una comida aleatoria %20
    char comidas[] = {'@', '+', '$', '-', '!', '?'};
    int probabilidades[] = {45, 15, 10, 5, 5, 20};
    int size = sizeof(comidas) / sizeof(comidas[0]);

    // generar un numero aleatorio entre 1 y 100
    int num = rand() % 100 + 1;

    // seleccionar la comida segun el numero
    int acumulado = 0;
    for (int i = 0; i < size; i++) {
        acumulado += probabilidades[i];
        if (num <= acumulado) {
            // si r es 1 termina la funcion y retorna la comida
            if (r) return comidas[i];
            comida_char = comidas[i];
            break;
        }
    }
}

// -- generar una posicion aleatoria para colocar la comido en la cuadricula --
void colocar_comida(void) {
    generar_comida(0); // genera la comida

    // buscar una posicion random que este vacia
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
        int rep = 1;
        while (rep) {
            rep = 0;
            switch (comida_char) {
                case '@': points += 1; break;
                case '+': points += 2; break;
                case '$': points += 3; break;
                case '-':
                    long_snake--;
                    points++;
                    break;
                case '!':
                    points++;
                    sumar_tiempo(); // reduce velocidad
                    count = 0;
                    break;
                case '?':
                    comida_char = generar_comida(1);
                    rep = 1; // repetir el switch denuevo
            }
        }

        // si la comida es distinta a "-"
        if (comida_char != '-') {
            snake[long_snake] = new;
            long_snake++;
        }

        // aumentar 1 al contador de comida y generar una comida nueva
        count++;
        colocar_comida();
    }
}

// -- ejecutar el juego --
void game(void) {
    node a, b, c;
    // posicion inicial de la snake
    a.x = 0, a.y = 2, snake[0] = a;
    b.x = 0, b.y = 1, snake[1] = b;
    c.x = 0, c.y = 0, snake[2] = c;

    // configurar la terminal para linux
    if (OS == "linux") configurar_terminal();

    srand(time(NULL));
    rellenar_cuadricula(); // crear la cuadricula vacia
    colocar_comida(); // agregar la comida

    // bucle del juego
    while (1) {
        limpiar_pantalla();
        leer_tecla();
        rellenar_cuadricula();
        colocar_snake();
        dibujar_cuadricula();
        printf("Puntos: %d\n", points);
        restar_tiempo();
        pausar();
        comer();
        if (mover_snake()) {
            break;
        }
    }

    // mensaje de cuando pierdes
    printf("Perdiste\n");

    // restaurar la terminal para linux
    if (OS == "linux") restaurar_terminal();

    // pedir nombre al jugador
    char player[9];
    printf("Ingresa tu nombre (8 caracteres maximo): ");
    scanf("%8s", player); // guardar solo los primeros 8 caracteres

    // guardar puntuacion
    archivo = fopen(name_file, "r");
    if (archivo == NULL) {
        create_highscore(player);
    } else {
        add_highscore(player);
    }
}

// -- mostrar como se juega y las reglas --
void reglas(void) {
    limpiar_pantalla();
    printf("-- Como jugar y regla --\n");
    printf("[1] Las flechas de dirrecion mueven a la snake\n");
    printf("[2] Si chocas con las paredes o contigo mismo pierdes\n");
    printf("[3] Cada 5 veces que la snake coma, el juego va mas rapido\n");
    printf("\n-- Tipos de comida --\n");
    printf("[@] +1 punto\n");
    printf("[+] +2 puntos\n");
    printf("[$] +3 puntos\n");
    printf("[-] Reduce el tamano de la snake\n"); // no tengo enie
    printf("[!] Hace que el juego sea mas lento y reinicia el contador de");
    printf(" comida\n");
    printf("[?] Tiene el efecto de cualquier otra comida de forma aleatoria\n");
    continuar();
}

// -- mostrar las puntuaciones --
int mostrar_puntuaciones(void) {
    archivo = fopen(name_file, "r");
    int count = 0;
    char linea[128];
    char *separador;
    limpiar_pantalla();

    // comprobar si el archivo de guardado no existe
    if (archivo == NULL) {
        printf("No hay datos guardados\n");
        continuar();
        return 0;
    }

    // leer linea por linea
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        // saltar la primer linea
        if (count < 1) {
            count++;
            continue;
        }

        // buscar donde estan los :
        linea[strcspn(linea, "\n")] = '\0';
        separador = strchr(linea, ':') ;

        // si existen :
        if (separador != NULL) {
            *separador = '\0'; // cortar la linea
            char *usuario = linea;
            printf("usuario: %s\n", usuario);
            int poins = atoi(separador +1);
            printf("puntos: %d\n", poins);
        }

    }
    continuar();
    return 0;
}

// -- mostrar los creditos del juego --
void creditos(void) {
    limpiar_pantalla();
    printf("-- Programadores --\n");
    printf("Programador: TakYzGG\n");
    printf("Github: TakYzGG\n");
    printf("Correo: takyz_gg@outlook.com\n");
    printf("\n-- Codigo fuente y ejecutables --\n");
    printf("https://github.com/TakYzGG/Snake-en-C\n");
    continuar();
}

// -- gracias por leer el codigo fuente --
void mensaje_sorpresa(void) {
    limpiar_pantalla();
    printf("-- Mensaje sorpresa --\n");
    printf("Si estas leyendo esto es porque entraste al codigo fuente\n");
    printf("Gracias por jugar mi juego\n");
    continuar();
}

// -- ejecutar la accion del menu dependiendo la opcion del usuario --
void func_menu(int option) {
    switch (option) {
        case 1: reglas(); break;
        case 2: game(); break;
        case 3: mostrar_puntuaciones(); break;
        case 4: creditos(); break;
        case 404: mensaje_sorpresa(); break;
    }
}

// -- funcion principal --
int main(void) {
    int option = -1;

    // menu principal
    while (option < 5) {
        limpiar_pantalla();
        printf("-- Menu snake v3 --\n");
        printf("[1] Como juegar?\n");
        printf("[2] Nueva partida\n");
        printf("[3] Ver puntuaciones\n");
        printf("[4] Creditos\n");
        printf("[*] Salir\n");
        printf(">>> ");
        scanf("%d", &option);
        func_menu(option);
    }

    return 0;
}
