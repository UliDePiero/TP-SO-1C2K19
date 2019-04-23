# tp-2019-1c-GGWP

1) Desde la consola, ejecutar:

	cd         		// para pararnos en home/utnso/
	
	leafpad .bashrc		// para abrir el editor de texto en .bashrc
	
	Ahi, ir a la ultima linea del archivo, y setear la variable LD_LIBRARY_PATH. Esta ultima linea tiene que decir:
	
	 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2018-2c-Intel-y-Gentes/BiblioCompartida/Debug

2) Verificar que cada ejecutable haya quedado bien linkeado a la BiblioCompartida

3) Desde la consola, en la carpeta Debug de cada proyecto, ejecutar ldd Ejecutable y ver como esta libBiblioCompartida.so

