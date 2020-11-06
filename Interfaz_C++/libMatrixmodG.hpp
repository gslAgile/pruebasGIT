/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * Libreria C++ de Device Driver Linux MatrixmodG (archivo cabecera - header file).
 * 
 * 
 * Esta libreria contiene las declarciones y definiciones de la clase RDPG_Driver para gestionar Redes de Petri Generalizadas (RDPG) en el espacio usuario
 * con ayuda del espacio kernel mediante la interaccion de llamadas al sistema con el file device "/proc/matrixmodG_fd" del Device Driver MatrixmodG.
 * 
 * Todas las funciones son transparentes para el usuario final, haciendo uso de la clase RDPG_Driver para la gestion de objetos RDPG en el kernel desde 
 * el espacio usuario.
 * 
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef RDPG_ULIBCPP_H
#define RDPG_ULIBCPP_H /* ULIB: por biblioteca de espacio usuario. */

#include <fstream>
#include <sstream>				/* stringstream: include <istringstram> and <ostringstream> (lectura y escritura de string). */
#include <iomanip>				/* incluye funciones: setw(), setfill(), etc. */
#include <iostream>
#include <vector>
#include <fcntl.h>      		/* Modos de apertura */
#include <stdlib.h>     		/* Funciones de ficheros */
#include <unistd.h>
#include <string.h>
using namespace std;


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * DEFINICION DE CLASES OPACAS PARA COMPONENTES DE RDPG: Se renombran las clases de objetos para estandarizar los componentes matrix_o y vector_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
typedef vector< vector<int> > matrix_o;
typedef vector<int> vector_o;

#define MGUL_DB_MSG 		0		/* Macro de mensajes de debug para de Lib user de driver MatrixmodG. 0: Deshailitada, 1: Habilitada. */
#define MAX_PLACES			1000	/* Numero maximo de plazas soportadas por aplicacion. */
#define MAX_TRANSITIONS 	1000	/* Numero maximo de transiciones soportadas por aplicacion. */
#define MAX_VDIM			50		/* Numero entero maximo de plazas y transiciones a visualizar en una lectura de componentes de una RDPG_o. */
#define MIN_VDIM			10		/* Numero entero maximo de plazas y transiciones a visualizar en una lectura de componentes de una RDPG_o. */

#define USR_BUF_SIZE		16384 	/* 2 ^ 14 */
#define N_CMD 				256
#define N_BYTES				10		/* Numero de bytes configurables en una cadena que muestra enteros. */

/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * ID_MCOMPONENT: Enumerador de los identificadores de los componentes matrices (matrix_o) de una RDPG_o.
 *
 * @note 	Los componentes de una RDPG_o pueden ser vectores o matrices. Se normaliza los identificadores de componentes matriz que 
 * comienzan con mX donde:
 * m: es una letra fija que indica en el nombre de la enumeracion que se asocia a una matriz (matrix_o).
 * X: puede ser cualquier nombre para completar el nombre del componente matrix_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
enum ID_MCOMPONENT{
	ID_MC_INIT = 0,					/**< Numero que indica el inicio de las enumeraciones en ID_MCOMPONENT. */
	_mII = 0,						/**< Identificador de la matriz de incidencia I. */
	_mIH,							/**< Identificador de la matriz de incidencia H. */
	_mIR,							/**< Identificador de la matriz de incidencia R. */
	_mIRe,							/**< Identificador de la matriz de incidencia Re. */
	ID_MC_END						/**< Numero que indica el fin de las enumeraciones en ID_MCOMPONENT. */
};


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * ID_VCOMPONENT: Enumerador de los identificadores de los componentes vectores (vector_o) de una RDPG_o.
 *
 * @note 	Los componentes pueden ser vectores o matrices, Se normaliza los identificadores de componentes vector que 
 * comienzan con vX donde:
 * v: es una letra fija que indica en el nombre de la enumeracion que se asocia a un vector.
 * X: puede ser cualquier nombre para completar el nombre del componente vector_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
