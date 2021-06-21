#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>

#include <thread>
#include <mutex>
#include <atomic>

#include <sys/stat.h>

using namespace cv;
using namespace std;


// class VideoCaptureMT {
// public:
//     VideoCaptureMT(int index, int height=480, int width=640);
//     VideoCaptureMT(std::string filePath, int height=480, int width=640);
//     ~VideoCaptureMT();
    
//     bool isOpened() {
//         return m_IsOpen;
//     }
//     void release() {
//         m_IsOpen = false;
//     }
//     int read(cv::Mat& frame);

// private:
//     void captureInit(int index, std::string filePath, int height, int width);
//     void captureFrame();

//     bool m_pFrame_next;
//     cv::VideoCapture* m_pCapture;
//     cv::Mat* m_pFrame;
//     std::mutex* m_pMutex;
//     std::thread* m_pThread;
//     std::atomic_bool m_IsOpen;
// };

// VideoCaptureMT::VideoCaptureMT(int index, int height, int width)
// {
//     captureInit(index, std::string(), height, width);
// }

// VideoCaptureMT::VideoCaptureMT(std::string filePath, int height, int width)
// {
//     captureInit(0, filePath, height, width);
// }

// VideoCaptureMT::~VideoCaptureMT()
// {
//     m_IsOpen = false;
//     m_pThread->join();
//     if (m_pCapture->isOpened()) {
//         m_pCapture->release();
//     }

//     delete m_pThread;
//     delete m_pMutex;
//     delete m_pCapture;
//     delete m_pFrame;
// }

// void VideoCaptureMT::captureInit(int index, std::string filePath, int height, int width)
// {
//     if (!filePath.empty()) {
//         m_pCapture = new cv::VideoCapture(filePath);
//     }
//     else {
//         m_pCapture = new cv::VideoCapture(index);
//     }
//     m_pCapture->set(cv::CAP_PROP_FRAME_WIDTH, width);
//     m_pCapture->set(cv::CAP_PROP_FRAME_HEIGHT, height);
//     m_pCapture->set(cv::CAP_PROP_FPS, 30);

//     cout << "camera " << index << endl;
//     cout << "Frame width: " << m_pCapture->get(CAP_PROP_FRAME_WIDTH) << endl;
//     cout << "     height: " << m_pCapture->get(CAP_PROP_FRAME_HEIGHT) << endl;
//     cout << "Capturing FPS: " << m_pCapture->get(CAP_PROP_FPS) << endl;

//     m_IsOpen = true;
//     m_pFrame_next = false;
//     m_pFrame = new cv::Mat(height, width, CV_8UC3);
//     m_pMutex = new std::mutex();
//     m_pThread = new std::thread(&VideoCaptureMT::captureFrame, this);
// }

// void VideoCaptureMT::captureFrame()
// {
//     cv::Mat frameBuff;
//     while (m_IsOpen) {
//         (*m_pCapture) >> frameBuff;
//         if (m_pMutex->try_lock()) {
//             frameBuff.copyTo(*m_pFrame);
//             m_pFrame_next = true;
//             m_pMutex->unlock();
//         }
//         cv::waitKey(5);
//     }
// }

// int VideoCaptureMT::read(cv::Mat& frame)
// {
//     if (m_pFrame->empty()) {
//         m_IsOpen = false;
//         return -1;
//     } else if (m_pFrame_next) {
//         m_pMutex->lock();
//         m_pFrame->copyTo(frame);
//         m_pFrame_next = false;
//         m_pMutex->unlock();
//         return 1;
//     }
//     return 0;
// }

int main(int argc, char** argv)
{
    int deviceID1 = 0;
    int deviceID2 = 1;
    int apiID = cv::CAP_ANY;
    string save_path = "/dev/shm/";
    if (argc == 3) {
        deviceID1 = atoi(argv[1]);
        deviceID2 = atoi(argv[2]);
    } else if (argc == 4) {
        save_path = argv[3];
    } else {
        cerr << "ERROR: parameter must be (deviceID1 deviceID2 [save_path])" << endl;
        return 1;
    }
    cout << "Opening camera..." << endl;
    VideoCapture capture1(deviceID1); // open the first camera
    VideoCapture capture2(deviceID2); // open the second camera
    if (!capture1.isOpened())
    {
        cerr << "ERROR: Can't initialize camera capture1" << endl;
        return 1;
    }
    if (!capture2.isOpened())
    {
        cerr << "ERROR: Can't initialize camera capture2" << endl;
        return 1;
    }


    VideoWriter writer;
    int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
    capture1.set(CAP_PROP_FOURCC,codec);
    capture1.set(CAP_PROP_FRAME_WIDTH, 640);
    capture1.set(CAP_PROP_FRAME_HEIGHT, 480);
    capture1.set(CAP_PROP_FPS, 30.0);
    capture2.set(CAP_PROP_FOURCC,codec);
    capture2.set(CAP_PROP_FRAME_WIDTH, 640);
    capture2.set(CAP_PROP_FRAME_HEIGHT, 480);
    capture2.set(CAP_PROP_FPS, 30.0);

    cout << "camera " << deviceID1 << endl;
    cout << "Frame width: " << capture1.get(CAP_PROP_FRAME_WIDTH) << endl;
    cout << "     height: " << capture1.get(CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "Capturing FPS: " << capture1.get(CAP_PROP_FPS) << endl;

    cout << "camera " << deviceID2 << endl;
    cout << "Frame width: " << capture2.get(CAP_PROP_FRAME_WIDTH) << endl;
    cout << "     height: " << capture2.get(CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "Capturing FPS: " << capture2.get(CAP_PROP_FPS) << endl;

    cout << endl << "Press 'ESC' to quit, 'space' to toggle frame processing" << endl;
    cout << endl << "Start grabbing..." << endl;

    std::string mkdir_pack_left = "mkdir -p " + save_path + "left/";
    std::string mkdir_pack_right = "mkdir -p " + save_path + "right/";
    system(mkdir_pack_left.c_str());
    system(mkdir_pack_right.c_str());

    Mat frame1, frame2;
    size_t nFrames = 0;
    int64 t0 = cv::getTickCount();
    int save_id = 0;
    while (capture1.isOpened() && capture2.isOpened())
    {
        capture1 >> frame1;
        capture2 >> frame2;
        if (frame1.empty() || frame2.empty())
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

        imshow("left", frame1);
        imshow("right", frame2);
        int key = waitKey(1);
        if (key == 27/*ESC*/) 
        {
            break;
        } else if (key == 's') {
            string save_path_left = save_path + "left/" + to_string(save_id) + ".jpg";
            string save_path_right = save_path + "right/" + to_string(save_id) + ".jpg";
            imwrite(save_path_left, frame1);
            imwrite(save_path_right, frame2);
            cout << "Save " << save_path_left << endl;
            cout << "Save " << save_path_right << endl;
            save_id++;
        }

    }
    std::cout << "Number of captured frames: " << nFrames << endl;
    return nFrames > 0 ? 0 : 1;
}
