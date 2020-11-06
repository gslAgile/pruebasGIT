/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * Libreria monitor.hpp/.cpp (archivo cabecera - header file).
 * 
 * 
 * Esta libreria contiene las declarciones y definiciones de la clase monitor para para la proteccion de recursos compartidos de la concurrencia de 
 * subprocesos de espacio usuario.
 * 
 * Las protecciones de los recursos compartidos se logran brindando exclusion mutua a los subprocesos con el uso de mutex. Por otro lado el control de
 * subprocesos se realiza mediante el uso de variables de condion (condition_variable) estructuras brindadas por la libreria estandar de C++.
 *
 * Recurso compartido a proteger: Redes de petri Generalizadas. Clase RDPG_Driver desde libreria libMatrixmodG.hpp/cpp.
 * 
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef MONITOR_ULIB_H
#define MONITOR_ULIB_H /* ULIB: por biblioteca de espacio usuario. */


#include "libMatrixmodG.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <omp.h>
#include <time.h>

using namespace std;


#define		N_TASKS		1000


class monitor {

private:

    mutex mtx_monitor;					/* Mutex que gestiona cola de procesos que ingresan al monitor. */
    vector <condition_variable> vCV;	/* Vector de todas las variables de condicion. */
    RDPG_Driver red;					/* Recurso compartido protegido por monitor al multiprocesamiento (se garantiza exclusion mutua). */
    double mtimeop_omp; 				/* Tiempo de operacion desde API omp. */
	double mtimeop_ltime; 				/* Tiempo de operacion desde libreria time. */


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * DECLARACION DE METODOS DE Objeto monitor
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/

public:

	/* Constructores */
	monitor(string, size_t, size_t);
	monitor(string, string, string, string, string, string);

	/* Destructor */

	/* Getters*/
	size_t get_transiciones() const { return red.get_transiciones(); }
	double get_mtimeop_omp() const { return mtimeop_omp; }
	double get_mtimeop_ltime() const { return mtimeop_ltime; }
	int get_TokensPlace(size_t);
	void set_mtimeop_omp(double p_time) { mtimeop_omp = p_time; }
	void set_mtimeop_ltime(double p_time) { mtimeop_ltime = p_time; }
	void set_kernel_RDPG(bool val) { red.set_kernel_RDPG_sec(val);}


	/* Metodos de monitor con proteccion de recurso RDPG. */
	void generar_tarea();
	void ON_OFF_CPU1();
	void ON_OFF_CPU2();
	void procesar_tarea_CPU1();
	void procesar_tarea_CPU2();
	void finalizar_tarea_CPU1();
	void finalizar_tarea_CPU2();
	bool empty();
	void print_vcomp(int);
	void import_RDPG(string, string, string, string, string);
	int shoot_RDPG(int);
	void print_allComp();
	void print_RDPGinfo();
	void ashoot_RDPG(int, unique_lock<mutex>&);
	void deleteRDPG();
	int notify_next_shoot(int);
	void notify_thread(int);

	/**/
	void generar_tarea_m();
	void encender_CPU1_m();
	void encender_CPU2_m();
	void procesar_tarea_CPU1_m();
	void procesar_tarea_CPU2_m();
	void finalizar_tarea_CPU1_m();
	void finalizar_tarea_CPU2_m();
	void apagar_CPU1_m();
	void apagar_CPU2_m();
};


#endif /* MONITOR_ULIB_H */