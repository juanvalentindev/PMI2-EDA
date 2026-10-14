///LIBRERIAS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
///CONSTANTES.

#define ELECTORES_ESPERADOS 2000

//RAL
#define M_RAL 2531


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
    elector electores[ELECTORES_ESPERADOS];
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
                if(primera_libre != -1){
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

    *pos = (primera_libre != -1) ? primera_libre : actual; //Me corrigio esta linea gemini, no se porque ponerla.


}

void AltaRAL(RAL *ral, elector nuevoElector, float *costo,int *exito){

    int encontrado;
    int costoPrima = 0; //Valor descartable
    int exitoPrima = 0;
    int pos;

    LocalizarRAL(ral,nuevoElector->dni,&pos,&costoPrima,&exitoPrima);

    if(exitoPrima == 0){
        nuevoElector.estado = 1;
        ral->electores[pos] = nuevoElector;
        ral->cantidad++;
         *exito = 1;
        return;
    }else{
        *exito = 0;
        return;
    }
}

int main()
{
    printf("Hello world!\n");
    return 0;
}
