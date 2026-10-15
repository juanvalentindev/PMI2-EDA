///LIBRERIAS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
///CONSTANTES.

#define ELECTORES_ESPERADOS 2000

//RS
// Tamaño de la tabla RS (N = 2000, rho = 1.61 -> M = 2000 / 1.61 = 1242.23)
#define M_RS 1243

//RAL
#define M_RAL 2531 //(Para el RAL se tendra  ρ= 0.79.)


//Definicion de struc's Patron
typedef struct {
    long dni;                 // El D.N.I. es un entero (usamos long por el tamaño del número)
    char nombreApellido[50];  // Secuencia de hasta 50 caracteres
    char domicilio[80];       // Secuencia de hasta 80 caracteres
    int codigoPostal;         // Es un entero
    int numeroMesa;           // Es un entero
    int circuito;             // Es un entero
    int estado;              //0: Virgen 1: Ocupado 2: Libre. (Ver nota en RAL).
} elector;

typedef struct {
    int cantidad;
    float costo_acu;
    float costo_max;
} MetricasOp;

typedef struct {
    MetricasOp alta;
    MetricasOp baja;
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
    char x[10];
    int longitud, i;
    int contador = 0;

    sprintf(x, "%d", dni);
    longitud = strlen(x);
    for (i = 0; i < longitud; i++) {
        contador += ((int)x[i]) * (i + 1);
    }
    return (contador % M);
}


//Funciones de Comparación para confirmación de BAJA

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

//REBALSE ABIERTO LINEAL (RAL)
//(Para el RAL se tendra  ρ= 0.79.)

/*    Codificación de Estados para las celdas
    0: Virgen.
    1: Ocupada
    2: Libre.
    DECISIÓN DE IMPLEMENTACIÓN DE CODIFICACIÓN
    Wanzo, voy a implementarlo en el struct elector, donde cada elector va tener un parametro llamado
    Estado, porque si hago por dni, que pasa si me dan de alta el dni 0?, el proyecto no menciona restricción
    alguna al respecto de entradas de DNI.
*/

typedef struct{
    elector electores[M_RAL];
    int cantidad;
}RAL;

void InitRAL(RAL *ral){

    (*ral).cantidad=0;

    for(int i = 0; i < ELECTORES_ESPERADOS;i++){
        ral->electores[i].estado = 0;
        ral->electores[i].dni = 0;
        ral->electores[i].codigoPostal = 0;
        ral->electores[i].numeroMesa = 0;
        ral->electores[i].circuito = 0;
        ral->electores[i].nombreApellido[0] = '\0';
        ral->electores[i].domicilio[0] = '\0';
    }
}


void LocalizarRAL(RAL *ral,long dni_buscado,int *pos,int *exito,int *costo){

    int actual = hashing((int)dni_buscado,M_RAL);
    int primeraLibre = -1;
    int celdasRecorridas = 0;

    *exito = 0;
    *costo = 0;

    while(celdasRecorridas < M_RAL){
        (*costo)++;

        int estado = ral->electores[actual].estado;

        switch(estado){
            case 0:{
                if(primeraLibre != -1){
                    *pos = primeraLibre;
                }else{
                    *pos = actual;
                }
                return;
            }

            case 1:{
                if(ral->electores[actual].dni == dni_buscado){
                    *pos = actual;
                    *exito = 1;
                    return;
                }
            }

            case 2:{
                //Se guarda la primera libre, entonces si es distinto de -1, no encontro una libre anteriormente.
                if (primeraLibre == -1){
                    primeraLibre = actual;
                }

            }
        }

        //Recirculación Lineal
        actual = (actual + 1) % M_RAL;
        celdasRecorridas++; //Aumentamos el contador para que siga recorriendo

    }

    *pos = (primeraLibre != -1) ? primeraLibre : actual; //Me corrigio esta linea gemini, no se porque ponerla.


}

void AltaRAL(RAL *ral, elector nuevoElector, float *costo,int *exito){

    int encontrado;
    int costoPrima = 0; //Valor descartable
    int exitoPrima = 0;
    int pos;


    LocalizarRAL(ral,nuevoElector.dni,&pos,&costoPrima,&exitoPrima);

    if(exitoPrima == 0){
        nuevoElector.estado = 1;
        ral->electores[pos] = nuevoElector;
        ral->cantidad++;
        *costo = (float)costoPrima;
         *exito = 1;
        return;
    }else{
        *exito = 0;
        return;
    }
}

