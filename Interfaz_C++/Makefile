# Makefile de APP C++
# 
# gcc -fopenmp ../Librerias_usuario/matrices.c ../Librerias_usuario/libMatrixmodG.c -o IUTS_matrixmod.out

#VARIABLES

#REGLAS
make:
	g++ libMatrixmodG.cpp monitor.cpp monitorCA2.cpp app.cpp -o app.out -pthread -std=c++0x -fopenmp
clean:
	rm -rf *.out
	rm -rf *~
