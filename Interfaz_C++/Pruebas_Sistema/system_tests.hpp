/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 *  Libreria system_tests.hpp/.cpp (archivo cabecera - header file).
 *  
 *  Esta libreria contiene todas las pruebas de sistema que se realizan sobre la clase monitor en la gestion de su recurso compartido del tipo RDPG.
 *  
 *  Las pruebas realizadas sirven para la medicion del rendimiento que existe en la gestion de RDPG, recurso protegido por la clase monitor.
 *  Tambien se realizan pruebas relacionadas al multiprocesamiento analizando si la respuesta del monitor es correcta frente a las solicitudes realizadas.
 *  
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef SYSTEM_TESTS_H
#define SYSTEM_TESTS_H

#include "../monitor.hpp"
#include "../monitorCA2.hpp"

#define N_MUESTRAS	50

/* Variables globales de pruebas de sistema automatizadas*/ 
/* Muestras */
double m_alloc_omp[N_MUESTRAS]; /* muestras de opoen MP */
double m_alloc_time[N_MUESTRAS]; /* muestras de time */


RDPG_Driver *TestRed; /* monitor de pruebas (tests).*/


/* Declaracion de funciones*/
void st_iniciar_vmuestra(void);
void st_promedio(void);
void st_muestras(void);

void CPS201_alloc_mem_nxm(RDPG_Driver *p_rdp, const size_t p_nplazas, const size_t p_ntransiciones);
void CPS202_add_comps_10x10(RDPG_Driver *p_rdp);
void CPS202_add_comps_100x100(RDPG_Driver *p_rdp);
void CPS202_add_comps_500x500(RDPG_Driver *p_rdp);
void CPS202_add_comps_1000x1000(RDPG_Driver *p_rdp);
void CPS203_create_RDPG(RDPG_Driver *p_rdp);
void CPS204_delete_rdp_nxm(RDPG_Driver *p_rdp, const size_t p_nplazas, const size_t p_ntransiciones);
void CPS205_shoot(RDPG_Driver *p_rdp);
void CPS206_readAllComp(RDPG_Driver *p_rdp);
void CPS207_readRDPGinfo(RDPG_Driver *p_rdp);
void CPS208_CA_threads3();
void CPS208_CA_threads5();
void CPS209_CA2_threads2();


#endif
