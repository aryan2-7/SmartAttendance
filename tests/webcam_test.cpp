#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    std::cout << "Opening webcam..." << std::endl;

    // 0 = default webcam. Try 1 or 2 if you have multiple cameras.
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "ERROR: Could not open webcam." << std::endl;
        std::cerr << "Check that your webcam is connected and not in use by another app." << std::endl;
        return -1;
    }

    std::cout << "Webcam opened successfully!" << std::endl;
    std::cout << "Press Q or ESC to quit." << std::endl;

    cv::Mat frame;

    while (true) {
        // Grab a frame from the webcam
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "ERROR: Empty frame received. Stopping." << std::endl;
            break;
        }

        // Display the frame in a window
        cv::imshow("Webcam Test — press Q to quit", frame);

        // Wait 30ms for a key press
        int key = cv::waitKey(30);
        if (key == 'q' || key == 'Q' || key == 27) { // 27 = ESC
            break;
        }
    }

    // Release the webcam and close windows
    cap.release();
    cv::destroyAllWindows();

    std::cout << "Test complete. If you saw your webcam feed, OpenCV is working correctly!" << std::endl;
    return 0;
}
