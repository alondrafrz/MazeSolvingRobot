import socket
import csv

# 1. Configuración de la red
# Dejamos la IP en "0.0.0.0" para que Python escuche en TODAS las tarjetas de red de tu PC
IP_ESCUCHA = "0.0.0.0" 
PUERTO = 1234
NOMBRE_ARCHIVO = r"C:\Users\frzal\Desktop\datos_seguidor_wifi.csv"

# 2. Crear el socket UDP
# AF_INET significa que usa IPv4 / SOCK_DGRAM significa que usa el protocolo UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((IP_ESCUCHA, PUERTO))

print(f"Sintonizando el puerto {PUERTO}... Esperando datos del robot por Wi-Fi.")

try:
    # 3. Crear y abrir el archivo CSV
    with open(NOMBRE_ARCHIVO, mode='w', newline='', encoding='utf-8') as archivo_csv:
        escritor_csv = csv.writer(archivo_csv)
        
        # Encabezados de las columnas
        escritor_csv.writerow(["UFL", "UFC", "UFR", "Vel_Izq", "Vel_Der"])
        print(f"Guardando datos en '{NOMBRE_ARCHIVO}'... Presiona Ctrl+C para detener.")

        while True:
            # 4. Quedarse esperando un paquete (tamaño máximo del buffer: 1024 bytes)
            data, addr = sock.recvfrom(1024)
            
            # Decodificar los bytes recibidos a texto plano y limpiar espacios/saltos de línea
            linea_limpia = data.decode('utf-8').strip()
            
            if linea_limpia:
                # Separar los datos por la coma
                datos = linea_limpia.split(',')
                
                # Validar que llegaron los 5 datos que configuramos en el ESP32
                if len(datos) == 5:
                    escritor_csv.writerow(datos)
                    print(f"Recibido desde {addr[0]}: {datos}")

except KeyboardInterrupt:
    print("\nCaptura inalámbrica detenida por el usuario. Archivo guardado con éxito.")
finally:
    # Cerrar el socket de red al terminar
    sock.close()
    print("Socket de red cerrado.")