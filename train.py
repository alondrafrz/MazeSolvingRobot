"""
Universidad Nacional Autónoma de México
Facultad de Ciencias
Licenciatura en Ciencias de la Computación
Servicio Social

Escrito por: Alondra Navila Felipe Ramírez
Última modificación: 17 de Junio de 2026

Descripción:
Este programa toma un conjunto de datos previamente generados 
de un robot seguidor de línea, y entrena un modelo de aprendizaje
automatizado para 

"""

# Importamos las bivliotecas
import tensorflow as tf
import pandas as pd
import matplotlib.pyplot as plt

# Cargamso los datos
df = pd.read_csv('datos_recorridos.csv')
print("Datos cargados correctamente:")
print(df.head()) #Sólo las primeras filas


# Preprocesamiento. Noramlizamos las entradas con Escalamiento lineal
##inputs = inputs.astype('float32') / 4095
#outputs = outputs / 255.0

input_columns =['UFL', 'UFC', 'UFR']

for col in input_columns:
    df[col] = pd.to_numeric(df[col], errors='coerce')

val_min = df[input_columns].min()
val_max = df[input_columns].max()

# Aplicamos eEscalamiento lineal
df_pre = df.copy()
df_pre[input_columns] = (df[input_columns] - val_min) / (val_max - val_min)

# Extraemos los datos ya normalizados
inputs = df_pre[input_columns].values 
outputs = df[['Vel_Izq', 'Vel_Der']].values

print("Muestra las entradas normalizadas")
print(inputs[:5])

# Creamos el modelo
model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(3,)),
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(2)
])

# Compilamos el modelo con optimizador Adam y métricas de regresión MSE y MAE
model.compile(optimizer='adam', loss='mse', metrics=['mae'])

# Entrenamos
print("Inicializando entrenamiento...")
history = model.fit(inputs, outputs, epochs=100, batch_size=8)

# Graficamos
print("\nGuardando gráficas de entrenamiento...")

# Gráfica MAE (Error Absoluto Medio) vs Épocas
plt.figure(figsize=(10, 5))
plt.plot(history.history['mae'], label ='Error Promedio (MAE)')
plt.title('Evolución del Error de Regresión vs Épocas')
plt.ylabel('Error (Escala 0-1)')
plt.xlabel('Época')
plt.legend(loc="upper right")
plt.grid(True)
plt.savefig('grafica_mae.png')
plt.close()

# Gráfica de Pérdida (Loss/MSE) vs Épocas
plt.figure(figsize=(10, 5))
plt.plot(history.history['loss'], label='Pérdida (MSE)')
plt.title('Pérdida de Entrenamiento vs Épocas')
plt.ylabel('Pérdida')
plt.xlabel('Época')
plt.legend(loc="upper right")
plt.grid(True)
plt.savefig('grafica_loss.png')
plt.close()

print("- Gráficas guardadas como 'grafica_mae.png' y 'grafica_loss.png'")

# Convertimos a formato TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.converter()

# Generamso el archivo .h para Arduino
def hex_to_c_array(hex_data,  var_name):
    c_str = ''
    c_str = 'const unsigned char' + var_name + '[] = {\n'
    for i, val in enumerate(hex_data):
        c_str += '0x{:02x}' '.format(val)'
        if (i + 1) % 12 == 0:
            c_str += '\n'
            c_str += '\n]; \n'
            c_str += 'const unsigned int' + var_name + '_len =' + str(len(hex_data)) + ';\n'
            return c_str
        
print("\n Generando archivo de modelo para ESP32...")
c_model = hex_to_c_array(tflite_model, "regression_model")

# Nombre del archivo de destino
filename = 'regression_model.h'

# Guardamos el contenido en un archivo de texto
with open(filename, 'w') as f:
    f.write(c_model)

print(f"- ¡Modelo guardado exitosamente como '{filename}'!")
print("Ya puedes usar este archivo directamente en tu sketch de Arduino.")