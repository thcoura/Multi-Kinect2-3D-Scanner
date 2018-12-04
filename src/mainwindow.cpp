#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle ("3D OSAS");

    Support.push_back(new support());
    Support[0]->cameraInit();                                       //! camera init, create connection with Kinects and Realsenses

    ui->graphicsView_rgbd_0->setScene(new QGraphicsScene(this));
    ui->graphicsView_depth_0->setScene(new QGraphicsScene(this));
    ui->graphicsView_ir_0->setScene(new QGraphicsScene(this));

    ui->graphicsView_rgbd_1->setScene(new QGraphicsScene(this));
    ui->graphicsView_depth_1->setScene(new QGraphicsScene(this));
    ui->graphicsView_ir_1->setScene(new QGraphicsScene(this));

    ui->graphicsView_rgbd_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_depth_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_ir_2->setScene(new QGraphicsScene(this));

    connect(Support[0], SIGNAL(newRGBD(QPixmap,int)), this, SLOT(onNewRGBD(QPixmap,int)));
    connect(Support[0], SIGNAL(newDepth(QPixmap,int)), this, SLOT(onNewDepth(QPixmap,int)));
    connect(Support[0], SIGNAL(newIR(QPixmap,int)), this, SLOT(onNewIR(QPixmap,int)));
    connect(Support[0], SIGNAL(newCloud()), this, SLOT(onNewCloud()));

    ui->graphicsView_rgbd_0->scene()->addItem(&rgbd[0]);
    ui->graphicsView_depth_0->scene()->addItem(&depth[0]);
    ui->graphicsView_ir_0->scene()->addItem(&ir[0]);
    ui->graphicsView_rgbd_1->scene()->addItem(&rgbd[1]);
    ui->graphicsView_depth_1->scene()->addItem(&depth[1]);
    ui->graphicsView_ir_1->scene()->addItem(&ir[1]);
    ui->graphicsView_rgbd_2->scene()->addItem(&rgbd[2]);
    ui->graphicsView_depth_2->scene()->addItem(&depth[2]);
    ui->graphicsView_ir_2->scene()->addItem(&ir[2]);

    viewer=new pcl::visualization::PCLVisualizer("viewer",false);
    ui->qvtkWidget->SetRenderWindow(viewer->getRenderWindow());
    viewer->setupInteractor(ui->qvtkWidget->GetInteractor(), ui->qvtkWidget->GetRenderWindow());
    ui->qvtkWidget->update ();
}

MainWindow::~MainWindow()
{
    delete ui;
    Support[0]->closeThreads();
}

void MainWindow::onNewCloud()
{

    viewer->removePointCloud("cloud");
    viewer->addPointCloud(Support[0]->merged_cloud->getCloud(), "cloud");
    viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "cloud");
    ui->qvtkWidget->update();
}

void MainWindow::onNewRGBD(QPixmap newFrm,int i)
{
    rgbd[i].setPixmap(newFrm);
}

void MainWindow::onNewDepth(QPixmap newFrm,int i)
{
    depth[i].setPixmap(newFrm);
}

void MainWindow::onNewIR(QPixmap newFrm,int i)
{
    ir[i].setPixmap(newFrm);
}

void MainWindow::on_pushButton_clicked()
{
    Support[0]->snap_running=true;
    Support[0]->cloudInit();
    Support[0]->threadsInit();                                      //! threads init, detached snapping and cloud computing started
}

void MainWindow::on_pushButton_2_clicked()
{
    Support[0]->closeThreads();
}

void MainWindow::on_pushButton_3_clicked()
{
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    Support[0]->changeComputeStyle(arg1);
}

void MainWindow::on_save_all_button_clicked()
{

    for(int i=0; i < Support[0]->getConnectedCams().size();i++)
    {
        cv::Mat tmpIR;
        Support[0]->getConnectedCams()[i]->getIR().convertTo(tmpIR,CV_8UC1,255,0);

        cv::imwrite("output/"+Support[0]->IntToStr(Support[0]->getConnectedCams()[i]->getId())+"/RGBD/RGBD_"+Support[0]->IntToStr(saved_frame_counter)+".jpeg",Support[0]->getConnectedCams()[i]->getRGBD());
        cv::imwrite("output/"+Support[0]->IntToStr(Support[0]->getConnectedCams()[i]->getId())+"/DEPTH/DEPTH_"+Support[0]->IntToStr(saved_frame_counter)+".jpeg",Support[0]->getConnectedCams()[i]->getDepth());
        cv::imwrite("output/"+Support[0]->IntToStr(Support[0]->getConnectedCams()[i]->getId())+"/RGB/RGB_"+Support[0]->IntToStr(saved_frame_counter)+".jpeg",Support[0]->getConnectedCams()[i]->getRGB());
        cv::imwrite("output/"+Support[0]->IntToStr(Support[0]->getConnectedCams()[i]->getId())+"/IR/IR_"+Support[0]->IntToStr(saved_frame_counter)+".jpeg",Support[0]->getConnectedCams()[i]->getIR() / 64);
    }
//    pcl::io::savePLYFile("output/CLOUDS/cloud_"+Support[0]->IntToStr(saved_frame_counter)+".ply",Support[0]->merged_cloud->getCloud());

    saved_frame_counter++;
}

void MainWindow::on_sequence_stop_clicked()
{
    Support[0]->getConnectedCams()[1]->stop();
    Support[0]->getConnectedCams()[2]->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    Support[0]->getConnectedCams()[0]->stop();
    Support[0]->getConnectedCams()[0]->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    Support[0]->getConnectedCams()[1]->stop();
    Support[0]->getConnectedCams()[1]->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    Support[0]->getConnectedCams()[2]->stop();
    Support[0]->getConnectedCams()[2]->stop();

}
