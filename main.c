///LIBRERIAS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

///CONSTANTES
#define ELECTORES_ESPERADOS 2000

//RS (Primo cercano a 2000 / 1.61)
#define M_RS 1249

//RAL
#define M_RAL 2531

//RAC (Primo cercano para rho = 0.83)
#define M_RAC 2411


//Definicion de structs
typedef struct {
    long dni;
    char nombreApellido[50];
    char domicilio[80];
    int codigoPostal;
    int numeroMesa;
    int circuito;
    int estado;               // 0: Virgen, 1: Ocupado, 2: Libre
} elector;

typedef struct {
    int cantidad;
    float costo_acu;
    float costo_max;
} MetricasOp;

typedef struct {
    MetricasOp evocar_exito;
    MetricasOp evocar_fracaso;
} Estadisticas;

void InicializarMetricas(MetricasOp *m) {
    m->cantidad = 0;
    m->costo_acu = 0.0;
    m->costo_max = 0.0;
}

void RegistrarCosto(MetricasOp *m, float costo_individual) {
    m->cantidad++;
    m->costo_acu += costo_individual;
    if (costo_individual > m->costo_max) {
        m->costo_max = costo_individual;
    }
}

int hashing(int dni, int M) {
    char x[15];
    int longitud, i;
    int contador = 0;

    sprintf(x, "%d", dni);
    longitud = strlen(x);
    for (i = 0; i < longitud; i++) {
        contador += ((int)x[i]) * (i + 1);
    }
    return (contador % M);
}

int CompararNombresNoCaseSensitive(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (tolower((unsigned char)*str1) != tolower((unsigned char)*str2)) return 0;
        str1++;
        str2++;
    }
    return (*str1 == *str2) ? 1 : 0;
}

int CompararNuplas(elector e1, elector e2) {
    if (e1.dni == e2.dni &&
        e1.codigoPostal == e2.codigoPostal &&
        e1.numeroMesa == e2.numeroMesa &&
        e1.circuito == e2.circuito &&
        CompararNombresNoCaseSensitive(e1.nombreApellido, e2.nombreApellido) == 1 &&
        CompararNombresNoCaseSensitive(e1.domicilio, e2.domicilio) == 1) {
        return 1;
    }
    return 0;
}

// --- REBALSE ABIERTO LINEAL (RAL) ---
typedef struct{
    elector electores[M_RAL];
    int cantidad;
} RAL;

void InitRAL(RAL *ral){
    ral->cantidad = 0;
    for(int i = 0; i < M_RAL; i++){
        ral->electores[i].estado = 0;
        ral->electores[i].dni = 0;
        ral->electores[i].codigoPostal = 0;
        ral->electores[i].numeroMesa = 0;
        ral->electores[i].circuito = 0;
        ral->electores[i].nombreApellido[0] = '\0';
        ral->electores[i].domicilio[0] = '\0';
    }
}

void LocalizarRAL(RAL *ral, long dni_buscado, int *pos, int *exito, int *costo){
    int actual = hashing((int)dni_buscado, M_RAL);
    int primeraLibre = -1;
    int celdasRecorridas = 0;
    *exito = 0;
    *costo = 0;


    while(celdasRecorridas < M_RAL){
        (*costo)++;
        int estado = ral->electores[actual].estado;

        switch(estado){
            case 0:
                *pos = (primeraLibre != -1) ? primeraLibre : actual;
                return;
            case 1:
                if(ral->electores[actual].dni == dni_buscado){
                    *pos = actual;
                    *exito = 1;
                    return;
                }
                break;
            case 2:
                if (primeraLibre == -1){
                    primeraLibre = actual;
                }
                break;
        }
        actual = (actual + 1) % M_RAL;
        celdasRecorridas++;
    }
    *pos = (primeraLibre != -1) ? primeraLibre : actual;
}

void AltaRAL(RAL *ral, elector nuevoElector,int *exito){
    int costoPrima = 0, exitoPrima = 0, pos;

    if (ral->cantidad == M_RAL) {
        *exito = 0;
        return;
    }

    LocalizarRAL(ral, nuevoElector.dni, &pos, &exitoPrima, &costoPrima);

    if(exitoPrima == 0){
        nuevoElector.estado = 1;
        ral->electores[pos] = nuevoElector;
        ral->cantidad++;
        *exito = 1;
    } else {
        *exito = 0;
    }
}