void BajaRal(RAL *ral,elector nuplaBaja,int *exito,float *costo){

    int pos,costoPrima,exitoPrima;
    LocalizarRAL(ral,nuplaBaja.dni,&pos,&costoPrima,&exitoPrima);



    if(exitoPrima != 0){
        int iguales = CompararNuplas(nuplaBaja,ral->electores[pos]);

        if(iguales != 0){
            ral->electores[pos].estado = 2;
            ral->cantidad--;
            *costo = (float)costoPrima;
            *exito = 1;
            return;
        }else{
            *exito = 0;
            return;
        }
    }else{
        *exito = 0;
        return;
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
        // Evocación fracasada: el DNI no está en la tabla
        *exito = 0;
    }
}
/*

 REBALSE ABIERTO CUADRATICO (RAL)
(Para el RAC se tendra  ρ= 0.83.)

    Codificación de Estados para las celdas
    0: Virgen.
    1: Ocupada
    2: Libre.
*/

typedef struct{
    elector electores[M_RAC];
    int cantidad;
} RAC;

void InitRAC(RAC *rac){

    (*rac).cantidad = 0;
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

    int actual = hashing((int)dni_buscado, M_RAC);
    int primeraLibre = -1;
    int celdasRecorridas = 0;

    *exito = 0;
    *costo = 0;

    while(celdasRecorridas < M_RAC){
        (*costo)++;

        int estado = rac->electores[actual].estado;

        switch(estado){
            case 0:{
                if(primeraLibre != -1){
                    *pos = primeraLibre;
                }else{
                    *pos = actual;
                }
                return;
            }

            case 1:{
                if(rac->electores[actual].dni == dni_buscado){
                    *pos = actual;
                    *exito = 1;
                    return;
                }
                break;
            }

            case 2:{
                //Se guarda la primera libre, entonces si es distinto de -1, no encontro una libre anteriormente.
                if (primeraLibre == -1){
                    primeraLibre = actual;
                }
                break;
            }
        }

        actual = (actual + 1) % M_RAC;
        celdasRecorridas++; //Aumentamos el contador para que siga recorriendo

    }

    *pos = (primeraLibre != -1) ? primeraLibre : actual;
}

void AltaRAC(RAC *rac, elector nuevoElector, float *costo, int *exito){

    int costoPrima = 0; //Valor descartable
    int exitoPrima = 0;
    int pos;
    LocalizarRAC(rac, nuevoElector.dni, &pos, &costoPrima, &exitoPrima);
    if(exitoPrima == 0){
        nuevoElector.estado = 1;
        rac->electores[pos] = nuevoElector;
        rac->cantidad++;
        *costo = (float)costoPrima;
        *exito = 1;
        return;
    }else{
        *exito = 0;
        return;
    }
}

void BajaRAC(RAC *rac, elector nuplaBaja, int *exito, float *costo){
    int pos, costoPrima, exitoPrima;
    LocalizarRAC(rac, nuplaBaja.dni, &pos, &costoPrima, &exitoPrima);

    if(exitoPrima != 0){
        int iguales = CompararNuplas(nuplaBaja, rac->electores[pos]);

        if(iguales != 0){
            rac->electores[pos].estado = 2;
            rac->cantidad--;
            *costo = (float)costoPrima;
            *exito = 1;
            return;
        }else{
            *exito = 0;
            return;
        }
    }else{
        *exito = 0;
        return;
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
        // Evocación fracasada: el DNI no está en la tabla
        *exito = 0;
    }
}


//Rebalse Separado

typedef struct NodoRS {
    elector e;                 // El balde con una ranura
    struct NodoRS *siguiente;  // Puntero al siguiente nodo
} NodoRS;

typedef struct {
    NodoRS *tabla[M_RS];       // Arreglo de punteros a los nodos
    int cantidad;
} RS;

