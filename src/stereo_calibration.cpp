#include <iostream>
#include "opencv2/opencv.hpp"

#include <string>
#include <unistd.h>
#include <sys/stat.h>

std::string data_path_ = "/data/data/2M/demo/demo_240p";
std::string calib_file_ = "/data/data/2M/demo/demo_240p/m2_calibration.yml";

static void calcChessboardCorners(cv::Size boardSize, float squareSize, std::vector<cv::Point3f>& corners)
{
    corners.resize(0);

    for( int i = 0; i < boardSize.height; i++ )
        for( int j = 0; j < boardSize.width; j++ )
            corners.push_back(cv::Point3f(float(j*squareSize),
                                      float(i*squareSize), 0));
}

int StereoCalibrate() {
    int i, k;
    int flags = 0;
    cv::Size boardSize = cv::Size(9, 6);
    cv::Size imageSize;
    const float squareSize = 0.0245f;
    float aspectRatio = 1.0;

    std::string outputFilename = calib_file_;
    std::string inputFilename = data_path_;

    std::vector<cv::String> left_images, right_images;
    cv::glob(inputFilename + "/left/*.jpg", left_images);
    cv::glob(inputFilename + "/right/*.jpg", right_images);

    // init
    cv::Mat cameraMatrix[2], distCoeffs[2];
    for( k = 0; k < 2; k++ ) {
        cameraMatrix[k] = cv::Mat_<double>::eye(3,3);
        cameraMatrix[k].at<double>(0,0) = aspectRatio;
        cameraMatrix[k].at<double>(1,1) = 1;
        distCoeffs[k] = cv::Mat_<double>::zeros(5,1);
    }

    printf("Find chessboard corners...\n");

    std::vector<std::vector<cv::Point2f> > imgpt[2];
    for( i = 0; i < (int)(left_images.size()); i++ ) {
        printf("%s\n", left_images[i].c_str());
        printf("%s\n", right_images[i].c_str());
    
        bool found[2];
        cv::Mat view[2], viewGray[2];
        std::vector<cv::Point2f> ptvec[2];

        view[0] = cv::imread(left_images[i], cv::IMREAD_COLOR);
        view[1] = cv::imread(right_images[i], cv::IMREAD_COLOR);
        if(view[0].empty() || view[1].empty()) 
            continue;
        for (int k = 0; k < 2; k++) {
            imageSize = view[k].size();
            cv::cvtColor(view[k], viewGray[k], cv::COLOR_BGR2GRAY);
            found[k] = cv::findChessboardCorners( view[k], boardSize, ptvec[k], cv::CALIB_CB_ADAPTIVE_THRESH );

            cv::cornerSubPix(viewGray[k], ptvec[k], cv::Size(5, 5), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 30, 0.1));

            cv::drawChessboardCorners( view[k], boardSize, cv::Mat(ptvec[k]), found[k] );
            cv::imshow("view", view[k]);
            int c = cv::waitKey() & 255;
            if( c == 27 || c == 'q' || c == 'Q' )
                return -1;
        }
        if( found[0] && found[1] ) {
            imgpt[0].push_back(ptvec[0]);
            imgpt[1].push_back(ptvec[1]);
        }
    }

    printf("Running calibration...\n");

    // calibrate each camera individually
    std::vector<cv::Mat> rvecs, tvecs;
    std::vector<std::vector<cv::Point3f> > objpt(1);
    calcChessboardCorners(boardSize, squareSize, objpt[0]);

    int N = imgpt[0].size();
    int Np = imgpt[0][0].size() * N;
    if(N < 3) {
        printf("Error: not enough views for stereo.\n");
        return false;
    }
    objpt.resize(N, objpt[0]);

    for (int k = 0; k < 2; k++) {
        double err = cv::calibrateCamera(   objpt, imgpt[k], imageSize, 
                                            cameraMatrix[k], distCoeffs[k], rvecs, tvecs,
                                            flags | cv::CALIB_FIX_K3/*|CALIB_FIX_K4|CALIB_FIX_K5|CALIB_FIX_K6*/);
        bool ok = cv::checkRange(cameraMatrix[k]) && cv::checkRange(distCoeffs[k]);
        if(!ok) {
            printf("Error: camera %d was not calibrated\n", k);
            return false;
        }
        printf("Camera %d calibration reprojection error = %f\n", k, sqrt(err/Np));
    }

    printf("Running stereo calibration...\n");
    cv::Mat R, T, E, F;
    double err = cv::stereoCalibrate(   objpt, imgpt[0], imgpt[1], 
                                        cameraMatrix[0], distCoeffs[0], 
                                        cameraMatrix[1], distCoeffs[1], 
                                        imageSize, R, T, E, F, 
                                        flags | cv::CALIB_FIX_INTRINSIC);
    printf("Calibration reprojection error = %f\n", sqrt(err/(Np*2)));


    printf("Running rectification\n");
    cv::Mat R1, R2, P1, P2, Q;
    cv::stereoRectify(  cameraMatrix[0], distCoeffs[0], 
                        cameraMatrix[1], distCoeffs[1], 
                        imageSize, R, T, R1, R2, P1, P2, Q, flags);
    printf("Done rectification\n");

    cv::FileStorage fs;
    fs.open(outputFilename, cv::FileStorage::WRITE);
    fs << "imageWidth" << imageSize.width;
    fs << "imageHeight" << imageSize.height;
    fs << "cameraMatrix1" << cameraMatrix[0];
    fs << "cameraMatrix2" << cameraMatrix[1];
    fs << "distCoeffs1" << distCoeffs[0];
    fs << "distCoeffs2" << distCoeffs[1];
    fs << "R" << R;
    fs << "T" << T;
    fs << "E" << E;
    fs << "F" << F;
    fs << "R1" << R1;
    fs << "R2" << R2;
    fs << "P1" << P1;
    fs << "P2" << P2;
    fs << "Q" << Q;
    fs.release();

    cv::Mat map1[2], map2[2];
    cv::initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, map1[0], map2[0]);
    cv::initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, map1[1], map2[1]);

    cv::destroyWindow("view");
    cv::Mat canvas(imageSize.height, 2*imageSize.width, CV_8UC3), small_canvas;
    canvas = cv::Scalar::all(0);

    for( i = 0; i < (int)(left_images.size()); i++ )
    {
        printf("%s\n", left_images[i].c_str());
        printf("%s\n", right_images[i].c_str());
        canvas = cv::Scalar::all(0);
        cv::Mat viewl = cv::imread(left_images[i], cv::IMREAD_COLOR);
        cv::Mat viewr = cv::imread(right_images[i], cv::IMREAD_COLOR);
        if (viewl.empty() || viewr.empty())
            continue;

        cv::Mat canvas_l = canvas.colRange(0, imageSize.width);
        cv::Mat canvas_r = canvas.colRange(imageSize.width, 2*imageSize.width);
        cv::remap(viewl, canvas_l, map1[0], map2[0], cv::INTER_LINEAR);
        cv::remap(viewr, canvas_r, map1[1], map2[1], cv::INTER_LINEAR);

        cv::resize( canvas, small_canvas, cv::Size(), 0.7, 0.7, cv::INTER_AREA );
        for( k = 0; k < small_canvas.rows; k += 16 )
            cv::line(small_canvas, cv::Point(0, k), cv::Point(small_canvas.cols, k), cv::Scalar(0,255,0), 1);
        cv::imshow("rectified", small_canvas);
        char c = (char)cv::waitKey(0);
        if( c == 27 || c == 'q' || c == 'Q' )
            break;
    }
    cv::destroyWindow("rectified");
    return 0;
}

int main() {
    StereoCalibrate();
    return 0;
}
