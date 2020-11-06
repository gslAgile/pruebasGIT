#ifndef MONITORCA2_ULIB_H
#define MONITORCA2_ULIB_H /* ULIB: por biblioteca de espacio usuario. */


#include "libMatrixmodG.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <omp.h>
#include <time.h>

using namespace std;


#define		N_OPERACIONES		1000


class monitorCA2 {

private:

    mutex mtx_monitor;					/* Mutex que gestiona cola de procesos que ingresan al monitor. */
    vector <condition_variable> vCV;	/* Vector de todas las variables de condicion. */
    RDPG_Driver red;					/* Recurso compartido protegido por monitor al multiprocesamiento (se garantiza exclusion mutua). */
    double mtimeop_omp; 				/* Tiempo de operacion desde API omp. */
	double mtimeop_ltime; 				/* Tiempo de operacion desde libreria time. */


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * DECLARACION DE METODOS DE Objeto monitorCA2
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/

public:

	/* Constructores */
	monitorCA2(string, size_t, size_t);
	monitorCA2(string, string, string, string, string, string);

	/* Destructor */

	/* Getters*/
	size_t get_transiciones() const { return red.get_transiciones(); }
	double get_mtimeop_omp() const { return mtimeop_omp; }
	double get_mtimeop_ltime() const { return mtimeop_ltime; }
	int get_TokensPlace(size_t);
	void set_mtimeop_omp(double p_time) { mtimeop_omp = p_time; }
	void set_mtimeop_ltime(double p_time) { mtimeop_ltime = p_time; }


	/* Metodos de monitorCA2 con proteccion de recurso RDPG. */
	void producir();
	void consumir();
	bool empty();
	void print_vcomp(int);
	void import_RDPG(string, string, string, string, string);
	int shoot_RDPG(int, SHOT_MODE);
	void print_allComp();
	void print_RDPGinfo();
	void ashoot_RDPG(int, unique_lock<mutex>&);
	void deleteRDPG();
	int notify_next_shoot(int);
};


#endif /* MONITORCA2_ULIB_H */