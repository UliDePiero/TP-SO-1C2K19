# tp-2019-1c-GGWP

1) Desde la consola, ejecutar:

	cd         		// para pararnos en home/utnso/
	
	leafpad .bashrc		// para abrir el editor de texto en .bashrc
	
	Ahí, ir a la última linea del archivo, y setear la variable LD_LIBRARY_PATH. Esta última línea tiene que decir:
	
	 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2019-1c-GGWP/BiblioCompartida/Debug

	Para linkear también las commons debemos agregar dos rutas como por ejemplo: 
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspace/tp-2019-1c-GGWP/BiblioCompartida/Debug/:~/workspace/so-commons-library/src/commons/Debug/
2) Verificar que cada ejecutable haya quedado bien linkeado a la BiblioCompartida

3) Desde la consola, en la carpeta Debug de cada proyecto, ejecutar ldd Ejecutable y ver como está libBiblioCompartida.so

