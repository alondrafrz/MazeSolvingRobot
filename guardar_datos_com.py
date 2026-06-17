import serial
import csv
import time

# 1. Configuración del puerto (Cambia 'COM3' por el puerto real de tu placa)
PUERTO_COM = 'COM6' 
BAUDIOS = 115200
NOMBRE_ARCHIVO = r"C:\Users\frzal\OneDrive\Documentos\Arduino\RobotSEguidorLinea\datosCom_Recorrido3.csv"

try:
    # 2. Abrir la conexión serial
    # El 'timeout=1' evita que el programa se quede congelado si la placa deja de enviar datos
    arduino = serial.Serial(PUERTO_COM, BAUDIOS, timeout=1)
    time.sleep(2) # Pausa técnica para dejar que el microcontrolador se reinicie al conectar
    print(f"Conectado exitosamente al puerto {PUERTO_COM}")

    # 3. Crear y abrir el archivo CSV para escribir
    # 'newline=""' previene que Windows agregue saltos de línea dobles innecesarios
    with open(NOMBRE_ARCHIVO, mode='w', newline='', encoding='utf-8') as archivo_csv:
        escritor_csv = csv.writer(archivo_csv)
        
        # Escribimos los encabezados de las columnas
        escritor_csv.writerow(["UFL", "UFC", "UFR", "Vel_Izq", "Vel_Der"])
        print(f"Guardando datos en '{NOMBRE_ARCHIVO}'... Presiona Ctrl+C para detener.")

        # 4. Bucle infinito para capturar las lecturas en tiempo real
        while True:
            if arduino.in_waiting > 0: # ¿Hay datos esperando en el buffer?
                # Leer la línea completa proveniente del ESP32/Arduino
                linea = arduino.readline()
                
                # 'decode' transforma los bytes crudos en texto (string)
                # 'strip' remueve los saltos de línea (\n o \r) de los extremos
                linea_limpia = linea.decode('utf-8', errors='ignore').strip()
                
                if linea_limpia: # Si la línea no está vacía
                    # Separamos la cadena por comas para convertirla en una lista de Python
                    # Ejemplo: "135,120,140,150,135" -> ["135", "120", "140", "150", "135"]
                    datos = linea_limpia.split(',')
                    
                    # Verificamos que la línea tenga exactamente los 5 elementos esperados
                    if len(datos) == 5:
                        escritor_csv.writerow(datos)
                        # Opcional: imprimir en la consola de Python para ver qué está pasando
                        print(f"Registrado: {datos}")

except serial.SerialException as e:
    print(f"Error de conexión serial: {e}")
except KeyboardInterrupt:
    # Captura el Ctrl+C en la terminal para cerrar el archivo limpiamente sin corromperlo
    print("\nCaptura detenida por el usuario. Archivo guardado con éxito.")
finally:
    # Aseguramos el cierre del puerto pase lo que pase
    if 'arduino' in locals() and arduino.is_open:
        arduino.close()
        print("Puerto serial cerrado.")