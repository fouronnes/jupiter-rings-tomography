all:
	g++ getjuno.cpp -I ../cspice/cspice/include ../cspice/cspice/lib/cspice.a -lm -o getjuno
