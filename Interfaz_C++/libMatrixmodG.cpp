/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * Libreria C++ de Device Driver Linux MatrixmodG (archivo fuente - source file).
 * 
 * 
 * Esta libreria contiene las declarciones y definiciones de la clase RDPG_Driver para gestionar Redes de Petri Generalizadas (RDPG) en el espacio usuario
 * con ayuda del espacio kernel mediante la interaccion de llamadas al sistema con el file device "/proc/matrixmodG_fd" del Device Driver MatrixmodG.
 * 
 * Todas las funciones son transparentes para el usuario final, haciendo uso de la clase RDPG_Driver para la gestion de objetos RDPG en el kernel desde
 * el espacio usuario.
 * 
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "libMatrixmodG.hpp"

int RDPG_Driver::n_objects=0; /* Inicializo contador global de objetos. */

/* Variables globales: */
const char *cmd_addm[ID_MC_END] = {"RDPG add mII ","RDPG add mIH ","RDPG add mIR ","RDPG add mIRe "};
const char *cmd_addv[ID_VC_END] = {"RDPG add vMI "};
const char *cmd_RDPGinfo[ID_VIEW_END] = {"RDPGinfo name\n","RDPGinfo places\n","RDPGinfo transitions\n","RDPGinfo shots\n","RDPGinfo memory\n","RDPGinfo comp\n",
"RDPGinfo empty\n", "RDPG cat mII\n","RDPG cat mIH\n","RDPG cat mIR\n","RDPG cat mIRe\n","RDPG cat mD\n",
"RDPG cat vMI\n","RDPG cat vMA\n","RDPG cat vMN\n","RDPG cat vQ\n", "RDPG cat vW\n","RDPG cat vE\n",
"RDPG cat vB\n","RDPG cat vL\n","RDPG cat vG\n","RDPG cat vA\n","RDPG cat vUDT\n","RDPG cat vEx\n","RDPG cat vHQCV\n","RDPG cat vHD\n"};

const char *cmd_createRDPG = "create RDPG ";
const char *cmd_delRDPG = "RDPG delete\n";
const char *cmd_confirmRDPG = "RDPG confirm\n";
const char *cmd_shootRDPG = "RDPG shoot ";
const char *cmd_getTokensPlace = "RDPG get tokens "; 
const char *cmd_getvHDelement = "RDPG get vHD "; 
const char *cmd_set_vG = "RDPG set vG ";
const char *cmd_inc_vHQCV = "RDPG inc vHQCV ";
const char *cmd_dec_vHQCV = "RDPG dec vHQCV ";

char libCadena[USR_BUF_SIZE];

/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * DEFINICION/IMPLEMENTACION DE FUNCIONES DE LIBRERIA RDPG_object.h
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Destructor de objetos RDPG_Driver. Metodo que se invoca al realizar la destrccion del objeto RDPG_Driver. Su llamada es automatica al finalizar
 *             el uso del objeto.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
