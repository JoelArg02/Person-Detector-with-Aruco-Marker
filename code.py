import socket
import cv2
import numpy as np
import tensorflow as tf
import tensorflow_hub as hub

# Configuración del socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('127.0.0.1', 8080))
server_socket.listen(1)
print("Esperando conexión del cliente...")

client_socket, addr = server_socket.accept()
print(f"Conectado a {addr}")

# Cargar el modelo preentrenado desde TensorFlow Hub
model = hub.load("https://tfhub.dev/tensorflow/ssd_mobilenet_v2/2")

def detect_people(image):
    input_tensor = cv2.resize(image, (300, 300))
    input_tensor = np.expand_dims(input_tensor, axis=0)

    tensor = tf.convert_to_tensor(input_tensor, dtype=tf.uint8)

    detections = model(tensor)

    detection_boxes = detections['detection_boxes'][0].numpy()
    detection_scores = detections['detection_scores'][0].numpy()
    detection_classes = detections['detection_classes'][0].numpy()

    person_boxes = detection_boxes[detection_classes == 1]
    person_scores = detection_scores[detection_classes == 1]

    valid_boxes = person_boxes[person_scores > 0.5]

    # Si se detecta una persona, devolver el primer cuadro
    if len(valid_boxes) > 0:
        box = valid_boxes[0]  # Solo manejamos el primer cuadro detectado
        h, w, _ = image.shape
        y_min, x_min, y_max, x_max = box
        x_min, x_max, y_min, y_max = int(x_min * w), int(x_max * w), int(y_min * h), int(y_max * h)
        return x_min, y_min, x_max, y_max
    else:
        return None

while True:
    img_size_data = client_socket.recv(4)
    if not img_size_data:
        break
    img_size = int.from_bytes(img_size_data, byteorder='little')

    img_data = b''
    while len(img_data) < img_size:
        packet = client_socket.recv(4096)
        if not packet:
            break
        img_data += packet

    nparr = np.frombuffer(img_data, np.uint8)
    frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    result = detect_people(frame)
    if result:
        x_min, y_min, x_max, y_max = result
        # Enviar las coordenadas de la persona detectada
        response = f"{x_min},{y_min},{x_max},{y_max}"
    else:
        # No se detectó ninguna persona
        response = "no_person_detected"

    client_socket.sendall(response.encode())

client_socket.close()
server_socket.close()