enum ID_VCOMPONENT{
	ID_VC_INIT = 0, 				/**< Numero que indica el fin de las enumeraciones en ID_VCOMPONENT.*/
	_vMI = 0,						/**< Identificador del vector de marcado inicial. */
	_vG,							/**< Identificador del vector de guardas. */
	ID_VC_END						/**< Numero que indica el fin de las enumeraciones en ID_VCOMPONENT.*/
};


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * ID_VIEWCOMPONENT: 	Enumerador de los identificadores de los comandos para visualizar informacion de la RDPG o de sus componentes.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
typedef enum{
	ID_VIEW_INIT = 0,				/**< Numero que indica el inicio de las enumeraciones en ID_COMPONENT.*/
	view_infoName=0,				/**< Identificador de comando que permite ver el nombre de la RDPG gestionada por driver.*/
	view_infoPlaces,				/**< Identificador de comando que permite ver el numero de plazas de la RDPG.*/
	view_infoTransitions,			/**< Identificador de comando que permite ver el numero de transiciones. */
	view_infoShots,					/**< Identificador de comando que permite ver el numero de disparos.*/
	view_infoMemory,				/**< Identificador de comando que permite ver la memoria utilizada por RDPG. */
	view_infoComponent,				/**< Identificador de comando que permite ver informacion asociada a componente seleccionado. */
	view_infoEmpty,					/**< Identificador de comando que permite conocer si la RDPG del kernel esta cargada o vacia. */
	view_mII,						/**< Identificador de comando que permite ver matriz de incidencia I. */
	view_mIH,						/**< Identificador de comando que permite ver la matriz de incidencia H. */
	view_mIR,						/**< Identificador de comando que permite ver la matriz de incidencia R. */
	view_mIRe,						/**< Identificador de comando que permite ver la matriz de incidencia Re. */
	view_mD, 						/**< Identificador de comando que permite ver la matriz de disparos D. */
	view_vMI, 						/**< Identificador de comando que permite ver el vector de marcodo inicial. */
	view_vMA,						/**< Identificador de comando que permite ver el vector de marcado actual. */
	view_vMN,						/**< Identificador de comando que permite ver el vector de marcado nuevo. */
	view_vQ,						/**< Identificador de comando que permite ver el vector Q. */
	view_vW,						/**< Identificador de comando que permite ver el vector W. */
	view_vE, 						/**< Identificador de comando que permite ver el vector E. */
	view_vB,						/**< Identificador de comando que permite ver e de comando que permite ver el vector B. */
	view_vL,						/**< Identificador de comando que permite ver el vector L. */
	view_vG,						/**< Identificador de comando que permite ver el vector G. */
	view_vA,						/**< Identificador de comando que permite ver el vector A. */
	view_vUDT,						/**< Identificador de comando que permite ver el vector UDT. */
	view_vEx, 						/**< Identificador de comando que permite ver el vector Ex. */
	view_vHQCV,						/**< Identificador de comando que permite ver el vector vHQCV. */
	view_vHD, 						/**< Identificador de comando que permite ver el vector vHD. */
	ID_VIEW_END						/**< Numero que indica el fin de las enumeraciones en ID_VIEWCOMPONENT.*/
}ID_VIEWCOMPONENT;


/**---------------------------------------------------------------------------------------------------------------------------------------------------------
 * RDPG_SHOT_RESULT: Enumerador de los resultados que puede devolver el ultimo disparo realizado de una RDPG_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
enum RDPG_SHOT_RESULT{
	SHOT_INIT=0,					/* Resultado cuando no se realizaron disparos en una RDPG_o. */
	SHOT_OK = 1, 					/* Resultado cuando el ultimo disparo realizado de una RDPG_o fue exitoso. */
	SHOT_FAILED=-1					/* Resultado cuando el ultimo disparo realizado de una RDPG_o no fue exitoso. */
} ;


/**---------------------------------------------------------------------------------------------------------------------------------------------------------
 * SHOT_MODE: Enumerador de los modos en que se puede disparar las transiciones de una red RDPG_o afectada.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
enum SHOT_MODE{
	SHOT_MODE_E = 1, 				/* Modo de disparo explicito, con este modo seleccionado en la red RDPG_o afectada, impactan los cambios de un disparo. Los vectores de maracado MN y MA de la red se veran afectados.*/
	SHOT_MODE_I						/* Modo de disparo implicito, con este modo seleccionado en la red RDPG_o afectada, no impactan los cambios de un disparo. Se conoce que sucede al disparar una transicion de la red pero los vectores de maracado MN y MA no se veran afectados.*/
} ;