void InitRS(RS *rs) {
    rs->cantidad = 0;

    // Inicializacion de todas las cabeceras apuntando a NULL
    for(int i = 0; i < M_RS; i++) {
        rs->tabla[i] = NULL;
    }
}

int LocalizarRS(RS *rs, long dni_buscado, NodoRS **posicion, float *costo) {
    // Calculo del indice con la funcion de hashing
    int indice = hashing(dni_buscado, M_RS);

    // Inicio de la búsqueda en la posición dada por h(x)
    NodoRS *p = rs->tabla[indice];
    *costo = 0.0;

    while (p != NULL && p->e.dni != dni_buscado) {
        (*costo)++;       // Contamos cada nodo visitado
        p = p->siguiente;
    }

    if (p != NULL) {
        // Salimos porque p->e.dni == dni_buscado
        (*costo)++;    // Contamos la comparacion que dio exito (PREGUNTAR AL PROFE)
        *posicion = p; // Guardamos la direccion del nodo encontrado
        return 1;      // Exito papa
    } else {
        // Salimos porque p llego a NULL (no se encontro)
        *posicion = NULL;
        return 0;      // Fracaso
    }
}

int AltaRS(RS *rs, elector nuevo_elector, Estadisticas *stats) {
    NodoRS *posicion = NULL;
    float costo_localizar = 0.0;

    // Localizar
    int existe = LocalizarRS(rs, nuevo_elector.dni, &posicion, &costo_localizar);

    if (existe == 1) {
        // Fracasa
        return 0;
    } else {
        // Verificacion del espacio en memoria
        NodoRS *nuevo_nodo = (NodoRS*)malloc(sizeof(NodoRS));

        if (nuevo_nodo != NULL) {
            // Modificacion en la estructura
            nuevo_nodo->e = nuevo_elector;
            // Calculamos el indice
            int indice = hashing(nuevo_elector.dni, M_RS);

            // Insertamos el nodo al PRINCIPIO de la LVD
            nuevo_nodo->siguiente = rs->tabla[indice];
            rs->tabla[indice] = nuevo_nodo;

            // Aumentamos el contador total de electores en la tabla
            rs->cantidad++;

            RegistrarCosto(&(stats->alta), costo_localizar + 1.0);

            return 1; // Exito
        } else {
            // Fracasa
            return 0;
        }
    }
}

int BajaRS(RS *rs, elector elector_baja, Estadisticas *stats) {
    NodoRS *posicion = NULL;
    float costo_localizar = 0.0;

    int existe = LocalizarRS(rs, elector_baja.dni, &posicion, &costo_localizar);

    // Confirmo la nupla completa
    if (existe == 1 && CompararNuplas(posicion->e, elector_baja) == 1) {

        // Modificacion estructural
        int indice = hashing(elector_baja.dni, M_RS);
        NodoRS *actual = rs->tabla[indice];
        NodoRS *anterior = NULL;

        // Como necesitamos el nodo 'anterior' para desenlazar,
        // avanzamos hasta llegar a la posición que nos dio el Localizar
        while (actual != posicion) {
            anterior = actual;
            actual = actual->siguiente;
        }

        // Vemos el nodo
        if (anterior == NULL) {
            // Era el primero de la lista
            rs->tabla[indice] = actual->siguiente;
        } else {
            // Estaba en el medio o al final
            anterior->siguiente = actual->siguiente;
        }

        free(actual);   // Liberamos memoria
        rs->cantidad--; // Actualizamos contador
        RegistrarCosto(&(stats->baja), costo_localizar);
        return 1; // Exito

    } else {
        RegistrarCosto(&(stats->baja), costo_localizar);
        return 0; // Fracasa
    }
}

int EvocacionRS(RS *rs, long dni_buscado, elector *elector_recuperado, Estadisticas *stats) {
    NodoRS *posicion = NULL;
    float costo_localizar = 0.0;

    int existe = LocalizarRS(rs, dni_buscado, &posicion, &costo_localizar);

    if (existe == 1) {
        // Copiamos los datos
        *elector_recuperado = posicion->e;

        RegistrarCosto(&(stats->evocar_exito), costo_localizar);
        return 1;
    } else {
        RegistrarCosto(&(stats->evocar_fracaso), costo_localizar);
        return 0; //Fracasa
    }
}

