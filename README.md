# Aruco Marker Detector

Esta aplicación es un sistema de detección de marcadores Aruco que utiliza OpenCV para capturar imágenes de una cámara, detectar marcadores y enviar los datos a un servidor. Además, la aplicación detecta intrusos basándose en la posición de las personas dentro de un área segura definida por los marcadores Aruco, y reproduce un archivo de sonido `.wav` cuando se detecta un intruso.

## Requisitos

Para compilar y ejecutar esta aplicación, necesitarás:

- **Windows** (ya que utiliza `PlaySound` y Winsock)
- **Microsoft Visual Studio** (o cualquier compilador C++ compatible en Windows)
- **OpenCV 4.8** con los módulos de contribución (para detección de Aruco)
- **Windows Media Player** (si se usa la versión con soporte MP3)
- **Una cámara web** conectada al equipo
- **Socket con el modelo precargado** se usa un modelo de tensorflor hub (ssd_mobilnet) en Python el cual se comunica por un socket

## Dependencias

La aplicación requiere las siguientes bibliotecas y herramientas:

- **OpenCV** (para el procesamiento de imágenes y detección de marcadores)
- **Winsock2** (para la comunicación de red)
- **Windows API** (`PlaySound`) para reproducir archivos de sonido `.wav`
- **ATL y Windows Media Player SDK** (opcional, solo si deseas usar MP3 en lugar de WAV)

## Instalación

1. **Instalar OpenCV**:
   - Descarga e instala OpenCV 4.8 desde [aquí](https://opencv.org/releases/).
   - Configura las variables de entorno y enlaces en tu proyecto para que apunten a las bibliotecas y archivos de encabezado de OpenCV.

2. **Configurar el proyecto en Visual Studio**:
   - Incluye las bibliotecas `ws2_32.lib` y `winmm.lib` en la configuración del enlazador.
   - Agrega los encabezados de OpenCV y las bibliotecas requeridas en las opciones de compilación.

3. **Coloca los archivos de sonido**:
   - Asegúrate de que el archivo de sonido `alarma.wav` esté en el mismo directorio que el ejecutable o proporciona la ruta correcta en el código.

## Uso

1. **Compilación**:
   - Abre el proyecto en Visual Studio y compílalo.
   - Asegúrate de que todas las dependencias estén correctamente configuradas.

3. **Ejecución**:
   - Ejecutar el archivo de Python con el modelo precargado de tensorflow hub
   - Esperar a que nos indique que esta abierto a la espera de conexión del socket
   - Ejecuta el archivo binario generado.
   - La aplicación te pedirá que selecciones una cámara disponible.
   - Una vez seleccionada, la aplicación comenzará a capturar imágenes y a buscar marcadores Aruco en tiempo real.
   - Si se detectan suficientes marcadores, se definirá un área segura.
   - Si una persona es detectada dentro del área segura, la aplicación enviará una alerta al servidor y reproducirá el sonido `alarma.wav`.


## Limitaciones

- Esta aplicación está diseñada para sistemas Windows debido al uso de APIs específicas como `PlaySound` y Winsock.
- Solo soporta archivos de sonido en formato `.wav`. Para soporte de MP3, es necesario agregar dependencias adicionales.
- Reconocimiento de personas muy lento, lo mejor seria usar la GPU para el modelo

## Contribuciones

Si tienes sugerencias o deseas contribuir a la mejora de esta aplicación, eres bienvenido a hacer un fork del repositorio y enviar pull requests.

## Licencia

Este proyecto está licenciado bajo la Licencia MIT. Consulta el archivo `LICENSE` para más detalles.
