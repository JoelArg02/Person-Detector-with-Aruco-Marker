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
            cerr << "Error al abrir la c�mara!" << endl;
            throw runtime_error("No se pudo abrir la c�mara");
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

        // Enumerar c�maras disponibles
        for (int i = 0; i < 10; i++) {  // Supongamos que hay como m�ximo 10 c�maras conectadas
            VideoCapture tempCamera(i);
            if (tempCamera.isOpened()) {
                cameras.push_back(move(tempCamera));
                cout << "C�mara " << i << " detectada." << endl;
            }
        }

        if (cameras.empty()) {
            cerr << "No se encontraron c�maras conectadas!" << endl;
            throw runtime_error("No se encontraron c�maras conectadas");
        }

        // Solicitar al usuario que seleccione una c�mara
        cout << "Seleccione una c�mara de la lista (0 - " << cameras.size() - 1 << "): ";
        cin >> selectedCamera;

        if (selectedCamera < 0 || selectedCamera >= cameras.size()) {
            cerr << "Selecci�n de c�mara no v�lida!" << endl;
            throw runtime_error("Selecci�n de c�mara no v�lida");
        }

        return selectedCamera;
    }

    bool initCamera(int cameraIndex) {
        camera = VideoCapture(cameraIndex);
        if (!camera.isOpened()) {
            cerr << "No se pudo abrir la c�mara con el �ndice " << cameraIndex << endl;
            return false;
        }
        cout << "C�mara con el �ndice " << cameraIndex << " abierta correctamente." << endl;
        return true;
    }
};

int main() {
    try {
        // Crear una instancia del detector de marcadores ArUco
        ArucoMarkerDetector detector;
        // Iniciar el proceso de detecci�n y visualizaci�n
        detector.detectAndDisplay();
    }
    catch (const exception& e) {
        cerr << "Excepci�n: " << e.what() << endl;
        return -1;
    }

    return 0;
}