/**---------------------------------------------------------------------------------------------------------------------------------------------------------
 * ST_TRANSITION: Enumerador de los estados en los que puede estar una transicion de una RDPG_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
enum ST_TRANSITION{
	ST_NO_SENSITIZED=0,				/* Estado de transicion no sensibilizada.*/	
	ST_SENSITIZED = 1 				/* Estado de transicion sensibilizada.*/	
} ;

/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * ST_CONNECTION: 	Enumerador de los estados posibles que puede tener un objeto DriverRDPG_o al momento de establecer la coneccion con el driver MatrixmodG.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
enum ST_CONNECTION{
	DISABLED_CONNECTION=0,			/* La conexion con el driver MatrixmodG no tuvo exito. */
	ENABLED_CONNECTION				/* La conexion con el driver MatrixmodG fue exitosa. */
};


/**---------------------------------------------------------------------------------------------------------------------------------------------------------
 * ERRORS_CODES: Enumerador de los codigos de eerores posibles de una RDPG_o.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
enum ERRORS_CODES{
	EC_NULL = 0,					/* Codigo cuando no se produjo ningun error. */
	EC_dobleCreacionDeRDPG,			/* Error que se ocaciona cuando se intenta crear dos o mas veces una misma RDPG_o.*/	
	EC_dobleEliminacionDeRDPG,		/* Error que se ocaciona cuando se produce la eliminacion de una RDPG que ya se elimino.*/	
	EC_transicionInexistente,
	EC_falloPosVP,
	EC_falloPosVT,
	EC_falloVdim,
	EC_extraccionDato,
	EC_datoIncorrecto,
	EC_componenteNoCreado,
	EC_CodigoCatComp,
	EC_posicionIncorrecta,
	EC_referenciaNula,
	EC_fileReadFail,
	EC_falloEmpty
};


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * DEFINICION DE CLASE RDPG: Clase de objeto RDPG gestionado por libreria.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
class RDPG_Driver
{
private:
  	/* Variables de objeto RDPG: identificaciones de RDPG_o y variables de estados de objeto RDPG_o */
	string name;					/* Nombre del objeto RDPG. */
	int obj_id;						/* Identificador del objeto RDPG. */
	int my_fd;						/* numero entero que almacena numero de file descriptor del archivo de dispositivo del driver /proc/matrixmod. */
	int connect_driver;
	int shot_result;				/* Resultado de ultimo disparo realizado en la RDPG_o. Ver enumeracion RDPG_SHOT_RESULT. */
	int error_code;					/* Codigo del ultimo error sucedido. */
	bool kernel_RDPG_sec;			/* Variable de proteccion de RDPG del kernel. Si la variable es false la RDPG del kernel sera elimina al finalizar el programa C++.*/
	size_t posVP;					/* Posicion de la vista de plazas. */
	size_t posVT;					/* Posicion de la vista de transiciones. */
	size_t vdim;					/* Dimension de visualizacion de componentes. Es el numero de plazas y transiciones a visualizar en una lectura (read) al driver. */
	size_t plazas;					/* Numero de plazas de la RDP en caracteres. */
	size_t transiciones;			/* Numero de transiciones de la RDPG. */
	size_t size;					/* Numero de bytes reservados por la RDPG. */


	/* Variables de objeto RDPG: componentes matrices de RDPG_o */
	matrix_o mII;    				/* Matriz de incidencia I. */
	matrix_o mIH;    				/* Matriz de incidencia H asociada a los brazos inhibidores. */
	matrix_o mIR;    				/* Matriz de incidencia R asociada a los brazos lectores. */
	matrix_o mIRe;   				/* Matriz de incidencia Re asociada a los arcos reset. */
	

	/* Variables de objeto RDPG: componentes vector de RDPG_o */
	vector_o vMI;  					/* Vector de marcado inicial. */


/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * DECLARACION DE METODOS PRIVADOS DE Objeto RDPG_Driver: Conjunto de metodos gestionados solo por libreria RDPG_object.hpp/cpp.
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
/* Metodos de objeto RDPG */
	void import_matrix(matrix_o&, string p_fname);
	void import_vector(vector_o&, string p_fname);
	void print_headerT();
	void print_headerP();
	void print_line(const char p_c, int p_tam=80) const {cout << setw( p_tam ) << setfill( p_c ) << '\n' << setfill( ' ' ); }
	
