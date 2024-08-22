#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;
using namespace cv;

class ArucoMarkerDetector {
public:
    ArucoMarkerDetector() {
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            cerr << "Error al iniciar Winsock: " << iResult << endl;
            throw runtime_error("Error al iniciar Winsock");
        }

        int cameraIndex = selectCamera();
        if (!initCamera(cameraIndex)) {
            cerr << "Error al abrir la cámara!" << endl;
            WSACleanup();
            throw runtime_error("No se pudo abrir la cámara");
        }

        detector = aruco::ArucoDetector(aruco::getPredefinedDictionary(aruco::DICT_6X6_250));
    }

    ~ArucoMarkerDetector() {
        WSACleanup();
    }

    void detectAndSend() {
        SOCKET sock = INVALID_SOCKET;
        struct sockaddr_in serv_addr;
        const char* server_ip = "127.0.0.1";
        int port = 8080;

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            cerr << "Error al crear el socket: " << WSAGetLastError() << endl;
            WSACleanup();
            return;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            cerr << "Dirección IP inválida o no soportada" << endl;
            closesocket(sock);
            WSACleanup();
            return;
        }

        if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            cerr << "Conexión fallida: " << WSAGetLastError() << endl;
            closesocket(sock);
            WSACleanup();
            return;
        }

        while (true) {
            Mat frame;
            camera >> frame;
            if (frame.empty()) {
                cerr << "Error al capturar el frame!" << endl;
                break;
            }

            vector<int> markerIds;
            vector<vector<Point2f>> markerCorners;

            detector.detectMarkers(frame, markerCorners, markerIds);

            if (markerIds.size() >= 4) {
                cout << "Se detectaron " << markerIds.size() << " marcadores." << endl;

                aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

                vector<Point2f> redPoints;
                for (int i = 0; i < 4; i++) {
                    redPoints.push_back(markerCorners[i][0]);
                }

                std::sort(redPoints.begin(), redPoints.end(), [](const Point2f& a, const Point2f& b) {
                    return a.y < b.y || (a.y == b.y && a.x < b.x);
                    });

                Point2f topLeft = redPoints[0];
                Point2f topRight = redPoints[1];
                Point2f bottomLeft = redPoints[2];
                Point2f bottomRight = redPoints[3];

                if (topRight.x < topLeft.x) {
                    swap(topLeft, topRight);
                }
                if (bottomRight.x < bottomLeft.x) {
                    swap(bottomLeft, bottomRight);
                }

                vector<Point2f> orderedPoints = { topLeft, topRight, bottomRight, bottomLeft };
                for (int i = 0; i < 4; i++) {
                    circle(frame, orderedPoints[i], 10, Scalar(0, 0, 255), -1);
                    line(frame, orderedPoints[i], orderedPoints[(i + 1) % 4], Scalar(0, 255, 0), 3);
                }

                Mat mask = Mat::zeros(frame.size(), CV_8UC1);
                vector<Point> pts;
                for (const auto& pt : orderedPoints) {
                    pts.push_back(Point(pt.x, pt.y));
                }
                fillConvexPoly(mask, pts, Scalar(255));

                Mat safeArea;
                frame.copyTo(safeArea, mask);

                vector<uchar> buf;
                imencode(".jpg", safeArea, buf);
                int size = buf.size();

                send(sock, (char*)&size, sizeof(int), 0);
                send(sock, (char*)buf.data(), size, 0);

                // Esperar la respuesta de Python
                char response[256];
                int response_size = recv(sock, response, 256, 0);
                if (response_size > 0) {
                    response[response_size] = '\0';
                    cout << "Respuesta de Python: " << response << endl;

                    if (strcmp(response, "no_person_detected") != 0) {
                        int x_min, y_min, x_max, y_max;
                        sscanf_s(response, "%d,%d,%d,%d", &x_min, &y_min, &x_max, &y_max);

                        // Dibujar el cuadro en la imagen original
                        rectangle(frame, Point(x_min, y_min), Point(x_max, y_max), Scalar(255, 0, 0), 2);
                    }
                }
            }
            else {
                cout << "No se detectaron suficientes marcadores en este frame." << endl;
            }

            // Mostrar el video en tiempo real
            imshow("Video en tiempo real - Detección de Aruco", frame);

            // Salir si se presiona la tecla 'q'
            if (waitKey(1) == 'q') {
                break;
            }
        }

        closesocket(sock);
    }

private:
    VideoCapture camera;
    aruco::ArucoDetector detector;

    int selectCamera() {
        vector<VideoCapture> cameras;
        int selectedCamera = -1;

        for (int i = 0; i < 10; i++) {
            VideoCapture tempCamera(i);
            if (tempCamera.isOpened()) {
                cameras.push_back(move(tempCamera));
                cout << "Cámara " << i << " detectada." << endl;
            }
        }

        if (cameras.empty()) {
            cerr << "No se encontraron cámaras conectadas!" << endl;
            throw runtime_error("No se encontraron cámaras conectadas");
        }

        cout << "Seleccione una cámara de la lista (0 - " << cameras.size() - 1 << "): ";
        cin >> selectedCamera;

        if (selectedCamera < 0 || selectedCamera >= cameras.size()) {
            cerr << "Selección de cámara no válida!" << endl;
            throw runtime_error("Selección de cámara no válida");
        }

        return selectedCamera;
    }

    bool initCamera(int cameraIndex) {
        camera = VideoCapture(cameraIndex);
        if (!camera.isOpened()) {
            cerr << "No se pudo abrir la cámara con el índice " << cameraIndex << endl;
            return false;
        }
        cout << "Cámara con el índice " << cameraIndex << " abierta correctamente." << endl;
        return true;
    }
};

int main() {
    try {
        ArucoMarkerDetector detector;
        detector.detectAndSend();
    }
    catch (const exception& e) {
        cerr << "Excepción: " << e.what() << endl;
        return -1;
    }

    return 0;
}