RDPG_Driver::~RDPG_Driver() { 

	n_objects--;
	
	if(kernel_RDPG_sec)	/* Usuario elije eliminar la RDPG del kernel o no eliminarla. */
	{
		matrixmodG_delRDPG(); 
	}
	else 	/* Se elimina directamente la RDPG del kernel. */
	{
		(void)write_matrixmodG((char *)cmd_delRDPG);	/* Se elimina RDPG en driver con comando write*/
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Constructor personalizado 1: Este metodo es el constructor de un objeto RDPG_Driver. Se inicializan todos sus elementos de acuerdo a los 
 * 			   valores de plazas y transiciones enviados por parametro.
 *
 * @param[in]  p_name         Nombre del RDPG_Driver.
 * @param[in]  p_places       Numero de plazas a configurar para la RDPG a enviar al driver MatrixmodG.
 * @param[in]  p_transicions  Numero de transiciones a configurar para la RDPG a enviar al driver MatrixmodG.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
RDPG_Driver::RDPG_Driver(string p_name, size_t p_places, size_t p_transicions)
: name(p_name), posVP(0), posVT(0), vdim(MAX_VDIM), error_code(EC_NULL), shot_result(SHOT_INIT)
{
	/* Determino numero de plazas y transiciones de RDPG desde archivo p_mII (su matriz de incidencia). */
	plazas = p_places;
	transiciones = p_transicions;
	kernel_RDPG_sec = false; 		/*Se deja sin proteccion la RDPG del kernel. */

	if( (plazas > 0) && (transiciones > 0))
	{
		int empty = matrixmodG_empty();
		if(empty)
		{
			/* Redimensiono los componentes matrix_o y vector_o de la RDPG. */
			resize_allmatrixs();
			resize_allvectors();

			/* Se establece conexion con file device de driver MatrixmodG. */
			connect_driver = DISABLED_CONNECTION; 					/* Inicia flag de conexion deshabilitada.*/
			connect_driverMatrixmodG();								/* Se establece conexion con driver.*/
			//system_test_mode = 0; 								/* Inicia por defecto modo de pruebas de sistema desactivado.*/

			/* Creo RDPG con plazas y transiciones siempre que se haya importado matriz de incidencia I minimamente. */
			matrixmodG_createRDPG();
		}
		else if(empty == 0){ /* Ya ai RDPG previamente cargada en kernel. Se establece solo conexion.*/
			
			/* Se establece conexion con file device de driver MatrixmodG. */
			connect_driver = DISABLED_CONNECTION; 					/* Inicia flag de conexion deshabilitada.*/
			connect_driverMatrixmodG();								/* Se establece conexion con driver.*/
			cout << "RDPG previamente cargada en kernel. Se establece solo conexion con DDL MatrixmodG." << endl;
		}
		else{
			cout << "Fallo la connexion con DDL MatrixmodG." << endl;
		}

	}
	else
	{
		cout << "No se pudo crear componentes de RDPG para: (" << p_places << " plazas) y" << p_transicions << " transiciones) ." << endl;
	}

	obj_id = (int)n_objects;
	n_objects++;
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Constructor personalizado 2: Este es el metodo constructor en una version alternativa, que permite la construccion del objeto RDPG_Driver
 * 			   mediante la lectura de los archivos de los componentes bases de una RDPG, cuyos nombres se envian por parametro.
 *
 * @param[in]  p_name  Nombre a configurar en el objeto RDPG_Driver.
 * @param[in]  p_mII   Nombre del archivo asociado al componente matrix_o mII de la RDPG.
 * @param[in]  p_mIH   Nombre del archivo asociado al componente matrix_o mIH de la RDPG.
 * @param[in]  p_mIR   Nombre del archivo asociado al componente matrix_o mIR de la RDPG.
 * @param[in]  p_mIRe  Nombre del archivo asociado al componente matrix_o mIRe de la RDPG.
 * @param[in]  p_vMI   Nombre del archivo asociado al componente matrix_o vMI de la RDPG.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
RDPG_Driver::RDPG_Driver(string p_name, string p_mII, string p_mIH, string p_mIR, string p_mIRe, string p_vMI)
: name(p_name), posVP(0), posVT(0), vdim(MAX_VDIM), error_code(EC_NULL), shot_result(SHOT_INIT)
{
	/* Determino numero de plazas y transiciones de RDPG desde archivo p_mII (su matriz de incidencia). */
	plazas = get_fileLines(p_mII);
	transiciones = get_lineElements(p_mII);
	kernel_RDPG_sec = false; 		/*Se deja sin proteccion la RDPG del kernel. */

	if((plazas > 0) && (transiciones > 0))
	{
		int empty = matrixmodG_empty();
		if(empty)
		{
			/* Redimensiono los componentes matrix_o y vector_o de la RDPG. */
			resize_allmatrixs();
			resize_allvectors();

			/* Se carga matrix_o con valores de acuerdo a archivos .txt de RDPG.*/
			import_RDPG(p_mII, p_mIH, p_mIR, p_mIRe, p_vMI);

			/* Se establece conexion con file device de driver MatrixmodG. */
			connect_driver = DISABLED_CONNECTION; 					/* Inicia flag de conexion deshabilitada.*/
			connect_driverMatrixmodG();								/* Se establece conexion con driver.*/

			/* Creo RDPG con plazas y transiciones siempre que se haya importado matriz de incidencia I minimamente. */
			matrixmodG_createRDPG();
			
			/* Cargo valores de mII a RDPG de p_DriverObj matrixmodG */
			matrixmodG_addm(_mII);
			
			/* Cargo valores de matriz de inicidencia H */
			matrixmodG_addm(_mIH);

			/* Cargo valores de matriz de inicidencia R */
			matrixmodG_addm(_mIR);

			/* Cargo valores de matriz de transiciones reset Re */
			matrixmodG_addm(_mIRe);

			/* Cargo valores de vectorde marcado inicil vMI a RDPG de matrixmodG */
			matrixmodG_addv(_vMI);

			/* Confirmacion de componentes enviados a RDPG del kernel. al enviar este comando, la RDPG del kernel termina de calcular el resto de componentes.*/
			matrixmodG_confirmRDPG();
		}
		else if(empty == 0){ /* Ya ai RDPG previamente cargada en kernel. Se establece solo conexion.*/
			
			/* Se establece conexion con file device de driver MatrixmodG. */
			connect_driver = DISABLED_CONNECTION; 					/* Inicia flag de conexion deshabilitada.*/
			connect_driverMatrixmodG();								/* Se establece conexion con driver.*/
			cout << "RDPG previamente cargada en kernel. Se establece solo conexion con DDL MatrixmodG." << endl;
		}
		else{
			cout << "Fallo la connexion con DDL MatrixmodG." << endl;
		}
	}
	else
	{
		cout << "No se pudo importar archivos *.txt de los componentes de RDPG: (" << name << ")." << endl;
		/* Se establece conexion con file device de driver MatrixmodG. */
		connect_driver = DISABLED_CONNECTION; 					/* Inicia flag de conexion deshabilitada.*/
		connect_driverMatrixmodG();								/* Se establece conexion con driver.*/
	}

	obj_id = (int)n_objects;
	n_objects++;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * METODOS GETTERS
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo retorna el numero de lineas de un archivo. 
 * 			   This method gets the file lines.
 *
 * @param[in]  p_fname  Nombre del archivo sobre el cual leer el numero de lineas.
 *
 * @return     El retorno es el numero de lineas del archivo, si tuvo exito la operacion.
 * 			   El retorno sera -EC_fileReadFail, si no tuvo exito la operacion.
 * 			   The return is file lines, if the operation was successful.
 * 			   The return will be -EC_fileReadFail, if operation no was successful.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
size_t RDPG_Driver::get_fileLines(string p_fname)
{
	ifstream fin; 						/* Objeto para lectura de archivo. */
	string line;						/* Cadena que almacena una linea del archivo. */
	size_t i = 0;

	fin.open(p_fname.c_str());			/* Apertura de archivo. */

	if(!fin.is_open())					/* Si fallo apertura de archivo, se retorna el fallo. */
	{
		cout << "   --> ERROR: No se pudo abrir archivo ( " << p_fname << " ) para su lectura." << endl;
		return -EC_fileReadFail;
	}

	getline(fin, line);					/* Captura de primer linea de archivo. */

	while( !fin.eof() ){ 
		getline(fin, line); i++;		/* Cuento numero de lineas en archivo. */
		
		if(i > MAX_PLACES)				/* Error por exeso de plazas soportadas por APP. */
			return (size_t)0;
	}

	return i;							/* Retorno numero de lineas de archivo. */
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo retorna el numero de elementos de la primer linea de un archivo.
 * 			   This method gets the elements number of file line first.
 *
 * @param[in]  p_fname  Nombre del archivo sobre el cual realizar la operacion.
 *
 * @return     Se retorna el nummero de elementos de la primer linea del archivo, si la operacion tuvo exito.
 * 			   Se retorna -EC_fileReadFail, si la operacion no tuvo exito.
 * 			   Return the elements number of file line first, if operation was successful.
 * 			   Return -EC_fileReadFail, if operation not was successful.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
size_t RDPG_Driver::get_lineElements(string p_fname)
{
	ifstream fin; 						/* Objeto para lectura de archivo. */
	string line;						/* Cadena que almacena una linea del archivo. */

	fin.open(p_fname.c_str());			/* Apertura de archivo. */

	if(!fin.is_open())					/* Si fallo apertura de archivo, se retorna el fallo. */
	{
		cout << "   --> ERROR: No se pudo abrir archivo ( " << p_fname << " ) para su lectura." << endl;
		return -EC_fileReadFail;
	}

	getline(fin, line);					/* Captura de primer linea de archivo. */

	istringstream dline(line);			/* Objeto para tratar string como un stream de entrada. Se inicia con dline (data line).*/
	int element;						/* Entero para almacenar elementos de matriz.*/
	size_t j=0;

	while(dline >> element){ 
		j++; 							/* Cuento numero de elementos enteros en linea. */
		if(j > MAX_TRANSITIONS)			/* Error por exeso de transiciones soportadas por APP. */
			return (size_t)0;
	}
	
	return j;							/* Retorno numero de elementos enteros en linea. */
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo retorna el numero de tokens de la plaza enviada por parametro para la RDPG cargada en el kernel de Linux.
 * 			   El metodo realiza una llamada al sistema (write) sobre el archivo de dispositivo matrixmodG_fd del driver MatrixmodG.
 *
 * @param[in]  p_place  Numero de la plaza sobre la que se desea conocer el numero de tokens.
 *
 * @return     Se retorna el numero de tokens de la plaza enviada por parametros, si la operacion tuvo exito.
 * 			   Se retorna -EC_datoIncorrecto, si la operacion no tuvo exito.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
int RDPG_Driver::get_TokensPlace(size_t p_place)
{
	char cmd[N_CMD];	 				/* comando para realizar disparo*/
	char s_place[N_BYTES];				/* cadena de caracter que almacena el numero entero de una plaza en ascii. */
	memset(cmd, '\0', N_CMD);			/* se limpia cadena */

	itoa(p_place, s_place, N_BYTES);	/* Conversion de numero entero a ascii. */

	strcat(cmd, cmd_getTokensPlace); 	/* se agrega comando asociado a getTokensPlace. */
	strcat(cmd, s_place);				/* se agrega cadena(nro plaza) al final de comando */


	if(p_place < plazas) /* Validacion de direccion correcta en vector. */
	{
		//cout << "Info: Se retorna numero de tokens de plaza P" << p_place << endl;
		return write(my_fd, cmd, strlen(cmd));
	}
	else
	{
		cout << "Error: Numero de plaza incorrecto. La plaza P" << p_place << " no existe en la RDPG: ("<< name <<")." << endl;
		error_code = -EC_datoIncorrecto;
		return -EC_datoIncorrecto;
	}
}


int RDPG_Driver::get_vHDelement(size_t p_transicion)
{
	char cmd[N_CMD];	 				/* comando para realizar disparo*/
	char s_transition[N_BYTES];			/* cadena de caracter que almacena el numero entero de una plaza en ascii. */
	memset(cmd, '\0', N_CMD);			/* se limpia cadena */

	itoa(p_transicion, s_transition, N_BYTES);	/* Conversion de numero entero a ascii. */

	strcat(cmd, cmd_getvHDelement); 	/* se agrega comando asociado a get_vHDelement. */
	strcat(cmd, s_transition);			/* se agrega cadena (nro transicion) al final de comando */


	if(p_transicion < transiciones) 	/* Validacion de direccion correcta en vector. */
	{
		return write(my_fd, cmd, strlen(cmd));
	}
	else
	{
		cout << "Error: Numero de transicion incorrecto. La transicion T" << p_transicion << " no existe en la RDPG: ("<< name <<")." << endl;
		error_code = -EC_datoIncorrecto;
		return -EC_datoIncorrecto;
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo setea la variable kernel_RDPG_sec de un objeto RDPG, con el valor enviado por parametro.
 *
 * @param[in]  value  Valor booleano para setear sobre la variable kernel_RDPG_sec.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::set_kernel_RDPG_sec(bool value)
{
	kernel_RDPG_sec = value;
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * METODOS DE OBJETOS RDPG
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo realiza la importacion de cada uno de los componentes base de una RDPG a traves del nombre de los archivos enviados por parametro.
 * 			   Se utilizan los metodos import_matrix() e import_vector() para completar la operacion.
 *
 * @param[in]  p_mII   Nombre del archivo asociado al componente mII.
 * @param[in]  p_mIH   Nombre del archivo asociado al componente mIH.
 * @param[in]  p_mIR   Nombre del archivo asociado al componente mIR.
 * @param[in]  p_mIRe  Nombre del archivo asociado al componente mIRe.
 * @param[in]  p_vMI   Nombre del archivo asociado al componente vMI.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::import_RDPG(string p_mII, string p_mIH, string p_mIR, string p_mIRe, string p_vMI)
{
	/* Carga de matrices desde archivos*/
	if(!p_mII.empty())
		import_matrix(ref_mcomp(_mII),p_mII);
	
	if(!p_mIH.empty())
		import_matrix(ref_mcomp(_mIH),p_mIH);
	
	if(!p_mIR.empty())
		import_matrix(ref_mcomp(_mIR),p_mIR);
	
	if(!p_mIRe.empty())
		import_matrix(ref_mcomp(_mIRe),p_mIRe);
	
	if(!p_vMI.empty())
		import_vector(ref_vcomp(_vMI),p_vMI);

	if(MGUL_DB_MSG) cout << "Importacion de archivos *.txt exitosa para los componentes de RDPG: (" << name << ")." << endl;
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de realizar la importacion de cada uno de los elementos de un componente matrix_o de la RDPG desde un archivo asociado.
 *
 * @param      p_mo     Referencia al objeto matrix_o sobre el cual importar los elementos desde el archivo.
 * @param[in]  p_fname  Nombre del archivo asociado desde el que se importan los elementos para el componente matrix_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::import_matrix(matrix_o& p_mo, string p_fname)
{
	ifstream fin; 							/* Objeto para lectura de archivo. */
	string line;							/* Cadena que almacena una linea del archivo. */
	size_t i = 0;

	fin.open(p_fname.c_str());				/* Apertura de archivo. */

	if(!fin.is_open())						/* Si fallo apertura de archivo, se retorna el fallo. */
	{
		cout << "   --> ERROR: No se pudo abrir archivo ( " << p_fname << " ) para su lectura." << endl;
		return;
	}

	getline(fin, line);						/* Captura de primer linea de archivo. */

	while( (!fin.eof()) && (i<plazas))		/* Mientras no se detecte final de archivo.*/
	{
		add_values_mcomp(p_mo, line, i);	/* Adicion de valores de linea en fila de matrix_o p_mo. */
		getline(fin, line);					/* Captura de siguiente linea de archivo. */
		i++;
	}

}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de realizar la importacion de cada uno de los elementos de un componente vecotr_o de la RDPG desde un archivo asociado.
 *
 * @param      p_vo     Referencia al objeto vector_o sobre el cual importar los elementos desde el archivo.
 * @param[in]  p_fname  Nombre del archivo asociado desde el que se importan los elementos para el componente vector_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::import_vector(vector_o& p_vo, string p_fname)
{
	ifstream fin; 						/* Objeto para lectura de archivo. */
	string line;						/* Cadena que almacena una linea del archivo. */

	fin.open(p_fname.c_str());			/* Apertura de archivo. */

	if(!fin.is_open())					/* Si fallo apertura de archivo, se retorna el fallo. */
	{
		cout << "   --> ERROR: No se pudo abrir archivo ( " << p_fname << " ) para su lectura." << endl;
		return;
	}

	getline(fin, line);					/* Captura de primer linea de archivo. */

	while( !fin.eof())					/* Mientras no se detecte final de archivo.*/
	{
		add_values_vcomp(p_vo, line);	/* Adicion de valores de primer linea en vector_o p_vo. */
		getline(fin, line);				/* Captura de siguiente linea de archivo. */
	}

}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de asignar los valores de una fila del componente matrix_o enviado por parametro.
 *
 * @param      p_mo    Referencia al objeto matrix_o sobre el cual asignar los valores.
 * @param[in]  p_line  Cadena de caracter con cada uno de los elementos a asignar.
 * @param[in]  p_fila  Numero de fila asociada a componente matrix_o sobre la que se asignan los elementos.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::add_values_mcomp(matrix_o& p_mo, string p_line, size_t p_fila)
{
	istringstream dline(p_line);		/* Objeto para tratar string como un stream de entrada. Se inicia con dline (data line).*/
	int element;						/* Entero para almacenar elementos de matriz.*/
	size_t j=0;

	while(dline >> element)
	{
		p_mo[p_fila][j] = element;		/* Asignacion de elemento de string en matrix_o. */
		j++;
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de asignar los valores de un componente vector_o enviado por parametro.
 *
 * @param      p_vo    Referencia al objeto vector_o sobre el que se desea realizar la asignacion.
 * @param[in]  p_line  Cadena de caracter con cada uno de los elementos a asignar.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::add_values_vcomp(vector_o& p_vo, string p_line)
{
	istringstream dline(p_line);		/* Objeto para tratar string como un stream de entrada. Se inicia con dline (data line).*/
	int element;						/* Entero para almacenar elementos de vector.*/
	size_t i=0;

	while(dline >> element)
	{
		p_vo[i] = element;				/* Asignacion de elemento de string en vector_o. */
		i++;
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * METODOS DE COMPOENTES matrix_o DE RDPG
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo analiza los componentes matrix_o y retorna la referencia de uno de ellos si coincide con el id enviado por parametro.
 *
 * @param[in]  p_mcomp  Identificador del componente matrix_o que se desea retornar.
 *
 * @return     Se retorna la referencia del objeto matrix_o asociada al identificador enviado por parametro.
 * 			   En caso de error se indica en el campo error_code con el error -EC_referenciaNula.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
matrix_o& RDPG_Driver::ref_mcomp(ID_MCOMPONENT p_mcomp)
{
	error_code = EC_NULL;

	if((p_mcomp>=ID_MC_INIT) && (p_mcomp < ID_MC_END))
	{
		if(p_mcomp == _mII)
			return mII;
		else if(p_mcomp == _mIH)
			return mIH;
		else if(p_mcomp == _mIR)
			return mIR;
		else if(p_mcomp == _mIRe)
			return mIRe;
	}
	
	error_code = -EC_referenciaNula;
	cout<< "Error: En la RDPG: (" << name << ") no existe la referencia de matrix_o indicada como parametro." << endl;
	//return (matrix_o&)0;
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de limpiar un componente matrix_o de la RDPG.
 *
 * @param[in]  p_mcomp  Identificador del componente matrix_o de la RDPG a limpiar.
 * 			 			Si no de encuentra el componente de la RDPG se avisa en el atributo error_code con -EC_componeteNoValido.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::clean_matrix(ID_MCOMPONENT p_mcomp)
{
	size_t i, j;

	if((p_mcomp>=ID_MC_INIT) && (p_mcomp < ID_MC_END))
	{
		matrix_o& comp = ref_mcomp(p_mcomp);
	
		for(i = 0; i<comp.size(); i++) {
			for(j = 0; j<comp[0].size(); j++)
				comp[i][j] = 0;
		}
	}
	else
	{

	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encargar de recargar un componente matrix_o de la RDPG con un valor literal enviado por parametro.
 *
 * @param[in]  p_mcomp  Identificador del componente matrix_o de la RDPG que se desea recargar.
 * @param[in]  p_valor  Valor literal que se desea cargar en cada uno de los elementos del componente matrix_o.
 * 			 			Si no de encuentra el componente de la RDPG se avisa en el atributo error_code con -EC_componeteNoValido.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::reload_matrix(ID_MCOMPONENT p_mcomp, int p_valor)
{
	size_t i, j;

	if((p_mcomp>=ID_MC_INIT) && (p_mcomp < ID_MC_END))
	{
		matrix_o& comp = ref_mcomp(p_mcomp);

		for(i = 0; i<comp.size(); i++) {
			for(j = 0; j<comp[0].size(); j++)
					comp[i][j] = p_valor;
		}
	}
	else
	{

	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de imprimir cualquier componete matrix_o de la RDPG.
 *
 * @param[in]  p_mcomp  Identificador del componente matrix_o de la RDPG a imprimir.
 * 			 			Si no de encuentra el componente de la RDPG se avisa en el atributo error_code con -EC_componeteNoValido.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::print_mcomp(ID_MCOMPONENT p_mcomp)
{
	size_t i, j, vp, vt;

	print_headerT();

	if((p_mcomp>=ID_MC_INIT) && (p_mcomp < ID_MC_END))
	{
		matrix_o& comp = ref_mcomp(p_mcomp);

		for(i = posVP, vp=0; (i<comp.size())&&(vp<vdim); i++, vp++) {
			if(i < 10)
				cout << " P"<< i << right << setw(3) << "|";
			else if(i < 100)
				cout << " P"<< i << right << setw(2) << "|";
			else if( i < MAX_PLACES)
				cout << " P"<< i << right << setw(1) << "|";
			for(j = posVT, vt=0; (j<comp[0].size())&&(vt<vdim); j++, vt++){
				if(j < 10)
					cout << right << setw( 3 ) << comp[i][j];
				else if(j < 100)
					cout << right << setw( 4 ) << comp[i][j];
				else if( j < MAX_TRANSITIONS)
					cout << right << setw( 5 ) << comp[i][j];
			}
			cout << endl;
		}
	}
	else
	{

	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de imprimir la cabecera de las transiciones de acuerdo con las configuraciones de los atributos vdim y posVT.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::print_headerT()
{
	size_t vt; 				/* Numero de transiciones visualizadas.*/
	if(((posVT+vdim)>=100) && (vdim>14) && (posVT<(MAX_TRANSITIONS-14)))
		print_line('-', 165);
	else if( ((posVT+vdim)<100) && (vdim>18) )
		print_line('-', 165);
	else
		print_line('-');
 	
 	cout << "     |  ";

 	for(size_t i=posVT, vt=0; (i < transiciones)&&(vt < vdim); i++, vt++)
 	{
 		if(i < 10)
 			cout << "T" << i << right << setw( 2 );
 		else if(i < 100)
 			cout << "T" << i << right << setw( 2 );
 		else if(i < MAX_TRANSITIONS)
 			cout << "T" << i << right << setw( 2 );
 	}
 	cout << endl;
 	
 	if(((posVT+vdim)>=100) && (vdim>14) && (posVT<(MAX_TRANSITIONS-14)))
		print_line('-', 165);
	else if( ((posVT+vdim)<100) && (vdim>18) )
		print_line('-', 165);
	else
		print_line('-');
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de imprimir la cabecera de las plazas de acuerdo con las configuraciones de los atributos vdim y posVP.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::print_headerP()
{
	size_t vp; 				/* Numero de plazas visualizadas.*/
	if(((posVP+vdim)>=100) && (vdim>14) && (posVP<(MAX_PLACES-14)))
		print_line('-', 165);
	else if( ((posVP+vdim)<100) && (vdim>18) )
		print_line('-', 165);
	else
		print_line('-');

 	cout << "     |  ";

 	for(size_t i=posVP, vp=0; (i < plazas)&&(vp < vdim); i++, vp++)
 	{
 		if(i < 10)
 			cout << "P" << i << right << setw( 2 );
 		else if(i < 100)
 			cout << "P" << i << right << setw( 2 );
 		else if(i < MAX_PLACES)
 			cout << "P" << i << right << setw( 2 );
 	}
 	cout << endl;

 	if(((posVP+vdim)>=100) && (vdim>14) && (posVP<(MAX_PLACES-14)))
		print_line('-', 165);
	else if( ((posVP+vdim)<100) && (vdim>18) )
		print_line('-', 165);
	else
		print_line('-');
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de realizar una reasignacion de tamaño en los componentes matrix_o del objeto RDPG de acuerdo al numero 
 * 			   de plazas y tranciones.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::resize_allmatrixs()
{
	resize_matrix(_mII, plazas, transiciones);
	resize_matrix(_mIH, plazas, transiciones);
	resize_matrix(_mIR, plazas, transiciones);
	resize_matrix(_mIRe, plazas, transiciones);
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de reasignar el tamaño de memoria a un componente matrix_o de la RDPG.
 *
 * @param[in]  p_mcomp  Identificador del componente matrix_o de la RDPG a realizar la reasignacion.
 * @param[in]  p_row    Numero de filas a reasignar en el componente matrix_o.
 * @param[in]  p_col    Numero de columnas a reasignar en el componente matrix_o-
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::resize_matrix(ID_MCOMPONENT p_mcomp, size_t p_row, size_t p_col)
{
	size_t i;
	matrix_o& comp = ref_mcomp(p_mcomp);

	comp.resize(p_row);				/* Redimension de filas de matriz. */

	for(i=0; i< comp.size(); i++)
	{
		comp[i].resize(p_col);		/* Redimension de columnas de matriz. */
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * METODOS DE COMPOENTES vector_o DE RDPG
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo analiza los componentes vector_o y retorna la referencia de uno de ellos si coincide con el id enviado por parametro.
 *
 * @param[in]  p_vcomp  Identificador de componente vector_o de la RDPG.
 *
 * @return     Se retorna la referencia de un objeto vector_o, si la operacion tiene exito.
 * 			   Si la operacion no tiene exito se acisa por el atributo error_code con el valor -EC_referenciaNula.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
vector_o& RDPG_Driver::ref_vcomp(ID_VCOMPONENT p_vcomp)
{
	error_code = EC_NULL;

	if((p_vcomp>=ID_VC_INIT) && (p_vcomp < ID_VC_END))
	{
		if(p_vcomp == _vMI)
			return vMI;
	}
	
	error_code = -EC_referenciaNula;
	cout<< "Error: En la RDPG: (" << name << ") no existe la referencia de vector_o indicado como parametro." << endl;
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Esta metodo se encarga de limpiar un componente vector_o de la RDPG.
 *
 * @param[in]  p_vcomp  Identificador del componente vector_o de la RDPG a limpiar.
 * 			 			Si no de encuentra el componente de la RDPG se avisa en el atributo error_code con -EC_componeteNoValido.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::clean_vector(ID_VCOMPONENT p_vcomp)
{
	size_t i;

	if((p_vcomp>=ID_VC_INIT) && (p_vcomp < ID_VC_END))
	{
		vector_o& comp = ref_vcomp(p_vcomp);
	
		for(i = 0; i<comp.size(); i++) {
				comp[i] = 0;
		}
	}
	else
	{

	}	
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo actualiza todos los elementos de un componente vector_o de una RDPG con el valor literal enviado por parametro.
 *
 * @param[in]  p_vcomp  Identificador del componente vector_o de la RDPG que se desea actualizar.
 * @param[in]  p_valor  Valor literal a asignar en cada uno de los elementos del componente vector_o de la RDPG.
 * 			 			Si no de encuentra el componente de la RDPG se avisa en el atributo error_code con -EC_componeteNoValido.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::reload_vector(ID_VCOMPONENT p_vcomp, int p_valor)
{
	size_t i;

	if((p_vcomp>=ID_VC_INIT) && (p_vcomp < ID_VC_END))
	{
		vector_o& comp = ref_vcomp(p_vcomp);

		for(i = 0; i<comp.size(); i++) {
					comp[i] = p_valor;
		}
	}
	else
	{

	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo imprime los elementos de un componente vector_o de acuerdo a las configuraciones de los atributos vdim, posVP y posVT.
 *
 * @param[in]  p_vcomp  Identificador del componente vector_o de la RDPG que se desea imprimir.
 * 			 			Si no de encuentra el componente de la RDPG se avisa en el atributo error_code con -EC_componeteNoValido.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::print_vcomp(ID_VCOMPONENT p_vcomp)
{
	size_t i, v;		/* v: elementos visualizados. */

	if(p_vcomp>=_vMI)
	{
		print_headerP();
		vector_o& comp = ref_vcomp(p_vcomp);

		cout << " "<< right << setw(5) << "|";

		for(i = posVP, v=0; (i<comp.size())&&(v<vdim); i++, v++) {
			if(i < 10)
				cout << right << setw( 3 ) << comp[i];
			else if(i < 100)
				cout << right << setw( 4 ) << comp[i];
			else if( i < MAX_TRANSITIONS)
				cout << right << setw( 5 ) << comp[i];
		}
		cout << endl;
	}
	else
	{

	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo se encarga de realizar una reasignacion de tamaño en los componentes vector_o del objeto RDPG de acuerdo al numero 
 * 			   de plazas y tranciones.  
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::resize_allvectors()
{
	vMI.resize(plazas);
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo copia todos los elementos de un vector origen hacia un vector destino.
 *
 * @param      v_dst  Referencia del objeto vector_o destino, sobre el cual se actualizan sus elementos.
 * @param[in]  v_src  Referencia del objeto vector_o origen, sobre el cual se copian sus elementos.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::cpy_vector(vector_o& v_dst, const vector_o& v_src)
{
	size_t i;
	if((!v_dst.empty()) && (!v_src.empty()) && (v_dst.size() == v_src.size()))
	{
		for(i=0; i< v_src.size(); i++)
			v_dst[i] = v_src[i];
	}
	else
		cout << " Falla en copia de vector." << endl;
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * METODOS DE DRIVER MATRIXMODG
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de realizar la conexion con el driver MatrixmodG por medio de su archivo de dispositivo en /proc.
 * 			   Si la conexion es exitosa se guarda el file descriptor para su acceso hasta finalizar la conexion.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::connect_driverMatrixmodG()
{
	/* Abrimos modulo matrixmodG*/
	my_fd = open("/proc/matrixmodG_fd", O_RDWR); /* abro file descriptor en modo escritura/lectura */

	if(my_fd<0 ) /* Verificacion de creacion. */
	{
		//error
		perror("	--> Error al abrir file descriptor /proc/matrixmodG_fd no existe.\n 	--> Finalizando programa.\n\n");
		disconnect_driverMatrixmodG();
		exit(1);
	}

	connect_driver = ENABLED_CONNECTION;
	if(MGUL_DB_MSG) cout << "\n	--> Se establecio conexion con driver exitosamente. \n";
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de desconectar el objeto RDPG_Driver del driver MatrixmodG cargado en el kernel. Luego de la desconexion con el
 * driver se elimina el objeto RDPG_Driver.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::disconnect_driverMatrixmodG()
{
	if(connect_driver == ENABLED_CONNECTION) 	/* Si conexion esta establecida con driver. Se finaliza. */
	{
		close(my_fd); 							/* Cierro connexion con file descriptor a file device de matrixmodG. */
		connect_driver = DISABLED_CONNECTION; 	/* Flag de conexion en estado deshabilitada. */
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de enviar una solicitud de creacion de una nueva RDPG al driver matrixmodG para que inicie la creacion de la RDPG
 * en el kernel de Linux. El envio de la solicitud se realiza escribiendo en el file device asociado al modulo MatrixmodG.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_createRDPG()
{
	if(!mII.empty()) /* Si existe como minimo la matriz de incidencia I, se crea RDPG en kernel.*/
	{
		char comando[N_CMD];
		memset(comando, '\0', N_CMD);// se limpia cadena
	
		// Se copia parametro a cadena comando
		strcpy(comando, cmd_createRDPG);
	
		char numeros[N_CMD];
		memset(numeros, '\0', N_CMD);// se limpia cadena
		
		// Se carga en cadena numeros los numeros enteros correspondientes a las plazas y transiciones de la RDPG
		sprintf(numeros, "%d_%d", plazas, transiciones);
	
		// Se almacena todo en cadena comando para dejarlo completo
		strcat(comando, numeros);
	
		// Se crea matriz en modulo con comando write
		(void)write_matrixmodG(comando);
	
		if(MGUL_DB_MSG) cout << "	--> Envio a driver comando:" << comando << endl;
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de enviar al driver MatrixmodG, los elementos de un componente matrix_o de la RDPG importada desde los archivos.
 * Los elementos a enviar se realizan desde uno de todos los componentes base de la RDPG, esto se indica mediante el parametro p_comp.
 * El envio de cada elemento del componente elegido se realiza escribiendo el comando de escritura al file device de driver MatrixmodG con los datos
 * requeridos de cada elemento en particular. Por ejemplo: 0_0_1, siendo fila_columna_valor donde fila es la posicion de fila de un componente matriz 
 * columna es la posicion de columna y valor es el valor que se desea asignar en la posicion indicada.
 *
 * @param[in]  p_comp       Numero entero que representa el componete de la RDPG que se desea enviar al driver MatrixmodG. Este entero no puede ser menor
 * que cero ni mayor que ID_MCOMP_END (ver enumeracion ID_MCOMPONENT).
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_addm(ID_MCOMPONENT p_comp)
{
	matrix_o& comp = ref_mcomp(p_comp); /* Se crea referencia temporal al componente matrix_o. */

	if(error_code == -EC_referenciaNula){
		cout << "   --> No se puede enviar elementos de componente matriz inexistente." << endl;
		return;
	}


	if(!comp.empty()) /* Si objeto fue importado previamente, se envia elementos asociados a driver. */
	{
		int i,j;
		char comando[N_CMD];
		memset(comando, '\0', N_CMD);// se limpia cadena
	
		/* Se copia parametro a cadena comando */
		strcpy(comando, cmd_addm[p_comp]);
		char valor_aux[N_CMD];
		memset(valor_aux, '\0', N_CMD);// se limpia cadena
			
		for(i = 0; i < comp.size(); i++)
		{
			for (j = 0; j < comp[0].size(); j++)
			{
				if(comp[i][j] != 0)
				{
					// Se carga en cadena_numeros los numeros enteros correspondientes a las filas y columnas
					sprintf(valor_aux, "%d_%d_%d", i, j, comp[i][j]);
	
					// Se almacena todo en el comando para dejarlo completo
					strcat(comando, valor_aux);
	
					//if(system_test_mode == 1)
					//	if(MGUL_DB_MSG) printf("	--> Envio a driver comando: %s\n", comando);
					//debug_check_point(&I,&MI,&H,&R,fd, comando);
					// Se crea matriz en modulo con comando write
					(void)write_matrixmodG(comando);
	
					memset(comando, '\0', N_CMD);// se limpia cadena
	
					// Se copia parametro a cadena comando
					strcpy(comando, cmd_addm[p_comp]);
					memset(valor_aux, '\0', N_CMD);// se limpia cadena
				}
			}
		}

		//cout << "	--> Envio de elementos de %s con exito.\n", p_DriverObj->component[p_comp]->name;
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de enviar al driver MatrixmodG, los elementos de un componente vector_o de la RDPG importada desde los archivos.
 * El envio de cada elemento del componente elegido se realiza escribiendo el comando de escritura al file device de driver MatrixmodG con los datos
 * requeridos de cada elemento en particular. Por ejemplo: 0_1, siendo posicion_valor donde posicion es la posicion del elemento a actualizar de un 
 * componente vector y valor es el valor que se desea asignar en la posicion indicada.
 *
 * @param[in]  p_comp       Identificado del componete vector de la RDPG que se desea actualizar enviandose al driver MatrixmodG. Este entero no puede ser menor
 * que cero ni mayor que ID_VCOMP_END (ver enumeracion ID_VCOMPONENT).
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_addv(ID_VCOMPONENT p_comp)
{
	vector_o& comp = ref_vcomp(p_comp); /* Se crea referencia temporal al componente matrix_o. */

	if(error_code == -EC_referenciaNula){
		cout << "   --> No se puede enviar elementos de componente vector inexistente." << endl;
		return;
	}


	if(!comp.empty()) /* Si objeto fue importado previamente, se envia elementos asociados a driver. */
	{
		int i;
		char comando[N_CMD];
		memset(comando, '\0', N_CMD);// se limpia cadena
	
		/* Se copia parametro a cadena comando */
		strcpy(comando, cmd_addv[p_comp]);
		char valor_aux[N_CMD];
		memset(valor_aux, '\0', N_CMD);// se limpia cadena
			
		for(i = 0; i < comp.size(); i++)
		{
			if(comp[i] != 0)
			{
				// Se carga en cadena_numeros los numeros enteros correspondientes a las filas y columnas
				sprintf(valor_aux, "%d_%d", i, comp[i]);

				// Se almacena todo en el comando para dejarlo completo
				strcat(comando, valor_aux);

				//if(system_test_mode == 1)
				//	if(MGUL_DB_MSG) printf("	--> Envio a driver comando: %s\n", comando);
				//debug_check_point(&I,&MI,&H,&R,fd, comando);
				// Se crea matriz en modulo con comando write
				(void)write_matrixmodG(comando);

				memset(comando, '\0', N_CMD);// se limpia cadena

				// Se copia parametro a cadena comando
				strcpy(comando, cmd_addv[p_comp]);
				memset(valor_aux, '\0', N_CMD);// se limpia cadena
			}
		}

		//cout << "	--> Envio de elementos de %s con exito.\n", p_DriverObj->component[p_comp]->name;
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de enviar el comando de confirmacion al driver MatrixmodG. Con este comando la RDPG del kernel se encarga de actualizar
 * todos sus componentes restantes a sus valores iniciales.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_confirmRDPG()
{
	if(!mII.empty())
	{
		(void)write_matrixmodG((char *)cmd_confirmRDPG); /* Confirmacion de componentes enviado a RDPG del kernel. La RDPG del kernel termina de calcular el resto de componentes.*/
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de enviar el comando para eliminar la RDPG del kernel de Linux. El envio del comando se realiza escribiendo en el
 * file device del driver MatrixmodG, el driver al detectarlo realiza la eliminacion de la RDPG. 
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_delRDPG()
{
	char cadena[N_CMD];
	memset(cadena, '\0', N_CMD);// se limpia cadena

	cout << "\n   ¿Desea eliminar la RDPG cargada en el kernel? (yes/no) ";
	scanf( "%s", cadena );

	if(strcmp(cadena, "yes") == 0)
	{
		cout << "	--> Envio a driver comando: "<< cmd_delRDPG << endl;

		// Se elimina RDPG en driver con comando write
		(void)write_matrixmodG((char *)cmd_delRDPG);
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de enviar el comando de solicitud de informacion de la RDPG cargada en el kernel de Linux. El envio del comando
 * se realiza mediante la escritura al file device del driver MatrixmodG. Luego de escribir dicho comando se realiza una lectura al file device el cual
 * devuelve la informacion solicitada en una cadena de espacio usuario.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_view_RDPGinfo()
{
	memset(libCadena, '\0', USR_BUF_SIZE);// se limpia cadena

	matrixmodG_view_compRDPG(view_infoName, libCadena);
    matrixmodG_view_compRDPG(view_infoPlaces, libCadena);
    matrixmodG_view_compRDPG(view_infoTransitions, libCadena);
    matrixmodG_view_compRDPG(view_infoShots, libCadena);
    matrixmodG_view_compRDPG(view_infoMemory, libCadena);
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de obtener el estado de todos los componentes gestionados por la RDPG cargada en el kernel de Linux. Para esto 
 * se escribe en el file device del driver MatricmodG el comando correspondiente a cada componente y se lo obtiene realizando la lectura al file device.
 * Intercalando una escritura y lectura al file device por cada componente de la RDPG se logra obtener todos los estados de componentes de la RDPG 
 * cargada en el kernel.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_view_allCompRDPG()
{
	int TIME_OFF =0;
	memset(libCadena, '\0', USR_BUF_SIZE);// se limpia cadena

	cout << "\n  Matriz de incidencia de la RDPG:\n";
	
	matrixmodG_view_compRDPG(view_mII, libCadena);

	cout << "\n  Matriz de incidencia de brazos inhibidores de la RDPG:\n" ;
	
    matrixmodG_view_compRDPG(view_mIH, libCadena);

    cout << "\n  Matriz de incidencia de brazos lectores de la RDPG:\n" ;
    
    matrixmodG_view_compRDPG(view_mIR, libCadena);

    cout << "\n  Matriz de transiciones reset de la RDPG:\n" ;
    
    matrixmodG_view_compRDPG(view_mIRe, libCadena);

    cout << "\n  Vectores de disparos posibles: \n" ;
    
	matrixmodG_view_compRDPG(view_mD, libCadena);

    cout << "\n  Marcado inicial de la RDPG:\n" ;
    
    matrixmodG_view_compRDPG(view_vMI, libCadena);

	cout << "\n  Marcado actual de la RDPG:\n" ;
	
    matrixmodG_view_compRDPG(view_vMA, libCadena);

    cout << "\n  Marcado nuevo de la RDPG:\n" ;
    
    matrixmodG_view_compRDPG(view_vMN, libCadena);

    cout << "\n  Vector Q de funcion cero: \n" ;
    
	matrixmodG_view_compRDPG(view_vQ, libCadena);

	cout << "\n  Vector W de funcion uno: \n" ;
	
	matrixmodG_view_compRDPG(view_vW, libCadena);

	cout << "\n  Vector E de transiciones sensibilizadas:\n" ;
	
    matrixmodG_view_compRDPG(view_vE, libCadena);

    cout << "\n  Vector B de transiciones des-sensibilizadas por arco inhibidor:\n" ;
    
    matrixmodG_view_compRDPG(view_vB, libCadena);

    cout << "\n  Vector L de transiciones des-sensibilizadas por arco lector:\n" ;
    
    matrixmodG_view_compRDPG(view_vL, libCadena);

    cout << "\n  Vector G de guardas de transiciones:\n" ;
    
    matrixmodG_view_compRDPG(view_vG, libCadena);

    cout << "\n  Vector A de transiciones de reseteo:\n" ;
    
    matrixmodG_view_compRDPG(view_vA, libCadena);

    cout << "\n  Vector UDT de resultado de ultimo disparo de transiciones:\n" ;
    
    matrixmodG_view_compRDPG(view_vUDT, libCadena);

	cout << "\n  Vector Ex de transiciones sensibilizadas extendido: \n" ;
	
	matrixmodG_view_compRDPG(view_vEx, libCadena);
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de obtener el estado de un componente de la RDPG cargada en el kernel. El componete sobre el cual se desea conocer
 * su estado se indica por parametro. Para enviar el comando se realiza una escritura al file device del driver MatrixmodG, para obtener la informacion 
 * del estado del componete se realiza una lectura al file device.
 *
 * @param[in]  cmd_INFO_ID  Macro representativa al numero en donde se encuentra el comando asociado del componente que se desea obtener su estado.
 * @param      cadena       Puntero a una cadena de caracter del espacio usuario donde se almacena la informacion proporcionada por el driver MatrixmodG
 * desde el file device asociado.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_view_compRDPG(int cmd_INFO_ID, char *cadena)
{
	char comando_auxwr[N_CMD];
	memset(comando_auxwr, '\0', N_CMD);// se limpia cadena

	if( (cmd_INFO_ID < ID_VIEW_END) && (cmd_INFO_ID >= ID_VIEW_INIT))
		strncpy(comando_auxwr, cmd_RDPGinfo[cmd_INFO_ID], strlen(cmd_RDPGinfo[cmd_INFO_ID]));
	else
	{
		cout << "LIB_MatrixmodG_error: Numero de comando indicado no existe en libreria. \n";
		return;
	}

	/* Write sobre driver */
	if(write_matrixmodG(comando_auxwr) == 0)
	{
		/* Read fichero */
		(void)read_matrixmodG(cadena);
	}

	cout <<  cadena << endl;
	
	memset(cadena, '\0', USR_BUF_SIZE);// se limpia cadena
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de enviar al driver MatrixmodG el comando de disparo de una transicion de la RDPG. El envio del comando se realiza
 * escribiendo en el archivo de dispositivo del driver MatrixmodG. La escritura sobre el archivo de dispositivo retorna 0 si el disparo fallo o 1 si el
 * disparo fue exitoso.
 *
 * @param      cadena       Numero de la transicion a disparar sobre la RDPG.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
int RDPG_Driver::matrixmodG_shoot_RDPG(int p_transicion)
{
	char cadena_read[N_CMD]; 						/* cadena para almacenar el resultado del disparo. */
	char comando[N_CMD]; 							/* comando para realizar disparo*/
	char s_transition[N_BYTES];						/* cadena de caracter que almacena el numero entero de una transicion en ascii. */
	memset(cadena_read, '\0', N_CMD);				/* se limpia cadena */
	memset(comando, '\0', N_CMD);					/* se limpia cadena */

	itoa(p_transicion, s_transition, N_BYTES);		/* Conversion de numero entero a ascii. */
			
	strcat(comando, cmd_shootRDPG); 				/* se agrega comando de disparo. */
	strcat(comando, s_transition);					/* se agrega cadena(nro transicion) al final de comando */

	if(MGUL_DB_MSG) printf("   Cadena ingresada: %s\n\n", comando);
	
	// Write sobre driver -> se realiza disparo
	//int k = write(my_fd, comando, strlen(comando));
	
	/*if(k)
	{
		cout << "Info: Se disparo exitosamente la transicion T" << p_transicion << "!!!\n";
	}
	else
		cout << "Info: Fallo disparo de transicion T" << p_transicion << "!!!\n";*/

	// indicamos a driver que muestre resultado de ultimo disparo
	/*string aux = "RDPGinfo shot_result\n";
	if(write_matrixmodG((char*)aux.c_str()) == 0)
	{
		// almacenamos cadena_read
		(void)read_matrixmodG(cadena_read);
		cout << "\n	-->	" << cadena_read;
	}*/

	/* Write sobre driver -> se realiza disparo */
	return write(my_fd, comando, strlen(comando));
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo envia el valor a configurar sobre una posicion del vector de guardas vG de la RDPG cargada en el kernel de Linux. Se realiza un
 * 			   envio de comando de escritura (write) sobre el driver MatrixmodG, enviado la posicion que se desea actualizar sobre el vector vG y el valor.
 *
 * @param[in]  p_transicion  Numero de la trnasicion asociada a inhibir. Coincide con la posicion a actualizar del vector vG.
 * @param[in]  p_valor       Valor con el que se actualiza el vector vG de la RDPG del kernel.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_set_vG(int p_transicion, int p_valor)
{
	char cmd[N_CMD];	 				/* comando de escritura a realizar en driver matrixmodG. */
	char s_pos_val[N_CMD];				/* cadena de caracter que almacena el numero entero de pos transicion y valor en ascii. */
	memset(cmd, '\0', N_CMD);			/* se limpia cadena */

	sprintf(s_pos_val, "%d_%d", p_transicion, p_valor);
	strcat(cmd, cmd_set_vG);	 		/* se agrega comando de seteo vG. */
	strcat(cmd, s_pos_val);				/* se agrega cadena(nro pos transicion) y valor de seteo al final de comando. */

	if(MGUL_DB_MSG) printf("   Cadena ingresada: %s\n\n", cmd);
	
	// Write sobre driver -> se realiza disparo
	(void)write_matrixmodG(cmd);
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo solicita un incremento de la cola de la variable de condicion asociada a la transicion a disparar enviada por parametro (vHQCV).
 *
 * @param[in]  p_transicion  Numero de transicion asociada a cola de variable de condicion que se desea incrementar.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_inc_vHQCV(size_t p_transicion)
{
	char cmd[N_CMD];	 				/* comando de escritura a realizar en driver matrixmodG. */
	char s_pos[N_CMD];					/* cadena de caracter que almacena el numero entero de pos transicion en ascii. */
	memset(cmd, '\0', N_CMD);			/* se limpia cadena */

	sprintf(s_pos, "%d", p_transicion);
	strcat(cmd, cmd_inc_vHQCV);	 		/* se agrega comando de incremento de vHQCV. */
	strcat(cmd, s_pos);					/* se agrega cadena(nro pos transicion) y al final de comando. */

	if(MGUL_DB_MSG) printf("   Cadena ingresada: %s\n\n", cmd);
	
	// Write sobre driver -> se realiza disparo
	(void)write_matrixmodG(cmd);
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo solicita un decremento de la cola de la variable de condicion asociada a la transicion a disparar enviada por parametro (vHQCV).
 *
 * @param[in]  p_transicion  Numero de transicion asociada a cola de variable de condicion que se desea decrementar.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::matrixmodG_dec_vHQCV(size_t p_transicion)
{
	char cmd[N_CMD];	 				/* comando de escritura a realizar en driver matrixmodG. */
	char s_pos[N_CMD];					/* cadena de caracter que almacena el numero entero de pos transicion en ascii. */
	memset(cmd, '\0', N_CMD);			/* se limpia cadena */

	sprintf(s_pos, "%d", p_transicion);
	strcat(cmd, cmd_dec_vHQCV);	 		/* se agrega comando de decremento de vHQCV. */
	strcat(cmd, s_pos);					/* se agrega cadena(nro pos transicion) y al final de comando. */

	if(MGUL_DB_MSG) printf("   Cadena ingresada: %s\n\n", cmd);
	
	// Write sobre driver -> se realiza disparo
	(void)write_matrixmodG(cmd);
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo escribe un comando al DDL matrixmodG, para determinar si la RDPG del kernel se encuentra vacia o no.
 *
 * @return     El retorno sera:
 * 				* 0 (false): si la RDPG del kernel se encuentra cargada y no vacia.
 * 				* 1 (true): si la RDPG del kernel se encuetra vacia (no cargada) en el kernel.
 * 				* -EC_falloEmpty: si se produjo un error de escritura con el CMD del DDL MatrixmodG.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
int RDPG_Driver::matrixmodG_empty()
{
	int rt_fempty = write(my_fd, cmd_RDPGinfo[view_infoEmpty], strlen(cmd_RDPGinfo[view_infoEmpty]));
	
	if(rt_fempty)
		return 1;
	else if( rt_fempty == 0)
		return 0;

	return -EC_falloEmpty;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * @brief      Este metodo realiza la conversion de un entero a un ascii en una cadena de caracteres de tamaño p_len enviado por parametro.
 *
 * @param[in]  p_entero  Numero entero que se desea convertir a ascii.
 * @param      p_str     Puntero a la cadena de caracter donde se almacena la conversion del entero a ascii.
 * @param[in]  p_len     Longitud de la cadena en la que se almacena la conversion.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void RDPG_Driver::itoa(int p_entero, char *p_str, size_t p_len)
{
	memset(p_str, '\0', p_len);// se limpia cadena

	if(sprintf(p_str, "%d", p_entero) < 0) /* Si falla conversion. */
	{
		strncpy(p_str, "?", p_len); /* Se indica con un ? (signo de pregunta) que no se dectecto el entero. */
		cout << "LIBMatrixmodG_error: Falla en conversion de entero a ascci (funcion itoa()). \n";
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de escribir sobre el archivo de dispositivo (o file device) del driver MatrixmodG. La escritura se realiza utilizando
 * el file descriptor que contiene el objeto RDPG_Driver enviado por parametro para establecer la conexion con el driver.
 *
 * @param      cadena       Puntero a la cadena de caracteres que se desean escribir en el file device del driver MatrixmodG.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
int RDPG_Driver::write_matrixmodG(char *cadena)
{	
	int id_thread;	
    
    /* Write a file device de driver matrixmodG. */
	if (write(my_fd, cadena, strlen(cadena)) != strlen(cadena))
	{
		printf("Error de escritura sobre driver\n");
		/* Close de files descriptors */
		disconnect_driverMatrixmodG(); /* Libera memoria utilizada y cierra la conexion con el driver (close(my_fd)). */
		exit(1);
	}

    return 0;
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------------*
 * @brief      Este metodo se encarga de leer el archivo de dispositivo (o file device) del driver MatrixmodG. La lectura se realiza utilizando el codigo 
 * del file descriptor que contiene el objeto RDPG_Driver enviado por parametro con el que se establece la conexion con el driver.
 *
 * @param      cadena       Puntero a la cadena de caracteres donde se almacena la lectura brindada por el driver MatrixmodG mediante el file device.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
int RDPG_Driver::read_matrixmodG(char *cadena)
{
	int id_thread;

    /* Read file device de driver MatrixmodG */
	if(read(my_fd, cadena, USR_BUF_SIZE) < 0)
	{
		printf("Error de lectura sobre driver\n");
		/* Close de files descriptors */
		disconnect_driverMatrixmodG(); /* Libera memoria utilizada y cierra la conexion con el driver (close(my_fd)). */
		exit(1);
	}

    return 0;
}