#include "kinect.h"

void Kinect::frames(std::atomic<bool> & keep_running)
{
    std::chrono::system_clock::time_point then, now;
    while(keep_running)
    {
        then=std::chrono::system_clock::now();
        if(this->pListener->waitForNewFrame(frame,camAttachTime))
        {
            try {
                    libfreenect2::Frame *depth=frame[libfreenect2::Frame::Depth];
                    libfreenect2::Frame *ir=frame[libfreenect2::Frame::Ir];
                    libfreenect2::Frame *rgb=frame[libfreenect2::Frame::Color];

                    this->pRegistrated->undistortDepth(depth,new_libfreenect_frames.undistortedDepth);
                    this->pRegistrated->apply(rgb,depth, new_libfreenect_frames.undistortedDepth, new_libfreenect_frames.registered,true, new_libfreenect_frames.depth2rgb);

                    cv::Mat(static_cast<int>(ir->height), static_cast<int>(ir->width), CV_32FC1, ir->data).copyTo(this->new_cam_frames.irMat);
                    cv::Mat(static_cast<int>(rgb->height), static_cast<int>(rgb->width), CV_8UC4, rgb->data).copyTo(this->new_cam_frames.colorMat);
                    cv::Mat(static_cast<int>(new_libfreenect_frames.undistortedDepth->height),  static_cast<int>(new_libfreenect_frames.undistortedDepth->width), CV_32FC1, new_libfreenect_frames.undistortedDepth->data).copyTo(this->new_cam_frames.depthMat);
                    cv::Mat(static_cast<int>(new_libfreenect_frames.registered->height),  static_cast<int>(new_libfreenect_frames.registered->width), CV_8UC4, new_libfreenect_frames.registered->data).copyTo(this->new_cam_frames.rgbdMat);

                    filterFrames();
                    computeHist();
                    cloudData(false);

                    new_frames_released=true;

                    cam_frames = new_cam_frames;

//                    now=std::chrono::system_clock::now();
//                    std::cout << "FRAME: "<<this->getId()<<" "<< std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count() << " ms" << std::endl;

                    pListener->release(frame);
                } catch(...)
                {
                    std::cout<<this->id<<" problem with Kinect::frames"<<std::endl;
                }
        }
        else
        {
            std::cout<<this->id<<" hasn't new frame"<<std::endl;
        }
    }
}

void Kinect::cloneFrames()
{
    cam_frames = new_cam_frames;
}

bool Kinect::cloudData(bool compute_cloud_style)
{
    std::chrono::system_clock::time_point then, now;
    then=std::chrono::system_clock::now();

    try {
            pcl::PointCloud<pcl::PointXYZRGB> tmpCloud;
            tmpCloud.clear();
//            tmpCloud.width = static_cast<uint32_t>(ir_depth_width);
//            tmpCloud.height = static_cast<uint32_t>(ir_depth_height);
//            tmpCloud.is_dense = false;

            if(compute_cloud_style==false)
                this->registered2cloud(tmpCloud);


            pcl::transformPointCloud(tmpCloud,tmpCloud,transformation_matrix,true);

            if(!tmpCloud.empty())
            {
                cloudInit(tmpCloud.points.size());
                pcl::copyPointCloud(tmpCloud,new_cam_frames.cloud);
            }
            tmpCloud.clear();
        }catch (...)
        {
            std::cout<<this->id<<" problem with  Kinect::cloudData"<<std::endl;
        }

//        now=std::chrono::system_clock::now();
//        std::cout << "CLOUD: "<<this->getId()<<" "<< std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count() << " ms" << std::endl;
}


void Kinect::cloudInit(size_t size)
{
    new_cam_frames.cloud.clear();
//    new_cam_frames.cloud.points.resize(size);
//    new_cam_frames.cloud.height=1;
//    new_cam_frames.cloud.width=static_cast<uint32_t>(size);
}

void Kinect::registered2cloud(pcl::PointCloud<pcl::PointXYZRGB> &tmpCloud)
{
    float x=0, y=0, z=0;
    unsigned long n=0;
    for(int x_side=0;x_side<ir_depth_width;x_side++)
    {
        for(int y_side=0;y_side<ir_depth_height;y_side++)
        {
            float rgb;
            pRegistrated->getPointXYZRGB(libfreenect_frames.undistortedDepth,libfreenect_frames.registered,y_side,x_side,x,y,z,rgb);
            const uint8_t *p = reinterpret_cast<uint8_t*>(&rgb);
            uint8_t b = p[0];
            uint8_t g = p[1];
            uint8_t r = p[2];

            if(std::isinf(x) || std::isinf(y) ||  std::isinf(z) || std::isnan(x) || std::isnan(y) || std::isnan(z) || z > static_cast<float>(maximal_depth))
            {
//                x=NAN;
//                z=NAN;
//                y=NAN;
            }
            else
            {
                pcl::PointXYZRGB point;
                point.x=x;
                point.y=y;
                point.z=z;
                point.r=r;
                point.g=g;
                point.b=b;
                tmpCloud.points.push_back(point);
            }
            n++;
        }
    }
    std::vector<int> removedPoints;
    pcl::removeNaNFromPointCloud(tmpCloud,tmpCloud,removedPoints);
    tmpCloud.resize(tmpCloud.points.size());
}