void BajaRAL(RAL *ral, elector nuplaBaja,int *exito){
    int pos, costoPrima, exitoPrima;
    LocalizarRAL(ral, nuplaBaja.dni, &pos, &exitoPrima, &costoPrima);
    if(exitoPrima != 0){
        if(CompararNuplas(nuplaBaja, ral->electores[pos]) != 0){
            ral->electores[pos].estado = 2;
            ral->cantidad--;
            *exito = 1;
        } else {
            *exito = 0;
        }
    } else {
        *exito = 0;
    }
}

void EvocacionRAL(RAL *ral, long dni_buscado, elector *elector_evocado, int *exito, float *costo) {
    int pos, exitoPrima, costoPrima;
    LocalizarRAL(ral, dni_buscado, &pos, &exitoPrima, &costoPrima);
    *costo = (float)costoPrima;
    if (exitoPrima == 1) {
        *elector_evocado = ral->electores[pos];
        *exito = 1;
    } else {
        *exito = 0;
    }
}

// --- REBALSE ABIERTO CUADRATICO (RAC) ---
typedef struct{
    elector electores[M_RAC];
    int cantidad;
} RAC;

void InitRAC(RAC *rac){
    rac->cantidad = 0;
    for(int i = 0; i < M_RAC; i++){
        rac->electores[i].estado = 0;
        rac->electores[i].dni = 0;
        rac->electores[i].codigoPostal = 0;
        rac->electores[i].numeroMesa = 0;
        rac->electores[i].circuito = 0;
        rac->electores[i].nombreApellido[0] = '\0';
        rac->electores[i].domicilio[0] = '\0';
    }
}

void LocalizarRAC(RAC *rac, long dni_buscado, int *pos, int *exito, int *costo){

    int pos_inicial = hashing((int)dni_buscado, M_RAC);
    int actual = pos_inicial;
    int primeraLibre = -1;
    int celdasRecorridas = 0;

    *exito = 0;
    *costo = 0;

    while(celdasRecorridas < M_RAC){
        (*costo)++;
        int estado = rac->electores[actual].estado;

        switch(estado){
            case 0:
                *pos = (primeraLibre != -1) ? primeraLibre : actual;
                return;
            case 1:
                if(rac->electores[actual].dni == dni_buscado){
                    *pos = actual;
                    *exito = 1;
                    return;
                }
                break;
            case 2:
                if (primeraLibre == -1){
                    primeraLibre = actual;
                }
                break;
        }

        celdasRecorridas++;
        actual = (actual + celdasRecorridas) % M_RAC;
    }

    *pos = (primeraLibre != -1) ? primeraLibre : actual;
}

void AltaRAC(RAC *rac, elector nuevoElector,int *exito){
    int costoPrima = 0, exitoPrima = 0, pos;

    if (rac->cantidad == M_RAC) {
        *exito = 0;
        return;
    }

    LocalizarRAC(rac, nuevoElector.dni, &pos, &exitoPrima, &costoPrima);

    if(exitoPrima == 0){
        nuevoElector.estado = 1;
        rac->electores[pos] = nuevoElector;
        rac->cantidad++;
        *exito = 1;
    } else {
        *exito = 0;
    }
}

void BajaRAC(RAC *rac, elector nuplaBaja, int *exito){
    int pos, costoPrima, exitoPrima;
    LocalizarRAC(rac, nuplaBaja.dni, &pos, &exitoPrima, &costoPrima);

    if(exitoPrima != 0){
        if(CompararNuplas(nuplaBaja, rac->electores[pos]) != 0){
            rac->electores[pos].estado = 2;
            rac->cantidad--;
            *exito = 1;
        } else {
            *exito = 0;
        }
    } else {
        *exito = 0;
    }
}

