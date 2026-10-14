///LIBRERIAS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
///CONSTANTES.

#define ELECTORES_ESPERADOS 2000

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
    float costo_acu;s
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

/*

 REBALSE ABIERTO LINEAL (RAL)
(Para el RAL se tendra  ρ= 0.79.)

    Codificación de Estados para las celdas
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

int main()
{
    printf("Hello world!\n");
    return 0;
}