//FUNCIONES PARA IMPRIMIR!

void ProcesarArchivo(RS *rs, Estadisticas *stats_rs) {
    FILE *archivo = fopen("Operaciones Padron.txt", "r");
    if (archivo == NULL) {
        printf("Error: No se pudo abrir 'Operaciones Padron.txt'.\n");
        return;
    }

    int operacion;
    // Leer hasta el fin de archivo
    while (fscanf(archivo, "%d\n", &operacion) == 1) {
        elector e;

        if (operacion == 1 || operacion == 2) {
            // Alta o Baja: Leemos la nupla completa
            fscanf(archivo, "%ld\n", &e.dni);

            fgets(e.nombreApellido, 50, archivo);
            e.nombreApellido[strcspn(e.nombreApellido, "\n")] = 0; // Eliminar salto de línea

            fgets(e.domicilio, 80, archivo);
            e.domicilio[strcspn(e.domicilio, "\n")] = 0; // Eliminar salto de línea

            fscanf(archivo, "%d\n", &e.codigoPostal);
            fscanf(archivo, "%d\n", &e.numeroMesa);
            fscanf(archivo, "%d\n", &e.circuito);
            e.estado = 1; // 1: Ocupado

            if (operacion == 1) {
                AltaRS(rs, e, stats_rs);
            } else if (operacion == 2) {
                BajaRS(rs, e, stats_rs);
            }

        } else if (operacion == 3) {
            // Evocación: Solo leemos el DNI
            long dni_buscado;
            elector recuperado;
            fscanf(archivo, "%ld\n", &dni_buscado);

            EvocacionRS(rs, dni_buscado, &recuperado, stats_rs);
        }
    }

    fclose(archivo);
    printf("Archivo procesado con exito.\n");
}

// Funcion auxiliar interna para imprimir una fila de metricas
void ImprimirBloqueMetrica(const char* nombre_op, MetricasOp m_rs) {
    float prom_rs = (m_rs.cantidad > 0) ? (m_rs.costo_acu / m_rs.cantidad) : 0.0;

    // Vacio para que venga el RAC y RAL
    printf("%s\n", nombre_op);
    printf("  %-16s | %15s | %15s | %15d |\n", "Cantidad", "-", "-", m_rs.cantidad);
    printf("  %-16s | %15s | %15s | %15.3f |\n", "Costo Acumulado", "-", "-", m_rs.costo_acu);
    printf("  %-16s | %15s | %15s | %15.3f |\n", "Costo Maximo", "-", "-", m_rs.costo_max);
    printf("  %-16s | %15s | %15s | %15.3f |\n", "Costo Promedio", "-", "-", prom_rs);
    printf("-------------------|-----------------|-----------------|-----------------|\n");
}

void MostrarTablaComparativa(Estadisticas stats_rs) {
    printf("\n");
    printf("-------------------|-----------------|-----------------|-----------------|\n");
    printf("                   |       RAL       |       RAC       |        RS       |\n");
    printf("-------------------|-----------------|-----------------|-----------------|\n");

    ImprimirBloqueMetrica("Alta", stats_rs.alta);
    ImprimirBloqueMetrica("Baja", stats_rs.baja);
    ImprimirBloqueMetrica("Evocar exitoso", stats_rs.evocar_exito);
    ImprimirBloqueMetrica("Evocar fracaso", stats_rs.evocar_fracaso);
}

int main() {
    // 1. Declaración e inicialización
    RS rebalse_separado;
    InitRS(&rebalse_separado);

    Estadisticas estadisticas_rs;
    InicializarMetricas(&estadisticas_rs.alta);
    InicializarMetricas(&estadisticas_rs.baja);
    InicializarMetricas(&estadisticas_rs.evocar_exito);
    InicializarMetricas(&estadisticas_rs.evocar_fracaso);

    // 2. Procesamiento masivo (Opción del menú)
    ProcesarArchivo(&rebalse_separado, &estadisticas_rs);

    // 3. Mostrar tabla (Opción del menú)
    MostrarTablaComparativa(estadisticas_rs);

    return 0;
}
