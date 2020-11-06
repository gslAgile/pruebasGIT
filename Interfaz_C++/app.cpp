#include "app.hpp"

// Declaracion de funciones
void funcion_1();
void funcion_2();

int main() {

	/* Archivos de importacion de RDPG: */
	string n_mII = "../Caso_Aplicacion/mII.txt";
	string n_mIH = "../Caso_Aplicacion/mIH.txt";
	string n_mIR = "../Caso_Aplicacion/mIR.txt";
	string n_mIRe = "../Caso_Aplicacion/mIRe.txt";// = "mIRe.txt";
	string n_vMI = "../Caso_Aplicacion/vMI.txt";

	//RDPG red("Red de Petri", 6, 4);
	//RDPG *red = new RDPG("Red de Petri", 1000, 1000);
	//RDPG *red = new RDPG("Red 1", n_mII, n_mIH, n_mIR, n_mIRe, n_vMI);
	string vacio;
	RDPG_Driver rdp ("Red 1", vacio, vacio, vacio, vacio, vacio);
	
	monitor *red = new monitor("Red 1", n_mII, n_mIH, n_mIR, n_mIRe, n_vMI);	

 	cout << " Hola, mundo RDPG! " << endl;

 	int a;

 	//cin >> a;

 	/* Variables para medicion de tiempos. */
 	double timeop_omp=0, timeop_ltime=0;
	double t_ini = 0;
	double t_fin = 0;
	clock_t t_ini2 = 0;
	clock_t t_fin2 = 0;

	if(!red->empty())
	{
	    //usleep(100);

		/* Comienza cuenta de tiempo. */
		t_ini2 = clock();
		t_ini = omp_get_wtime();


		thread t2([&]() {
	        while(true)
	        {
	        	red->encender_CPU1();
	        	red->apagar_CPU1();
	        }
	    });


	    thread t4([&]() {
	        while(true)
	        {
	        	red->encender_CPU2();
	        	red->apagar_CPU2();
	        }
	    });


	    thread t3([&]() {
	        while(true)
	        {
	        	red->procesar_tarea_CPU1();
	        	//sleep(1);
	        	red->finalizar_tarea_CPU1();
	        }

	        cout << "Hilo 5: Termino." << endl;
	    });


	    thread t5([&]() {
	        while(true)
	        {
	        	red->procesar_tarea_CPU2();
	        	//sleep(2);
	        	red->finalizar_tarea_CPU2();
	        }
	        cout << "Hilo 5: Termino." << endl;
	    });

	    sleep(3);

	    thread t1([&]() {
	        for(size_t i=0; i<N_TASKS; i++)
	        {
	        	red->generar_tarea();
	        }
	        red->print_vcomp(view_vMA);
	    });

	    /* Finaliza cuenta de tiempo. */
		t_fin2 = clock();
		t_fin = omp_get_wtime();

		timeop_omp = (t_fin-t_ini);
		timeop_ltime = ((double)(t_fin2 - t_ini2) / CLOCKS_PER_SEC);


		int opcion = 0, op_thread = 0;
	    while( opcion != 4)
		{
		    cout << "\n--------------------------------------------------------------------------------\n";
	    	cout <<  "\t\tAPP C++ - MENU DE OPERACIONES RDPG \n";
	    	cout << "--------------------------------------------------------------------------------\n";
	        cout <<  "\n  1. Ver estado de RDPG (marcado actual).";
	        cout <<  "\n  2. Ver estado completo de RDPG (todos los componentes).";
	        cout <<  "\n  3. Ver vector Ex de transicicones sensibilizadas extendido en la RDPG.";
	        cout <<  "\n  4. Salir y finalizar hilos.";
	        cout <<  "\n  5. Salir y esperar hilos.";
	        cout <<  "\n\n   Ingrese numero de opcion: ";
	        cin >> opcion;


	        switch(opcion)
	        {
	        	case 1: 
	        			red->print_vcomp(view_vMA);
	        			break;

	        	case 2:	
	        			red->print_allComp();
	        			break;

	        	case 3:	
	        			red->print_vcomp(view_vEx);
	        			break;

	        	case 4:	
	        			op_thread = 1;
	        			break;

	        	case 5:
	        			op_thread = 2;
	        			break;

	        	default:
	        			cout << "   Tiempo de operacion promedio con openMP: " << timeop_omp << endl;
						cout << "   Tiempo de operacion promedio con lib time: " << timeop_ltime << endl;
	        			cout<< "Opcion incorrecta!" << endl;
	        }
	    }

	    red->deleteRDPG();

	    if(op_thread == 2)
		{
			t1.join();
			t2.join();
			t3.join();
			t4.join();
			t5.join();	
		}
	}


	delete red;


	return 0;
}