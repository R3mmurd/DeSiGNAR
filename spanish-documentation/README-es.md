# DeSiGNAR (Data Structures GeNeral librARy)

Este es el comienzo de un proyecto completo el cual se espera sea
una biblioteca general para ser usada en el desarrollo de modelos
y como herramienta para simulaciones.

La estructura de esta librería es, por directorios:


| Directorio        | Descripción|
| :-------------: |:-------------|
| *include*     | Contiene todos los archivos de cabecera. |
| *src*      | Contiene todos los archivos fuente (implementaciones declaradas en las cabeceras)|
| *samples* | Contiene algunos test y demos con instrucciones de uso de las distintas abstracciones implementadas. |
| *obj* | En este directorio serán creados todos los archivos objeto cuando sea compilada la biblioteca.|
| *lib* | Cuando se compila la biblioteca, en este directorio será creado el archivo **libDesignar.a**.|
| *bin* | Cuando sea compilada la biblioteca, en este directorio serán creados los archivos binarios a ejecutar.|

Para compilar la biblioteca, se debe ejecutar el comando: 

```shell

~$ make library

```


Para compilar los samples, se debe ejecutar el comando:

```shell

~$ make samples

```
