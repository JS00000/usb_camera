#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>

#include <ros/ros.h>
#include <ros/package.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>

int main(int argc, char** argv)
{
    bool enable_raw = false;
    bool enable_rect = false;
    bool enable_info = false;
    std::string deviceName = "/dev/video0";
    int apiID = cv::CAP_V4L2;
    std::vector<std::string> programArgs{};
    ::ros::removeROSArgs(argc, argv, programArgs);
    if (programArgs.size() >= 2) {
        deviceName = programArgs[1];
        std::cout << "deviceName: " << deviceName << std::endl;
    }
    for (int i = 2; i < programArgs.size(); i++) {
        if (programArgs[i] == "raw") {
            enable_raw = true;
            std::cout << "Publish raw" << std::endl;
        }
        if (programArgs[i] == "rect") {
            enable_rect = true;
            std::cout << "Publish rect" << std::endl;
        } 
        if (programArgs[i] == "info") {
            enable_info = true;
            std::cout << "Publish info" << std::endl;
        } 
    }

    if (!(enable_raw || enable_rect || enable_info)) {
        std::cout << "No work to do, exit." << std::endl;
        return 0;
    }

    // Initialize ros node
    ros::init(argc, argv, "camera_calib");
    ros::NodeHandle nh;

    image_transport::ImageTransport it_raw_l(nh);
    image_transport::ImageTransport it_raw_r(nh);
    image_transport::ImageTransport it_rect_l(nh);
    image_transport::ImageTransport it_rect_r(nh);
    image_transport::Publisher image_pub_raw_l;
    image_transport::Publisher image_pub_raw_r;
    image_transport::Publisher image_pub_rect_l;
    image_transport::Publisher image_pub_rect_r;
    ros::Publisher camera_info_pub_l;
    ros::Publisher camera_info_pub_r;
    sensor_msgs::CameraInfo camera_info_l, camera_info_r;

    if (enable_raw) {
        image_pub_raw_l = it_raw_l.advertise("m2_camera/left/image_raw", 1);
        image_pub_raw_r = it_raw_r.advertise("m2_camera/right/image_raw", 1);
    }
    if (enable_rect) {
        image_pub_rect_l = it_rect_l.advertise("m2_camera/left/image_rect", 1);
        image_pub_rect_r = it_rect_r.advertise("m2_camera/right/image_rect", 1);
    }
    if (enable_info) {
        camera_info_pub_l = nh.advertise<sensor_msgs::CameraInfo>("m2_camera/left/camera_info", 1);
        camera_info_pub_r = nh.advertise<sensor_msgs::CameraInfo>("m2_camera/right/camera_info", 1);
    }

    cv::Mat cameraMatrix1, cameraMatrix2;
    cv::Mat distCoeffs1, distCoeffs2;
    cv::Mat R1, R2, P1, P2;
    cv::Size imageSize;
    cv::Mat map1[2], map2[2];

    if (enable_info || enable_rect) {
        std::string camera_calib_para = ros::package::getPath("usb_camera") + "/calib/m2_calibration.yml";
        std::cout << "Read camera calib parameter from " << camera_calib_para << std::endl;
        // read parameter
        cv::FileStorage fs(camera_calib_para, cv::FileStorage::READ);
        fs["imageWidth"] >> imageSize.width;
        fs["imageHeight"] >> imageSize.height;
        fs["cameraMatrix1"] >> cameraMatrix1;
        fs["cameraMatrix2"] >> cameraMatrix2;
        fs["distCoeffs1"] >> distCoeffs1;
        fs["distCoeffs2"] >> distCoeffs2;
        fs["R1"] >> R1;
        fs["R2"] >> R2;
        fs["P1"] >> P1;
        fs["P2"] >> P2;
        fs.release();

        camera_info_l.width = imageSize.width;
        camera_info_l.height = imageSize.height;
        camera_info_l.distortion_model = "plumb_bob";
        camera_info_l.K = { cameraMatrix1.at<double>(0,0), cameraMatrix1.at<double>(0,1), cameraMatrix1.at<double>(0,2), 
                            cameraMatrix1.at<double>(1,0), cameraMatrix1.at<double>(1,1), cameraMatrix1.at<double>(1,2), 
                            cameraMatrix1.at<double>(2,0), cameraMatrix1.at<double>(2,1), cameraMatrix1.at<double>(2,2)};
        camera_info_l.D = { distCoeffs1.at<double>(0,0), 
                            distCoeffs1.at<double>(1,0), 
                            distCoeffs1.at<double>(2,0), 
                            distCoeffs1.at<double>(3,0), 
                            distCoeffs1.at<double>(4,0)};
        camera_info_l.R = { R1.at<double>(0,0), R1.at<double>(0,1), R1.at<double>(0,2), 
                            R1.at<double>(1,0), R1.at<double>(1,1), R1.at<double>(1,2), 
                            R1.at<double>(2,0), R1.at<double>(2,1), R1.at<double>(2,2)};
        camera_info_l.P = { P1.at<double>(0,0), P1.at<double>(0,1), P1.at<double>(0,2), P1.at<double>(0,3), 
                            P1.at<double>(1,0), P1.at<double>(1,1), P1.at<double>(1,2), P1.at<double>(1,3), 
                            P1.at<double>(2,0), P1.at<double>(2,1), P1.at<double>(2,2), P1.at<double>(2,3)};

        camera_info_r.width = imageSize.width;
        camera_info_r.height = imageSize.height;
        camera_info_r.distortion_model = "plumb_bob";
        camera_info_r.K = { cameraMatrix2.at<double>(0,0), cameraMatrix2.at<double>(0,1), cameraMatrix2.at<double>(0,2), 
                            cameraMatrix2.at<double>(1,0), cameraMatrix2.at<double>(1,1), cameraMatrix2.at<double>(1,2), 
                            cameraMatrix2.at<double>(2,0), cameraMatrix2.at<double>(2,1), cameraMatrix2.at<double>(2,2)};
        camera_info_r.D = { distCoeffs2.at<double>(0,0), 
                            distCoeffs2.at<double>(1,0), 
                            distCoeffs2.at<double>(2,0), 
                            distCoeffs2.at<double>(3,0), 
                            distCoeffs2.at<double>(4,0)};
        camera_info_r.R = { R2.at<double>(0,0), R2.at<double>(0,1), R2.at<double>(0,2), 
                            R2.at<double>(1,0), R2.at<double>(1,1), R2.at<double>(1,2), 
                            R2.at<double>(2,0), R2.at<double>(2,1), R2.at<double>(2,2)};
        camera_info_r.P = { P2.at<double>(0,0), P2.at<double>(0,1), P2.at<double>(0,2), P2.at<double>(0,3), 
                            P2.at<double>(1,0), P2.at<double>(1,1), P2.at<double>(1,2), P2.at<double>(1,3), 
                            P2.at<double>(2,0), P2.at<double>(2,1), P2.at<double>(2,2), P2.at<double>(2,3)};

        // std::cout << camera_info_l << std::endl;
        // std::cout << camera_info_r << std::endl;

        cv::initUndistortRectifyMap(cameraMatrix1, distCoeffs1, R1, P1, imageSize, CV_16SC2, map1[0], map2[0]);
        cv::initUndistortRectifyMap(cameraMatrix2, distCoeffs2, R2, P2, imageSize, CV_16SC2, map1[1], map2[1]);
    }

    std::cout << "Opening camera " << deviceName << std::endl;
    cv::VideoCapture capture(deviceName, apiID); // open the camera
    if (!capture.isOpened())
    {
        std::cerr << "ERROR: Can't initialize camera capture" << std::endl;
        return 1;
    }

    int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    capture.set(cv::CAP_PROP_FOURCC,codec);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FPS, 30.0);

    std::cout << "Frame width: " << capture.get(cv::CAP_PROP_FRAME_WIDTH) << std::endl;
    std::cout << "     height: " << capture.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
    std::cout << "Capturing FPS: " << capture.get(cv::CAP_PROP_FPS) << std::endl;

    cv::Mat frame;
    size_t nFrames = 0;
    std_msgs::Header header_l, header_r;
    header_l.frame_id = "camera_left";
    header_l.stamp = ros::Time::now();
    header_r.frame_id = "camera_right";
    header_r.stamp = ros::Time::now();

    while (::ros::ok() && ::ros::master::check()) {
        capture >> frame; // read the next frame from camera
        if (frame.empty()) {
            std::cerr << "ERROR: Can't grab camera frame." << std::endl;
            break;
        }
        nFrames++;
        cv::Mat frame_l = frame(cv::Rect(0, 0, frame.size().width/2, frame.size().height));
        cv::Mat frame_r = frame(cv::Rect(frame.size().width/2, 0, frame.size().width/2, frame.size().height));

        if (enable_raw) {
            sensor_msgs::ImagePtr msg_raw_l = cv_bridge::CvImage(header_l, "bgr8", frame_l).toImageMsg();
            sensor_msgs::ImagePtr msg_raw_r = cv_bridge::CvImage(header_r, "bgr8", frame_r).toImageMsg();
            image_pub_raw_l.publish(msg_raw_l);
            image_pub_raw_r.publish(msg_raw_r);
        }
        if (enable_rect) {
            cv::Mat rect_l(imageSize, CV_8UC3);
            cv::Mat rect_r(imageSize, CV_8UC3);
            cv::remap(frame_l, rect_l, map1[0], map2[0], cv::INTER_LINEAR);
            cv::remap(frame_r, rect_r, map1[1], map2[1], cv::INTER_LINEAR);
            sensor_msgs::ImagePtr msg_rect_l = cv_bridge::CvImage(header_l, "bgr8", rect_l).toImageMsg();
            sensor_msgs::ImagePtr msg_rect_r = cv_bridge::CvImage(header_r, "bgr8", rect_r).toImageMsg();
            image_pub_rect_l.publish(msg_rect_l);
            image_pub_rect_r.publish(msg_rect_r);
        }
        if (enable_info) {
            camera_info_pub_l.publish(camera_info_l);
            camera_info_pub_r.publish(camera_info_r);
        }
        // cv::imshow("left", rect_l);
        // cv::imshow("right", rect_r);
        // int key = cv::waitKey(1);
        // if (key == 27/*ESC*/) {
        //     break;
        // }
    }
    std::cout << "Number of captured frames: " << nFrames << std::endl;
    return nFrames > 0 ? 0 : 1;
}