	/* Metodos privados para componentes matrix_o de RDPG. */
	void identity_matrix(ID_MCOMPONENT);
	void clean_matrix(ID_MCOMPONENT);
	void reload_matrix(ID_MCOMPONENT, int);
	void resize_allmatrixs();
	void resize_matrix(ID_MCOMPONENT, size_t, size_t);

	/* Metodos privados para componentes vector_o de RDPG. */
	void clean_vector(ID_VCOMPONENT);
	void reload_vector(ID_VCOMPONENT, int);
	void resize_allvectors();
	void cpy_vector(vector_o&, const vector_o&);

/*---------------------------------------------------------------------------------------------------------------------------------------------------------
 * DECLARACION DE METODOS PUBLICOS DE Objeto RDPG_Driver
 *---------------------------------------------------------------------------------------------------------------------------------------------------------*/
public:
	/* Variables globales. */
	static int n_objects;											/* Numero de todos los objetos instanciados. */
	
	/* Constructores */
	RDPG_Driver(string , size_t, size_t);
	RDPG_Driver(string, string, string, string, string, string);	/* Constructor personalizado version 2. */

	/* Destructor */
	~RDPG_Driver();

	/* Getters: Metodos inline */
	size_t get_fileLines(string);
	size_t get_lineElements(string);
	string get_name() const { return name; }
	int get_objID() const { return obj_id; }
	int get_errorCode() const { return error_code; }
	size_t get_posVP() const { return posVP; }
	size_t get_posVT() const { return posVT; }
	size_t get_vdim() const { return vdim; }
	size_t get_size() const { return size; }
	size_t get_plazas() const { return plazas; }
	size_t get_transiciones() const  { return transiciones; }
	int get_TokensPlace(size_t);
	int get_vHDelement(size_t);
	int get_sensitized(size_t);
	bool empty();

	/* Setters */
	void set_kernel_RDPG_sec(bool value);

	/* Metodos de objeto RDPG */
	void import_RDPG(string p_mII, string p_mIH, string p_mIR, string p_mIRe, string p_vMI);
	void add_values_mcomp(matrix_o&, string, size_t);
	void add_values_vcomp(vector_o&, string);
	void print_allComp() const;
	void print_RDPGinfo() const;
	
	/* Metodos publicos para componentes matrix_o de RDPG. */
	matrix_o& ref_mcomp(ID_MCOMPONENT);
	void print_mcomp(ID_MCOMPONENT);

	/* Metodos publicos para componentes vector_o de RDPG. */
	vector_o& ref_vcomp(ID_VCOMPONENT);
	void print_vcomp(ID_VCOMPONENT);

	/* Metodos del driver MatrixmodG: */
	void connect_driverMatrixmodG();
	void disconnect_driverMatrixmodG();
	void matrixmodG_createRDPG();
	void matrixmodG_addm(ID_MCOMPONENT p_comp);
	void matrixmodG_addv(ID_VCOMPONENT p_comp);
	void matrixmodG_delRDPG();
	void matrixmodG_confirmRDPG();
	void matrixmodG_view_RDPGinfo();
	void matrixmodG_view_allCompRDPG();
	void matrixmodG_view_compRDPG(int,char *);
	int matrixmodG_shoot_RDPG(int);
	void matrixmodG_set_posVP(size_t);
	void matrixmodG_set_posVT(size_t);
	void matrixmodG_set_vdim(size_t);
	void matrixmodG_set_vG(int, int);
	void matrixmodG_inc_vHQCV(size_t);
	void matrixmodG_dec_vHQCV(size_t);
	int matrixmodG_empty();
	void itoa(int p_entero, char *p_str, size_t p_len);

	/* Llamadas al sistema sobre driver matrixmodG. */
	int write_matrixmodG(char *cadena);
	int read_matrixmodG(char *cadena);

};


#endif /* RDPG_ULIBCPP_H */