#ifndef KINECT_H
#define KINECT_H

#include "camera.h"

#include <string>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <fstream>
#include <memory>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/frame_listener.hpp>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

#define chrono_counter

#define EIGEN_MALLOC_ALREADY_ALIGNED 0

#define camAttachTime 2000
#define ir_depth_width 512
#define ir_depth_height 424
#define ir_depth_bpp 4
#define color_width 1920
#define color_height 1080+2
#define color_bpp 4
#define minimal_depth 350
#define maximal_depth 800
#define mutex_lock_time 15
#define frame_mutex_lock_time 10

class Kinect : public Camera
{
public:

    Kinect(libfreenect2::Freenect2 *_freenect, int id);     // kinect object
    void        start();                                    // kinect start
    void        stop();

    void        registration();                             // kinect register
    void        frames(std::atomic<bool> & keep_running);   // kinect wrapper to opencv

    void        cloneFrames();

    cv::Mat     getRGB()         { return cam_frames.colorMat; }                                    // return opencv mat
    cv::Mat     getDepth()       { return cam_frames.depthMat; }
    cv::Mat     getIR()          { return cam_frames.irMat; }
    cv::Mat     getRGBD()        { return cam_frames.rgbdMat; }
    cv::Mat     getMask()        { return cam_frames.mask; }
    cv::Mat     getRangedDepth() { return cam_frames.rangedDepthMat; }
    cv::Mat     getRangedRGBD()  { return cam_frames.rangedRGBDMat; }
    cv::Mat     getHistogram()   { return cam_frames.histMat; }

    camera_frames   getFrames()  { return cam_frames; }


    void        depth2cloud(pcl::PointCloud<pcl::PointXYZRGB>::Ptr  &tmpCloud);
    void        registered2cloud(pcl::PointCloud<pcl::PointXYZRGB> &tmpCloud);

    bool        cloudData(bool);
    void        filterFrames();
    void        computeHist();


    void saveLUT(cv::Mat depth, cv::Mat rgbd, std::string filename,int counter);


    void cloudInit(size_t size);                                // prepare cloud for copying
    pcl::PointCloud<pcl::PointXYZRGB> getCloudData() { return cam_frames.cloud; }


    void loadCamParams();

    bool lockFrames(int lock_time);
    void unlockFrames();

    ~Kinect();

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

    libfreenect2::PacketPipeline * pPipeline            = new libfreenect2::CudaKdePacketPipeline();
    libfreenect2::Freenect2 * pFreenect                 = nullptr;
    libfreenect2::Freenect2Device * pDev                = nullptr;
    libfreenect2::Registration * pRegistrated           = nullptr;
    libfreenect2::SyncMultiFrameListener * pListener    = nullptr;
    libfreenect2::FrameMap frame;

    struct libfnct_frames
    {
        libfreenect2::Frame *undistortedDepth = new libfreenect2::Frame(ir_depth_width, ir_depth_height, ir_depth_bpp);
        libfreenect2::Frame *undistorted    = new libfreenect2::Frame(ir_depth_width, ir_depth_height, ir_depth_bpp);
        libfreenect2::Frame *registered     = new libfreenect2::Frame(ir_depth_width, ir_depth_height, ir_depth_bpp);
        libfreenect2::Frame *depth2rgb      = new libfreenect2::Frame(color_width,color_height, color_bpp);
    };




    std::timed_mutex frame_mutex;

    libfreenect2::Freenect2Device::ColorCameraParams    rgb_calib_params;
    libfreenect2::Freenect2Device::IrCameraParams       ir_calib_params;

    camera_frames new_cam_frames;
    camera_frames cam_frames;
    libfnct_frames new_libfreenect_frames;
    libfnct_frames libfreenect_frames;
};
#endif // KINECT_H
