/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * Libreria monitor.hpp/.cpp (archivo fuente - source file).
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

#include "monitor.hpp"


monitor::monitor(string p_name, size_t p_places, size_t p_transitions)
: red("RDPG Test", p_places, p_transitions), mtimeop_omp(0.0), mtimeop_ltime(0.0)
{
	/* Verificacion de cantidad de transiciones y de vector de variables de condicion. Una CV por cada transicion de la RDPG. */
	if(red.get_transiciones() < 1)
	{
		cout << " ERROR: No existen transiciones en la RDPG importada." << endl;
		return;
	}
}


monitor::monitor(string p_name, string p_mII, string p_mIH, string p_mIR, string p_mIRe, string p_vMI)
: red(p_name, p_mII, p_mIH, p_mIR, p_mIRe, p_vMI), vCV(red.get_lineElements(p_mII)), mtimeop_omp(0.0), mtimeop_ltime(0.0)
{
	/* Verificacion de cantidad de transiciones y de vector de variables de condicion. Una CV por cada transicion de la RDPG. */
	if(red.get_transiciones() < 1)
	{
		cout << " ERROR: No existen transiciones en la RDPG importada." << endl;
		return;
	}
} 




void monitor::generar_tarea()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */
    
    //red.print_vcomp(_vEx); 

    /* Analisis de Guardas. */
    if((red.get_TokensPlace(2) == -EC_datoIncorrecto) && (red.get_TokensPlace(8) == -EC_datoIncorrecto))
    {
    	/* Si no existen marcados de plazas indicadas, gestionar error.*/

    }
    else if(red.get_TokensPlace(2) <= red.get_TokensPlace(8))
    {
    	/* Manejo de politica de RDPG mediante configuracion de guardas. */
    	red.matrixmodG_set_vG(1,1); 	/* equivale a vG[1]=1; y update_vEx(); */
    	red.matrixmodG_set_vG(7,0);		/* equivale a vG[7]=0; y update_vEx(); */

    	ashoot_RDPG(0, l);

    	/* Envio tarea a buffer de CPU1. */
    	ashoot_RDPG(1, l);

    	cout <<"Hilo 0: Se genero tarea para CPU1." << endl;

    	/* Despierto hilo que enciende CPU1. */

    	/* Abandono monitor.*/
    }
    else /* red.get_TokensPlace(2) > red.get_TokensPlace(8) */
    {
    	/* Manejo de politica de RDPG mediante configuracion de guardas. */
    	red.matrixmodG_set_vG(7,1);			/* equivale a vG[7]=1; y update_vEx(); */
    	red.matrixmodG_set_vG(1,0); 		/* equivale a vG[1]=0; y update_vEx(); */

    	ashoot_RDPG(0, l);

    	/* Envio tarea a buffer de CPU2. */
    	ashoot_RDPG(7, l);

    	cout <<"Hilo 0: Se genero tarea para CPU2." << endl;

    	/* Despierto hilo que enciende CPU2. */

    	/* Abandono monitor.*/
    }

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::ON_OFF_CPU1()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	cout <<"Hilo 1: CPU1 en Stand by." << endl;

	ashoot_RDPG(4, l);					/* Se solicita a la RDPG una peticion de encendido de la CPU1 (T4). */

	ashoot_RDPG(5, l);					/* Se solicita a la RDPG encender CPU1 (T5). */

	cout <<"Hilo 1: CPU1 ON." << endl;

	ashoot_RDPG(6, l);					/* Apago CPU1, inicia su modo de ahorro de energetico (T6).*/

	cout <<"Hilo 1: CPU1 OFF." << endl;
	
										/* Abandono monitor, */

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::ON_OFF_CPU2()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	cout <<"Hilo 3: CPU2 en Stand by." << endl;

	ashoot_RDPG(10, l);					/* Se solicita a la RDPG una peticion de encendido de la CPU2 (T10). */

	ashoot_RDPG(11, l);					/* Se solicita a la RDPG encender CPU2 (T11). */

	cout <<"Hilo 3: CPU2 ON." << endl;

	ashoot_RDPG(12, l);					/* Apago CPU2, inicia su modo de ahorro de energetico (T12).*/

	cout <<"Hilo 3: CPU2 OFF." << endl;
	
										/* Abandono monitor, */

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::procesar_tarea_CPU1()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	cout <<"Hilo 2: CPU1 idle." << endl;

	ashoot_RDPG(2, l);		/* Se solicita a RDPG iniciar el procesamiento de la tarea por la CPU1 (T2). */

	cout <<"Hilo 2: CPU1 procesando tarea..." << endl;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::procesar_tarea_CPU2()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */
	
	cout <<"Hilo 4: CPU2 idle." << endl;

	ashoot_RDPG(8, l);					/* Se solicita a RDPG iniciar el procesamiento de la tarea por la CPU2 (T8). */

	cout <<"Hilo 4: CPU2 procesando tarea..." << endl;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::finalizar_tarea_CPU1()
{
	unique_lock<mutex> l{mtx_monitor};		/* Adquiere exclusion mutua del monitor */
		
	ashoot_RDPG(3, l);						/* Finalizo ejecucion de tarea en CPU1 (T3).*/
	
	cout <<"Hilo 2: CPU1 termino tarea!" << endl;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::finalizar_tarea_CPU2()
{
	unique_lock<mutex> l{mtx_monitor};		/* Adquiere exclusion mutua del monitor */

	ashoot_RDPG(9, l);						/* Finalizo ejecucion de tarea en CPU2 (T9).*/

	cout <<"Hilo 4: CPU2 termino tarea!" << endl;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */



void monitor::deleteRDPG()
{
	unique_lock<mutex> l{mtx_monitor};		/* Adquiere exclusion mutua del monitor */

	red.matrixmodG_delRDPG();

} /* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


bool monitor::empty()
{
	if(red.get_plazas() == 0)
	{
		return true;
	}

	return false;
}


void monitor::print_vcomp(int p_comp)
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	char tmp_cadena[USR_BUF_SIZE];

	red.matrixmodG_view_compRDPG(p_comp, tmp_cadena);

} /* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::print_allComp()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	red.matrixmodG_view_allCompRDPG();

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */



void monitor::import_RDPG(string p_mII, string p_mIH, string p_mIR, string p_mIRe, string p_vMI)
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	//red.import_RDPG(p_mII, p_mIH, p_mIR, p_mIRe, p_vMI);

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


int monitor::shoot_RDPG(int p_transicion)
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	int k = red.matrixmodG_shoot_RDPG(p_transicion);
	
	if(k)
	{
		cout << "	--> Info: Disparo de transicion T"<< p_transicion << " exitoso!!!\n";
	}
	else
		cout << "	--> Info: Fallo disparo de transicion T" << p_transicion << "!!!\n";

	return k;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::ashoot_RDPG(int p_transicion, unique_lock<mutex>& lck)
{
	//unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	double timeop_omp=0; 				/* tiempo de operacion desde API omp. */
	double timeop_ltime=0; 				/* tiempo de operacion desde libreria time. */

	/* Variables para medicion de tiempos. */
	double t_ini = 0;
	double t_fin = 0;
	clock_t t_ini2 = 0;
	clock_t t_fin2 = 0;

	/* Comienza cuenta de tiempo. */
	t_ini2 = clock();
	t_ini = omp_get_wtime();

	while(red.matrixmodG_shoot_RDPG(p_transicion) == SHOT_FAILED) /* Si disparo de transicion fallo. */
	{
		/* Finaliza cuenta de tiempo. */
		t_fin2 = clock();
		t_fin = omp_get_wtime();

		timeop_omp = (t_fin-t_ini);
		timeop_ltime = ((double)(t_fin2 - t_ini2) / CLOCKS_PER_SEC);

		/* Registro de tiempo Total acumulado: */
		mtimeop_omp += timeop_omp;
		mtimeop_ltime += timeop_ltime;

		red.matrixmodG_inc_vHQCV((size_t)p_transicion);	/* Entra un hilo a cola de CV de transicion a disparar. */
		vCV[p_transicion].wait(lck); 					/* Se duerme hilo en vCV[transicion]. */
		red.matrixmodG_dec_vHQCV((size_t)p_transicion);	/* Sale un hilo de cola de CV de transicion a disparar. */	

		/* Comienza cuenta de tiempo. */
		t_ini2 = clock();
		t_ini = omp_get_wtime();
	}

	/* Si disparo de transicion tuvo exito: red.shoot_rdpg(p_transicion, p_mode) == SHOT_OK */
	//update_vHD();

	int proximo_disparo = notify_next_shoot(p_transicion);
	
	if((proximo_disparo>=0))		/* Si ai proximo disparo, despierto hilo de mayor prioridad a disparar. */
	{
		if(red.get_vHDelement((size_t)proximo_disparo)>0)
			vCV[proximo_disparo].notify_one();
	}
									/* Si no ai proximo disparo, abandono monitor. */

	/* Finaliza cuenta de tiempo. */
	t_fin2 = clock();
	t_fin = omp_get_wtime();

	timeop_omp = (t_fin-t_ini);
	timeop_ltime = ((double)(t_fin2 - t_ini2) / CLOCKS_PER_SEC);

	/* Registro de tiempo Total acumulado: */
	mtimeop_omp += timeop_omp;
	mtimeop_ltime += timeop_ltime;
}


/* Politica de proximo disparo a realizar depende de cada RDPG en particular. */
int monitor::notify_next_shoot(int p_transicion)
{
	switch(p_transicion)
	{
		case 1: return 4;

		case 3: return 6;

		case 5: return 2;

		case 7: return 10;

		case 9: return 12;

		case 11: return 8;

		default: return -1;
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Despierta un hilo dormido de la transicion indicada por parametro.
 *
 * @param[in]  p_transicion  Numero de la transicion sobre la que se desea depertar el hilo.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void monitor::notify_thread(int p_transicion)
{	
	if((p_transicion>=0))		/* Si ai proximo disparo, despierto hilo de mayor prioridad a disparar. */
	{
		if(red.get_vHDelement((size_t)p_transicion)>0)
			vCV[p_transicion].notify_one();
	}
}



void monitor::print_RDPGinfo()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	//return red.shoot_rdpg(p_transicion, p_mode);

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


int monitor::get_TokensPlace(size_t p_place)
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	return red.get_TokensPlace(p_place);

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


/* Funciones ejecutados por un mismo hilo (m: monocore). */
void monitor::generar_tarea_m()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */
	//mtx_monitor.lock();
	static size_t tasks=0; 				/* Tareas generadas. */
	tasks++;

	//red.print_vcomp(_vEx);

    /* Analisis de Guardas. */
    if((red.get_TokensPlace(2) == -EC_datoIncorrecto) && (red.get_TokensPlace(8) == -EC_datoIncorrecto))
    {
    	/* Si no existen marcados de plazas indicadas, gestionar error.*/

    }
    else if(red.get_TokensPlace(2) <= red.get_TokensPlace(8))
    {
    	/* Manejo de politica de RDPG mediante configuracion de guardas. */
    	red.matrixmodG_set_vG(1,1); 		/* equivale a vG[1]=1; y update_vEx(); */
    	red.matrixmodG_set_vG(7,0);			/* equivale a vG[7]=0; y update_vEx(); */

    	red.matrixmodG_shoot_RDPG(0);

    	/* Envio tarea a buffer de CPU1. */
    	red.matrixmodG_shoot_RDPG(1);

    	cout <<"Hilo 0: Se genero tarea para CPU1." << endl;

    	/* Abandono monitor.*/
    }
    else /* red.get_TokensPlace(2) > red.get_TokensPlace(8) */
    {
    	/* Manejo de politica de RDPG mediante configuracion de guardas. */
    	red.matrixmodG_set_vG(7,1);			/* equivale a vG[7]=1; y update_vEx(); */
    	red.matrixmodG_set_vG(1,0); 		/* equivale a vG[1]=0; y update_vEx(); */

    	red.matrixmodG_shoot_RDPG(0);

    	/* Envio tarea a buffer de CPU2. */
    	red.matrixmodG_shoot_RDPG(7);

    	cout <<"Hilo 0: Se genero tarea para CPU2." << endl;

    	/* Abandono monitor.*/
    }

    /*if(tasks== N_TASKS){
    	cout << "Vector de marcado actual: "<<endl;
    	red.print_vcomp(view_vMA);
    	cout << endl;
    }*/
    //mtx_monitor.unlock();

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::encender_CPU1_m()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	cout <<"Hilo 1: CPU1 en Stand by." << endl;

	red.matrixmodG_shoot_RDPG(4);		/* Se solicita a la RDPG una peticion de encendido de la CPU1 (T4). */

	red.matrixmodG_shoot_RDPG(5);		/* Se solicita a la RDPG encender CPU1 (T5). */

	cout <<"Hilo 1: CPU1 ON." << endl;
	
										/* Abandono monitor, */

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::encender_CPU2_m()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	cout <<"Hilo 3: CPU2 en Stand by." << endl;

	red.matrixmodG_shoot_RDPG(10);	/* Se solicita a la RDPG una peticion de encendido de la CPU2 (T10). */

	red.matrixmodG_shoot_RDPG(11);	/* Se solicita a la RDPG encender CPU2 (T11). */

	cout <<"Hilo 3: CPU2 ON." << endl;
	
										/* Abandono monitor, */

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::procesar_tarea_CPU1_m()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	cout <<"Hilo 2: CPU1 idle." << endl;

	red.matrixmodG_shoot_RDPG(2);		/* Se solicita a RDPG iniciar el procesamiento de la tarea por la CPU1 (T2). */

	cout <<"Hilo 2: CPU1 procesando tarea..." << endl;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::procesar_tarea_CPU2_m()
{
	unique_lock<mutex> l{mtx_monitor};	/* Adquiere exclusion mutua del monitor */

	cout <<"Hilo 4: CPU2 idle." << endl;

	cout <<"Hilo 4: CPU2 procesando tarea..." << endl;
	red.matrixmodG_shoot_RDPG(8);		/* Se solicita a RDPG iniciar el procesamiento de la tarea por la CPU2 (T8). */

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::finalizar_tarea_CPU1_m()
{
	unique_lock<mutex> l{mtx_monitor};		/* Adquiere exclusion mutua del monitor */
	
	red.matrixmodG_shoot_RDPG(3);			/* Finalizo ejecucion de tarea en CPU1 (T3).*/
	
	cout <<"Hilo 2: CPU1 termino tarea!" << endl;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::finalizar_tarea_CPU2_m()
{
	unique_lock<mutex> l{mtx_monitor};		/* Adquiere exclusion mutua del monitor */

	red.matrixmodG_shoot_RDPG(9);		/* Finalizo ejecucion de tarea en CPU2 (T9).*/

	cout <<"Hilo 4: CPU2 termino tarea!" << endl;

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::apagar_CPU1_m()
{
	unique_lock<mutex> l{mtx_monitor};		/* Adquiere exclusion mutua del monitor */

	cout << "Hilo 1: Esperando apagar CPU1.";

	cout <<"Hilo 1: CPU1 OFF." << endl;
	red.matrixmodG_shoot_RDPG(6);			/* Apago CPU1, inicia su modo de ahorro de energetico (T6).*/

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */


void monitor::apagar_CPU2_m()
{
	unique_lock<mutex> l{mtx_monitor};		/* Adquiere exclusion mutua del monitor */

	cout << "Hilo 3: Esperando apagar CPU1." << endl;

	//red.print_vcomp(_vE);

	cout <<"Hilo 3: CPU2 OFF." << endl;
	red.matrixmodG_shoot_RDPG(12);/* Apago CPU2, inicia su modo de ahorro de energetico (T12).*/

}/* Libera la exclusion mutua al finalizar el bloque de codigo de la funcion. */
