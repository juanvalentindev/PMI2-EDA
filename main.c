///LIBRERIAS
#include <stdio.h>
#include <stdlib.h>
///CONSTANTES.

#define ELECTORES_ESPERADOS 2000


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


/*

 REBALSE ABIERTO LINEAL (RAL)
(Para el RAL se tendra  ρ= 0.79.)

    Codificación de Estados para las celdas
    0: Virgen.
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
    ral.cantidad->0;

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


int main()
{
    printf("Hello world!\n");
    return 0;
}
