#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

class ArucoMarkerDetector {
public:
    ArucoMarkerDetector() {
        int cameraIndex = selectCamera();
        if (!initCamera(cameraIndex)) {
            cerr << "Error al abrir la cámara!" << endl;
            throw runtime_error("No se pudo abrir la cámara");
        }

        // Crear el diccionario de marcadores ArUco
        detector = aruco::ArucoDetector(aruco::getPredefinedDictionary(aruco::DICT_6X6_250));
    }

    void detectAndDisplay() {
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

            if (!markerIds.empty()) {
                cout << "Marcador(es) detectado(s): " << markerIds.size() << endl;
                aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
            }
            else {
                cout << "No se detectaron marcadores en este frame." << endl;
            }

            // Mostrar el frame con los marcadores detectados
            imshow("Detected Markers", frame);

            // Salir del bucle si se presiona la tecla 'q'
            if (waitKey(1) == 'q') {
                break;
            }
        }
    }

private:
    VideoCapture camera;
    aruco::ArucoDetector detector;

    int selectCamera() {
        vector<VideoCapture> cameras;
        int selectedCamera = -1;

        // Enumerar cámaras disponibles
        for (int i = 0; i < 10; i++) {  // Supongamos que hay como máximo 10 cámaras conectadas
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

        // Solicitar al usuario que seleccione una cámara
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
        // Crear una instancia del detector de marcadores ArUco
        ArucoMarkerDetector detector;
        // Iniciar el proceso de detección y visualización
        detector.detectAndDisplay();
    }
    catch (const exception& e) {
        cerr << "Excepción: " << e.what() << endl;
        return -1;
    }

    return 0;
}
