#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>

#include <sys/stat.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    int deviceID = 0;
    int apiID = cv::CAP_V4L2;
    string save_path = "/dev/shm/";
    if (argc >= 2) {
        deviceID = atoi(argv[1]);
    }
    if (argc >= 3) {
        save_path = argv[2];
    }
    cout << "Opening camera " << deviceID << endl;
    VideoCapture capture(deviceID, apiID); // open the first camera
    if (!capture.isOpened())
    {
        cerr << "ERROR: Can't initialize camera capture" << endl;
        return 1;
    }

    int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
    // int codec = VideoWriter::fourcc('Y', 'U', 'Y', 'V');
    // int codec = VideoWriter::fourcc('R', 'G', 'B', '3');
    // int codec = VideoWriter::fourcc('B', 'G', 'R', '3');
    // int codec = VideoWriter::fourcc('Y', 'U', '1', '2');
    // int codec = VideoWriter::fourcc('Y', 'V', '1', '2');
    capture.set(CAP_PROP_FOURCC,codec);
    // capture.set(CAP_PROP_FRAME_WIDTH, 2560);
    // capture.set(CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CAP_PROP_FRAME_HEIGHT, 480);
    // capture.set(CAP_PROP_FRAME_WIDTH, 640);
    // capture.set(CAP_PROP_FRAME_HEIGHT, 240);
    // capture.set(CAP_PROP_FORMAT, -1);
    // capture.set(CAP_PROP_MODE, 3);
    // capture.set(CAP_PROP_FRAME_WIDTH, 1280);
    // capture.set(CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(CAP_PROP_FPS, 30.0);

    cout << "Frame width: " << capture.get(CAP_PROP_FRAME_WIDTH) << endl;
    cout << "     height: " << capture.get(CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "Capturing FPS: " << capture.get(CAP_PROP_FPS) << endl;

    cout << endl << "Press 'ESC' to quit, 's' to save image" << endl;
    cout << endl << "Start grabbing..." << endl;

    std::string mkdir_pack_left = "mkdir -p " + save_path + "left/";
    std::string mkdir_pack_right = "mkdir -p " + save_path + "right/";
    system(mkdir_pack_left.c_str());
    system(mkdir_pack_right.c_str());

    Mat frame;
    size_t nFrames = 0;
    int64 t0 = cv::getTickCount();
    int save_id = 0;
    for (;;)
    {
        capture >> frame; // read the next frame from camera
        if (frame.empty())
        {
            cerr << "ERROR: Can't grab camera frame." << endl;
            break;
        }
        nFrames++;
        if (nFrames % 10 == 0)
        {
            const int N = 10;
            int64 t1 = cv::getTickCount();
            cout << "Frames captured: " << cv::format("%5lld", (long long int)nFrames)
                 << "    Average FPS: " << cv::format("%9.1f", (double)getTickFrequency() * N / (t1 - t0))
                 << "    Average time per frame: " << cv::format("%9.2f ms", (double)(t1 - t0) * 1000.0f / (N * getTickFrequency()))
                 << std::endl;
            t0 = t1;
        }
        // imshow("Frame", frame);
        Mat frame_left = frame(Rect(0, 0, frame.size().width/2, frame.size().height));
        Mat frame_right = frame(Rect(frame.size().width/2, 0, frame.size().width/2, frame.size().height));
        imshow("left", frame_left);
        imshow("right", frame_right);
        int key = waitKey(1);
        if (key == 27/*ESC*/) 
        {
            break;
        } else if (key == 's') {
            string save_path_left = save_path + "left/" + to_string(save_id) + ".jpg";
            string save_path_right = save_path + "right/" + to_string(save_id) + ".jpg";
            imwrite(save_path_left, frame_left);
            imwrite(save_path_right, frame_right);
            cout << "Save " << save_path_left << endl;
            cout << "Save " << save_path_right << endl;
            save_id++;
        }

    }
    std::cout << "Number of captured frames: " << nFrames << endl;
    return nFrames > 0 ? 0 : 1;
}
