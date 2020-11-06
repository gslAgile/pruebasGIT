# Makefile de APP C++
# 
# Compilacion con dos lineas para soporte al multiprocesamiento de hilos (pthread):
# g++ -c ../RDPG_object.cpp ../monitor.cpp system_tests.cpp -pthread -std=c++11 -fopenmp  # generate target object file
# g++ RDPG_object.o monitor.o system_tests.o -o system_tests.out -pthread -std=c++11 	-fopenmp # link to target binary

#REGLAS
make:	
	g++ ../libMatrixmodG.cpp ../monitor.cpp ../monitorCA2.cpp system_tests.cpp -o test.out -pthread -std=c++0x -fopenmp
clean:
	rm -rf *.out
	rm -rf *~
