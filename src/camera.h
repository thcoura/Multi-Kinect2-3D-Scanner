#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "globalsettings.h"

#include <libfreenect2/libfreenect2.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/io.h>
#include <pcl/filters/filter_indices.h>
#include <pcl/common/transforms.h>

#define camAttachTime 2000
#define ir_depth_width 512
#define ir_depth_height 424
#define ir_depth_bpp 4
#define color_width 1920
#define color_height 1080+2
#define color_bpp 4

class Camera
{
public:

    struct camera_frames
    {
        cv::Mat depthMat     = cv::Mat::zeros(ir_depth_height, ir_depth_width, CV_32FC1);
        cv::Mat irMat       = cv::Mat::zeros(ir_depth_height, ir_depth_width, CV_32FC1);
        cv::Mat colorMat    = cv::Mat::zeros(color_height, color_width, CV_8UC4);
        cv::Mat rgbdMat     = cv::Mat::zeros(ir_depth_height, ir_depth_width, CV_8UC4);

        cv::Mat rangedDepthMat  =  cv::Mat::zeros(ir_depth_height, ir_depth_width, CV_32FC1);
        cv::Mat rangedRGBDMat   =  cv::Mat::zeros(ir_depth_height, ir_depth_width, CV_8UC4);
        cv::Mat mask            =  cv::Mat::zeros(ir_depth_height, ir_depth_width, cv::THRESH_BINARY);
        cv::Mat histMat         =  cv::Mat::zeros(ir_depth_height, ir_depth_width, CV_8UC3);

        pcl::PointCloud<pcl::PointXYZRGB> cloud; //= pcl::PointCloud<pcl::PointXYZRGB>::Ptr(new pcl::PointCloud<pcl::PointXYZRGB>);
    };

    static Camera * create_camera(libfreenect2::Freenect2 * _freenect, int id);
    virtual ~Camera();
    int                     getId();
    std::string             getSerial();
    std::string             getCamType();

    virtual bool            lockFrames(int) = 0;
    virtual void            unlockFrames()  = 0;

    virtual void            start() = 0;
    virtual void            stop() = 0;

    virtual void            frames(std::atomic<bool> & keep_running)    = 0;
//    virtual void            cloudData(std::atomic<bool> & keep_running, std::atomic<bool> & compute_cloud_style) = 0;


    virtual void            loadCamParams() = 0;

    virtual cv::Mat         getRGB()        = 0;
    virtual cv::Mat         getDepth()      = 0;
    virtual cv::Mat         getIR()         = 0;
    virtual cv::Mat         getRGBD()       = 0;

    virtual cv::Mat         getMask()       = 0;
    virtual cv::Mat         getRangedRGBD() = 0;
    virtual cv::Mat         getRangedDepth()  = 0;
    virtual cv::Mat         getHistogram()    = 0;

    virtual camera_frames   getFrames()     = 0;

    void rangeFrames(cv::Mat rangedRGBD, cv::Mat rgbd, cv::Mat tmpMask);
    void morphFrames(cv::Mat  &tmpDepthMat, cv::Mat  &tmpRGBDMat, cv::Mat &tmpMask);
    void faceDetection(cv::Mat  rangedRGBD, cv::Mat  rgbd);

    bool getFramesReleasedCheck() {return new_frames_released; }
    void resetFramesReleased() { new_frames_released=false; }

    virtual pcl::PointCloud<pcl::PointXYZRGB> getCloudData()   = 0;
    std::shared_ptr<GlobalSettings> globalSettings = globalSettings->instance();

    Eigen::Matrix4d transformation_matrix;
    boost::property_tree::ptree pt;

    std::string serial, camera_type;
    int id=0;
    bool new_frames_released=false;

private:

};

#endif // CAMERA_H