void EvocacionRAC(RAC *rac, long dni_buscado, elector *elector_evocado, int *exito, float *costo) {
    int pos, exitoPrima, costoPrima;
    LocalizarRAC(rac, dni_buscado, &pos, &exitoPrima, &costoPrima);
    *costo = (float)costoPrima;
    if (exitoPrima == 1) {
        *elector_evocado = rac->electores[pos];
        *exito = 1;
    } else {
        *exito = 0;
    }
}

// --- REBALSE SEPARADO (RS) ---
typedef struct NodoRS {
    elector e;
    struct NodoRS *siguiente;
} NodoRS;

typedef struct {
    NodoRS *tabla[M_RS];
    int cantidad;
} RS;

void InitRS(RS *rs) {
    rs->cantidad = 0;
    for(int i = 0; i < M_RS; i++) {
        rs->tabla[i] = NULL;
    }
}

//Libera todos los nodos
void DestruirRS(RS *rs) {
    for (int i = 0; i < M_RS; i++) {
        NodoRS *p = rs->tabla[i];
        while (p != NULL) {
            NodoRS *aux = p;
            p = p->siguiente;
            free(aux);
        }
        rs->tabla[i] = NULL;
    }
    rs->cantidad = 0;
}

void LocalizarRS(RS *rs, long dni_buscado, NodoRS **posicion, NodoRS **anterior, int *exito, float *costo) {
    int indice = hashing((int)dni_buscado, M_RS);
    NodoRS *p = rs->tabla[indice];
    NodoRS *ant = NULL;

    *costo = 1.0;

    while (p != NULL && p->e.dni != dni_buscado) {
        (*costo)++;
        ant = p;
        p = p->siguiente;
    }

    if (p != NULL) {
        (*costo)++;
        *posicion = p;
        *anterior = ant;
        *exito = 1;
    } else {
        *posicion = NULL;
        *anterior = ant;   //ultimo nodo recorrido, o NULL si lista vacia
        *exito = 0;
    }
}

void AltaRS(RS *rs, elector nuevo_elector, int *exito) {
    NodoRS *posicion = NULL;
    NodoRS *anterior = NULL;
    float costo_localizar = 0.0;

    LocalizarRS(rs, nuevo_elector.dni, &posicion, &anterior, exito, &costo_localizar);

    if (*exito == 1) {
        *exito = 2; //Fracaso ya existe el X
    } else {
        NodoRS *nuevo_nodo = (NodoRS*)malloc(sizeof(NodoRS));
        if (nuevo_nodo != NULL) {
            nuevo_nodo->e = nuevo_elector;
            int indice = hashing((int)nuevo_elector.dni, M_RS);

            nuevo_nodo->siguiente = rs->tabla[indice];
            rs->tabla[indice] = nuevo_nodo;
            rs->cantidad++;
            *exito = 1;
        } else {
            *exito = 0; //Fracaso, sin memoria
        }
    }
}

void BajaRS(RS *rs, elector elector_baja, int *exito) {
    NodoRS *posicion = NULL;
    NodoRS *anterior = NULL;
    float costo_localizar = 0.0;

    LocalizarRS(rs, elector_baja.dni, &posicion, &anterior, exito, &costo_localizar);

    if (*exito == 1 && CompararNuplas(posicion->e, elector_baja) == 1) {
        if (anterior == NULL) {
            int indice = hashing((int)elector_baja.dni, M_RS);
            rs->tabla[indice] = posicion->siguiente;
        } else {
            anterior->siguiente = posicion->siguiente;
        }

        free(posicion);
        rs->cantidad--;
        *exito = 1;
    } else {
        *exito = 0;
    }
}

void EvocacionRS(RS *rs, long dni_buscado, elector *elector_recuperado,
                 int *exito, Estadisticas *stats) {
    NodoRS *posicion = NULL;
    NodoRS *anterior = NULL;
    float costo_localizar = 0.0;

    LocalizarRS(rs, dni_buscado, &posicion, &anterior, exito, &costo_localizar);

    if (*exito == 1) {
        *elector_recuperado = posicion->e;
        RegistrarCosto(&(stats->evocar_exito), costo_localizar);
    } else {
        RegistrarCosto(&(stats->evocar_fracaso), costo_localizar);
    }
}

//FUNCIONES DE MOSTRAR

void LimpiarBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void PausarPagina(void) {
    printf("\n-- Presione Enter para continuar --");
    fflush(stdout);
    LimpiarBufferEntrada();
}

void MostrarRS(RS *rs) {
    printf("\n========================================================================================\n");
    printf("                              ESTRUCTURA RS (M = %d)\n", M_RS);
    printf("========================================================================================\n");

    int lineas_impresas = 0;

    for (int i = 0; i < M_RS; i++) {
        printf("[%04d] ", i); // Imprime el índice de la tabla hash

        NodoRS *actual = rs->tabla[i];

        // Verificar si la lista está vacía o contiene elementos
        if (actual == NULL) {
            printf("LISTA VACIA\n");
        } else {
            printf("Elementos en la lista:\n");

            while (actual != NULL) {
                elector e = actual->e;
                printf("       -> DNI: %ld | Nombre: %s | Domicilio: %s | CP: %d | Mesa: %d | Circuito: %d\n",
                       e.dni,
                       e.nombreApellido,
                       e.domicilio,
                       e.codigoPostal,
                       e.numeroMesa,
                       e.circuito);

                actual = actual->siguiente;
            }
        }

        lineas_impresas++;

        if (lineas_impresas % 15 == 0) {
            printf("----------------------------------------------------------------------------------------\n");
            printf("Mostrando %d de %d listas. Presione ENTER para ver las siguientes...", i + 1, M_RS);

            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    printf("\n========================================================================================\n");
    printf("Total de electores cargados en RS: %d\n", rs->cantidad);
    printf("========================================================================================\n");
}


void MostrarRAL(RAL *ral) {
    printf("\n========================================================================================\n");
    printf("                              ESTRUCTURA RAL (M = %d)\n", M_RAL);
    printf("========================================================================================\n");

    int lineas_impresas = 0;


    for (int i = 0; i < M_RAL; i++) {
        int estado = ral->electores[i].estado;

        printf("[%04d] ", i);

        if (estado == 0) {
            printf("NUNCA USADA (Virgen)\n");
        }
        else if (estado == 2) {
            printf("LIBRE (Dada de baja)\n");
        }
        else if (estado == 1) {
            elector e = ral->electores[i];
            printf("OCUPADA -> DNI: %ld | Nombre: %s | Domicilio: %s | CP: %d | Mesa: %d | Circuito: %d\n",
                   e.dni,
                   e.nombreApellido,
                   e.domicilio,
                   e.codigoPostal,
                   e.numeroMesa,
                   e.circuito);
        }

        lineas_impresas++;

        if (lineas_impresas % 15 == 0) {
            printf("----------------------------------------------------------------------------------------\n");
            printf("Mostrando %d de %d posiciones. Presione ENTER ", i + 1, M_RAL);

            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    printf("\n========================================================================================\n");
    printf("Total de electores cargados en RAL: %d\n", ral->cantidad);
    printf("========================================================================================\n");
}

void MostrarRAC(RAC *rac) {
    printf("\n========================================================================================\n");
    printf("                              ESTRUCTURA RAL (M = %d)\n", M_RAC);
    printf("========================================================================================\n");

    int lineas_impresas = 0;


    for (int i = 0; i < M_RAC; i++) {
        int estado = rac->electores[i].estado;

        printf("[%04d] ", i);

        if (estado == 0) {

            printf("NUNCA USADA (Virgen)\n");
        }
        else if (estado == 2) {

            printf("LIBRE (Dada de baja)\n");
        }
        else if (estado == 1) {

            elector e = rac->electores[i];
            printf("OCUPADA -> DNI: %ld | Nombre: %s | Domicilio: %s | CP: %d | Mesa: %d | Circuito: %d\n",
                   e.dni,
                   e.nombreApellido,
                   e.domicilio,
                   e.codigoPostal,
                   e.numeroMesa,
                   e.circuito);
        }

        lineas_impresas++;

        if (lineas_impresas % 15 == 0) {
            printf("----------------------------------------------------------------------------------------\n");
            printf("Mostrando %d de %d posiciones. Presione ENTER para ver las siguientes 15...", i + 1, M_RAL);

            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    printf("\n========================================================================================\n");
    printf("Total de electores cargados en RAC: %d\n", rac->cantidad);
    printf("========================================================================================\n");
}
/* ==========================================================================
   MEMORIZACIÓN DESDE ARCHIVO
   ========================================================================== */

int memorizarDesdeArchivo(RS *rs, RAL *ral, RAC *rac, Estadisticas *statsRS, Estadisticas *statsRAL, Estadisticas *statsRAC){
    //Liberar datos previos (si los hay) y dejar RS en estado limpio.
    DestruirRS(rs);
    InitRAL(ral);
    InitRAC(rac);

    elector eTemp;
    int exitoRS, exitoRAL, exitoRAC;
    float costoRAL, costoRAC;

    FILE *archivo = fopen("Operaciones_Padron.txt", "r");
    char buffer[150];

    InicializarMetricas(&statsRS->evocar_exito);
    InicializarMetricas(&statsRS->evocar_fracaso);

    InicializarMetricas(&statsRAL->evocar_exito);
    InicializarMetricas(&statsRAL->evocar_fracaso);


    InicializarMetricas(&statsRAC->evocar_exito);
    InicializarMetricas(&statsRAC->evocar_fracaso);

    if (archivo == NULL) {
        return 0;
    }

    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {
        int codigoOp = atoi(buffer);

        if(codigoOp == 1 || codigoOp == 2){ // Alta o baja
            if (fgets(buffer, sizeof(buffer), archivo) == NULL) break;
            eTemp.dni = atol(buffer);

            fgets(buffer, sizeof(buffer), archivo);
            buffer[strcspn(buffer, "\r\n")] = 0;
            strcpy(eTemp.nombreApellido, buffer);

            fgets(buffer, sizeof(buffer), archivo);
            buffer[strcspn(buffer, "\r\n")] = 0;
            strcpy(eTemp.domicilio, buffer);

            fgets(buffer, sizeof(buffer), archivo);
            eTemp.codigoPostal = atoi(buffer);

            fgets(buffer, sizeof(buffer), archivo);
            eTemp.numeroMesa = atoi(buffer);

            fgets(buffer, sizeof(buffer), archivo);
            eTemp.circuito = atoi(buffer);

            eTemp.estado = 1;

            if(codigoOp == 1){
                // 1. ALTA RS
                AltaRS(rs, eTemp, &exitoRS);

                // 2. ALTA RAL
                AltaRAL(ral, eTemp, &exitoRAL);

                // 3. ALTA RAC
                AltaRAC(rac, eTemp, &exitoRAC);

            } else {
                // 1. BAJA RS
                BajaRS(rs, eTemp, &exitoRS);

                // 2. BAJA RAL
                BajaRAL(ral, eTemp, &exitoRAL);

                // 3. BAJA RAC
                BajaRAC(rac, eTemp, &exitoRAC);
            }

        } else if(codigoOp == 3){ // Evocación
            if (fgets(buffer, sizeof(buffer), archivo) == NULL) break;
            long dniEvocar = atol(buffer);

            // 1. EVOCACION RS
            EvocacionRS(rs, dniEvocar, &eTemp, &exitoRS, statsRS);

            // 2. EVOCACION RAL
            EvocacionRAL(ral, dniEvocar, &eTemp, &exitoRAL, &costoRAL);
            if (exitoRAL == 1) {
                RegistrarCosto(&statsRAL->evocar_exito, costoRAL);
            } else {
                RegistrarCosto(&statsRAL->evocar_fracaso, costoRAL);
            }

            // 3. EVOCACION RAC
            EvocacionRAC(rac, dniEvocar, &eTemp, &exitoRAC, &costoRAC);
            if (exitoRAC == 1) {
                RegistrarCosto(&statsRAC->evocar_exito, costoRAC);
            } else {
                RegistrarCosto(&statsRAC->evocar_fracaso, costoRAC);
            }
        }
    }

    fclose(archivo);
    return 1;
}

void LimpiarPantalla() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main() {
    RS miRS;
    RAL miRAL;
    RAC miRAC;

    Estadisticas statsRS, statsRAL, statsRAC;

    InitRS(&miRS);
    InitRAL(&miRAL);
    InitRAC(&miRAC);

    int opcion;
    int estructurasCargadas = 0;

    do {
        LimpiarPantalla();
        printf("\n======================================================\n");
        printf("               MENU PRINCIPAL - HASHING             \n");
        printf("======================================================\n");
        printf(" 1. Comparacion de Estructuras (Procesar Archivo)\n");
        printf(" 2. Mostrar Estructura\n");
        printf(" 3. Salir\n");
        printf("======================================================\n");
        printf("Ingrese una opcion: ");
        scanf("%d", &opcion);
        LimpiarPantalla();

        switch(opcion) {
            case 1: {


                printf("\nIniciando procesamiento del archivo de operaciones...\n");

                int cargaExitosa = memorizarDesdeArchivo(&miRS, &miRAL, &miRAC,
                                                         &statsRS, &statsRAL, &statsRAC);

                if (cargaExitosa == 0) {
                    printf("Se aborto la ejecucion: no se pudo abrir el archivo.\n");
                } else {
                    estructurasCargadas = 1;
                    printf("Procesamiento exitoso.\n");

                    printf("\n-----------------------------|--------------|--------------|--------------|\n");
                    printf("                             |      RS      |      RAL     |      RAC     |\n");
                    printf("-----------------------------|--------------|--------------|--------------|\n");

                    /*
                    // --- ALTA ---
                    printf("Alta                         |              |              |              |\n");
                    printf("  Cantidad                   | %12d | %12d | %12d |\n", statsRS.alta.cantidad, statsRAL.alta.cantidad, statsRAC.alta.cantidad);
                    printf("  Costo Acumulado            | %12.3f | %12.3f | %12.3f |\n", statsRS.alta.costo_acu, statsRAL.alta.costo_acu, statsRAC.alta.costo_acu);
                    printf("  Costo Maximo               | %12.3f | %12.3f | %12.3f |\n", statsRS.alta.costo_max, statsRAL.alta.costo_max, statsRAC.alta.costo_max);
                    printf("  Costo Promedio             | %12.3f | %12.3f | %12.3f |\n",
                        statsRS.alta.cantidad > 0 ? statsRS.alta.costo_acu / statsRS.alta.cantidad : 0,
                        statsRAL.alta.cantidad > 0 ? statsRAL.alta.costo_acu / statsRAL.alta.cantidad : 0,
                        statsRAC.alta.cantidad > 0 ? statsRAC.alta.costo_acu / statsRAC.alta.cantidad : 0);
                    printf("-----------------------------|--------------|--------------|--------------|\n");

                    // --- BAJA ---
                    printf("Baja                         |              |              |              |\n");
                    printf("  Cantidad                   | %12d | %12d | %12d |\n", statsRS.baja.cantidad, statsRAL.baja.cantidad, statsRAC.baja.cantidad);
                    printf("  Costo Acumulado            | %12.3f | %12.3f | %12.3f |\n", statsRS.baja.costo_acu, statsRAL.baja.costo_acu, statsRAC.baja.costo_acu);
                    printf("  Costo Maximo               | %12.3f | %12.3f | %12.3f |\n", statsRS.baja.costo_max, statsRAL.baja.costo_max, statsRAC.baja.costo_max);
                    printf("  Costo Promedio             | %12.3f | %12.3f | %12.3f |\n",
                        statsRS.baja.cantidad > 0 ? statsRS.baja.costo_acu / statsRS.baja.cantidad : 0,
                        statsRAL.baja.cantidad > 0 ? statsRAL.baja.costo_acu / statsRAL.baja.cantidad : 0,
                        statsRAC.baja.cantidad > 0 ? statsRAC.baja.costo_acu / statsRAC.baja.cantidad : 0);
                    printf("-----------------------------|--------------|--------------|--------------|\n");
                    */

                    // --- EVOCAR EXITOSO ---
                    printf("Evocar exitoso               |              |              |              |\n");
                    printf("  Cantidad                   | %12d | %12d | %12d |\n", statsRS.evocar_exito.cantidad, statsRAL.evocar_exito.cantidad, statsRAC.evocar_exito.cantidad);
                    printf("  Costo Acumulado            | %12.3f | %12.3f | %12.3f |\n", statsRS.evocar_exito.costo_acu, statsRAL.evocar_exito.costo_acu, statsRAC.evocar_exito.costo_acu);
                    printf("  Costo Maximo               | %12.3f | %12.3f | %12.3f |\n", statsRS.evocar_exito.costo_max, statsRAL.evocar_exito.costo_max, statsRAC.evocar_exito.costo_max);
                    printf("  Costo Promedio             | %12.3f | %12.3f | %12.3f |\n",
                        statsRS.evocar_exito.cantidad > 0 ? statsRS.evocar_exito.costo_acu / statsRS.evocar_exito.cantidad : 0,
                        statsRAL.evocar_exito.cantidad > 0 ? statsRAL.evocar_exito.costo_acu / statsRAL.evocar_exito.cantidad : 0,
                        statsRAC.evocar_exito.cantidad > 0 ? statsRAC.evocar_exito.costo_acu / statsRAC.evocar_exito.cantidad : 0);
                    printf("-----------------------------|--------------|--------------|--------------|\n");

                    // --- EVOCAR FRACASO ---
                    printf("Evocar fracaso               |              |--------------|--------------|\n");
                    printf("  Cantidad                   | %12d | %12d | %12d |\n", statsRS.evocar_fracaso.cantidad, statsRAL.evocar_fracaso.cantidad, statsRAC.evocar_fracaso.cantidad);
                    printf("  Costo Acumulado            | %12.3f | %12.3f | %12.3f |\n", statsRS.evocar_fracaso.costo_acu, statsRAL.evocar_fracaso.costo_acu, statsRAC.evocar_fracaso.costo_acu);
                    printf("  Costo Maximo               | %12.3f | %12.3f | %12.3f |\n", statsRS.evocar_fracaso.costo_max, statsRAL.evocar_fracaso.costo_max, statsRAC.evocar_fracaso.costo_max);
                    printf("  Costo Promedio             | %12.3f | %12.3f | %12.3f |\n",
                        statsRS.evocar_fracaso.cantidad > 0 ? statsRS.evocar_fracaso.costo_acu / statsRS.evocar_fracaso.cantidad : 0,
                        statsRAL.evocar_fracaso.cantidad > 0 ? statsRAL.evocar_fracaso.costo_acu / statsRAL.evocar_fracaso.cantidad : 0,
                        statsRAC.evocar_fracaso.cantidad > 0 ? statsRAC.evocar_fracaso.costo_acu / statsRAC.evocar_fracaso.cantidad : 0);
                    printf("-----------------------------|--------------|--------------|--------------|\n");

                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    getchar();
                }
                break;
            }

            case 2:
                if (estructurasCargadas == 0) {
                    printf("\nError: Debe ejecutar la 'Comparacion de Estructuras' (Opcion 1) primero para cargar los datos.\n");
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    getchar();
                } else {
                    int subOpcion;
                    printf("\n--- MOSTRAR ESTRUCTURA ---\n");
                    printf("1. Rebalse Separado (RS)\n");
                    printf("2. Rebalse Abierto Lineal (RAL)\n");
                    printf("3. Rebalse Abierto Cuadratico (RAC)\n");
                    printf("Elija estructura a mostrar: ");
                    scanf("%d", &subOpcion);

                    if (subOpcion == 1) {
                        MostrarRS(&miRS);
                    } else if (subOpcion == 2) {
                        MostrarRAL(&miRAL);
                        int c; while ((c = getchar()) != '\n' && c != EOF); getchar();
                    } else if (subOpcion == 3) {
                        MostrarRAC(&miRAC);
                        int c; while ((c = getchar()) != '\n' && c != EOF); getchar();
                    }
                }
                break;

            case 3:
                printf("\nFinalizando programa. Cuidate Masterr\n");
                break;

            default:
                printf("\nOpcion no valida. Intente nuevamente.\n");
        }

    } while(opcion != 3);

    //Liberar toda la memoria dinamica del RS antes de salir.
    DestruirRS(&miRS);

    return 0;
}
