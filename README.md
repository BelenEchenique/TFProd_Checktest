
># Automatización chequeo eléctrico

Este código tiene como fin optimizar el chequeo eléctrico para el FES. La automatización se lleva a cabo a través de la programación de una herramienta (_ToolFES_), que cuenta con la integración de un microcontrolador Teensy 3.5 y un microcontrolador ATSAM integrados. A través del contacto de la herramienta con el FES mediante \textit{Pogo pins}, esta es capaz de leer los voltajes correspondientes al chequeo eléctrico mediante ADCs. La herramienta se complementa  con una interfaz gráfica que permite mostrar los valores de los voltajes obtenidos y las evaluaciones de cada punto de prueba. 

El objetivo de este proyecto es optimizar el proceso de testeo eléctrico que hasta la fecha se realiza manualmente.

## Tests a automatizar

A continuación se muestra una imagen con los puntos de medición en el FES y una descripción de cada uno. 

![Testpoints](https://github.com/BelenEchenique/TFProd/blob/main/Esquematicos/TEST%20POINT%20Y%20PAD%20DE%20CONECCIONES%20FES%20V4.6%20%283%29.png)

 - TP1: referencia para batería. Toma el valor de tierra, por lo que no se mide. 
 - TP2: voltaje de la batería. El rango correcto inicial está configurado entre 3.3 y 4.2V.
 - TP3: tierra. Se utiliza como referencia.
 - TP4: por defecto, 3.3V
 - TP5: por defecto, 3.3V
 - TP6: este punto corresponde a la elevación de voltaje al momento de estimular con el FES. El valor correcto al que debería elevarse el voltaje en un FES corresponde a 170V, mientras que si se trata de un WalkFES, corresponde a 150V. 
 - TP7: canales de estimulación (12). Se determinó que esta medición no es necesaria.
 - TP8: señal de voltaje. La medición correcta en este punto debería un valor superior a 1V. Este punto está pendiente.  
 - TP9: medición correspondiente a la mitad del valor de la batería. Si el valor de la batería es incorrecto, no se realiza esta medición. 
 - TP10: medición correspondiente a un valor mayor a 6V. 
   

## Requisitos para llevar a cabo el chequeo
Para poder llevar a cabo el chequeo se debe realizar lo siguiente:

1. Conectar la _ToolFES_ a un puerto USB 
2. Buscar el número de puerto al cual se conectó (se puede buscar en el administrador de dispositivos)

## Funcionamiento interfaz
 
El flujo de la interfaz se puede observar en el diagrama a continuación.

![Flujo](https://github.com/BelenEchenique/TFProd/blob/main/Interface/Flujo.PNG)

  
## Cambio de variables

Para cambiar los rangos aceptables para un _testpoint_, se debe abrir el archivo _default_values.csv_ y reemplazar los valores estándar. Los porcentajes de tolerancia para cada caso se pueden cambiar de la misma manera. 

Para cambiar los valores de los resistores, se debe abrir el archivo _Codigos/util_fes.h_ y acudir a la linea 89. Las líneas que siguen indican el número y valor de cada resistencia. 


## Como debuggear

	make o ./build_all.bat en carpeta
	
	1. Abrir powershell, abre server debugger
	pyocd gdb --target k64f 
	2. Abrir segundo powershell arm-none-eabi-gdb <file.elf>
	3. target remote :3333
	4. Si nuevo programa -> load continue
	5. RESET = monitor reset halt -> continue
	6. 