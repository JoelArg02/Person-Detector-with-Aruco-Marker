import socket
import cv2
import numpy as np
import tensorflow as tf
import tensorflow_hub as hub

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('127.0.0.1', 8080))
server_socket.listen(1)
print("Esperando conexión del cliente...")

client_socket, addr = server_socket.accept()
print(f"Conectado a {addr}")

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

    if len(valid_boxes) > 0:
        h, w, _ = image.shape

        x_min_combined = w
        y_min_combined = h
        x_max_combined = 0
        y_max_combined = 0

        for box in valid_boxes:
            y_min, x_min, y_max, x_max = box
            x_min, x_max, y_min, y_max = int(x_min * w), int(x_max * w), int(y_min * h), int(y_max * h)
            x_min_combined = min(x_min_combined, x_min)
            y_min_combined = min(y_min_combined, y_min)
            x_max_combined = max(x_max_combined, x_max)
            y_max_combined = max(y_max_combined, y_max)


        return [(x_min_combined, y_min_combined, x_max_combined, y_max_combined)]
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

        coordinates_str = [f"{x_min},{y_min},{x_max},{y_max}" for x_min, y_min, x_max, y_max in result]
        response = ";".join(coordinates_str)
    else:
        response = "no_person_detected"

    client_socket.sendall(response.encode())

# Cierre de la conexión
client_socket.close()
server_socket.close()
