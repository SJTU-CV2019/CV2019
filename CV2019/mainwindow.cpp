#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <algorithm>
#include <vector>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "dyeing.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pos=new QTableWidget();
    pos->setParent(this);
    pos->setGeometry(50,110,200,500);
    pos->setColumnCount(4);
    pos->setColumnWidth(0, 40);
    pos->setColumnWidth(1, 40);
    pos->setColumnWidth(2, 40);
    pos->setColumnWidth(3, 60);
    QStringList sListHeader;
    sListHeader<< "r"<<"x"<<"y" <<"状态";
    pos->setHorizontalHeaderLabels(sListHeader);
    pos->show();

    src_img=new QLabel();
    src_img->show();
    src_img->setParent(this);
    src_img->setGeometry(300,100,245,245);
    src_img->setText("");
    src_img->show();

    mid_img=new QLabel();
    mid_img->show();
    mid_img->setParent(this);
    mid_img->setGeometry(300,360,245,245);
    mid_img->setText("");
    mid_img->show();

    dst_img=new QLabel();
    dst_img->show();
    dst_img->setParent(this);
    dst_img->setGeometry(600,100,500,500);
    dst_img->setText("");
    dst_img->show();



}

MainWindow::~MainWindow()
{
    delete ui;
}

#define SHADOW 80

void MainWindow::on_pushButton_2_clicked()
{
    QString fileFull;
    fileFull = QFileDialog::getOpenFileName(this,tr("file"),"/",tr("text(*.bmp)"));  //获取整个文件名

    if(!fileFull.isNull())
    {
        oldimage = cv::imread(fileFull.toStdString(), 1);
        qDebug() << oldimage.type();
        cv::Mat image = oldimage.clone();
        std::vector<cv::Mat> channels;
        cv::split(oldimage, channels);
        int rows = oldimage.rows;
        int cols = oldimage.cols;
        for (int i = 0; i < rows; i++)
        {
            for (int j =0; j < cols; j++)
            {
                if(channels[0].at<uchar>(i,j) < SHADOW && channels[1].at<uchar>(i,j) <SHADOW  && channels[2].at<uchar>(i,j) < SHADOW)
                {
                    channels[0].at<uchar>(i,j) = 0;
                    channels[1].at<uchar>(i,j) = 0;
                    channels[2].at<uchar>(i,j) = 0;
                }
            }
        }
        cv::Mat result;
        cv::merge(channels, result);


        cv::Mat gray;
        cv::cvtColor(result, gray, cv::COLOR_RGB2GRAY);
        cv::Mat arr = gray.clone();
        cv::resize(gray, gray, cv::Size(500,500));

        cv::GaussianBlur(arr, arr, cv::Size(3, 3), 3, 3);  //用高斯模糊平滑图像，去除不必要的噪点（如边缘突起）
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(arr, circles, cv::HOUGH_GRADIENT, 1, arr.rows / 10, 200, 50, 0, 0);
        //霍夫变换找圆， circles中存储圆心和半径

        for (size_t i = 0; i < circles.size(); i++)
        {
            cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            //绘制圆心
            circle(image, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
            //绘制圆轮廓
            circle(image, center, radius, cv::Scalar(255, 30, 0), 3, 8, 0);
        }
        std::vector<bool> circleattrs;
        std::vector<cv::Mat> cuttingcircles;
        for (size_t i = 0; i < circles.size(); i++)
        {
            cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            cv::Mat cuttingimage;
            if((cvRound(circles[i][0])-radius)>0&&(cvRound(circles[i][1])-radius)>0&&(cvRound(circles[i][0])+radius)<cols&&(cvRound(circles[i][1])+radius)<rows)
            {
                cuttingimage= cv::Mat(oldimage, cv::Rect(cvRound(circles[i][0])-radius,cvRound(circles[i][1])-radius,2*radius,2*radius)).clone();
                cuttingcircles.push_back(cuttingimage);
            }
            else if((cvRound(circles[i][0])-radius)<=0&&(cvRound(circles[i][1])-radius)>0&&(cvRound(circles[i][0])+radius)<cols&&(cvRound(circles[i][1])+radius)<rows)
            {
                cuttingimage= cv::Mat(oldimage, cv::Rect(0,cvRound(circles[i][1])-radius,2*radius,2*radius)).clone();
                cuttingcircles.push_back(cuttingimage);
            }
            else if((cvRound(circles[i][0])-radius)>0&&(cvRound(circles[i][1])-radius)<=0&&(cvRound(circles[i][0])+radius)<cols&&(cvRound(circles[i][1])+radius)<rows)
            {
                cuttingimage= cv::Mat(oldimage, cv::Rect(cvRound(circles[i][0])-radius,0,2*radius,2*radius)).clone();
                cuttingcircles.push_back(cuttingimage);
            }
            else if((cvRound(circles[i][0])-radius)<=0&&(cvRound(circles[i][1])-radius)<=0&&(cvRound(circles[i][0])+radius)<cols&&(cvRound(circles[i][1])+radius)<rows)
            {
                cuttingimage= cv::Mat(oldimage, cv::Rect(0,0,2*radius,2*radius)).clone();
                cuttingcircles.push_back(cuttingimage);
            }
            else if((cvRound(circles[i][0])-radius)>0&&(cvRound(circles[i][1])-radius)>0&&(cvRound(circles[i][0])+radius)>=cols&&(cvRound(circles[i][1])+radius)<rows)
            {
                cuttingimage= cv::Mat(oldimage, cv::Rect(cvRound(circles[i][0])-radius,cvRound(circles[i][1])-radius,2*radius,rows-cvRound(circles[i][1])+radius-1)).clone();
                cuttingcircles.push_back(cuttingimage);
            }
            else if((cvRound(circles[i][0])-radius)>0&&(cvRound(circles[i][1])-radius)>0&&(cvRound(circles[i][0])+radius)<cols&&(cvRound(circles[i][1])+radius)>=rows)
            {
                cuttingimage= cv::Mat(oldimage, cv::Rect(cvRound(circles[i][0])-radius,cvRound(circles[i][1])-radius,cols-cvRound(circles[i][0])+radius-1,2*radius)).clone();
                cuttingcircles.push_back(cuttingimage);
            }
            else if((cvRound(circles[i][0])-radius)>0&&(cvRound(circles[i][1])-radius)>0&&(cvRound(circles[i][0])+radius)>=cols&&(cvRound(circles[i][1])+radius)>=rows)
            {
                cuttingimage= cv::Mat(oldimage, cv::Rect(cvRound(circles[i][0])-radius,cvRound(circles[i][1])-radius,cols-cvRound(circles[i][0])+radius-1,rows-cvRound(circles[i][1])+radius-1)).clone();
                cuttingcircles.push_back(cuttingimage);
            }


        }
        for(size_t i = 0; i < cuttingcircles.size(); i++)
        {
            cv::Mat srcImage=cuttingcircles[i];
            cv::Mat midImage, dstImage;

            cv::cvtColor(srcImage, midImage, cv::COLOR_RGB2GRAY);//转化边缘检测后的图为灰度图
            GaussianBlur(midImage, midImage, cv::Size(9, 9), 2, 2);
            std::vector<cv::Vec3f> circles1;
            HoughCircles(midImage, circles1, cv::HOUGH_GRADIENT, 2, midImage.rows/20, 100, 80, 0, 0);
            //依次在图中绘制出圆
            int count  =0;
            for (size_t i = 0; i < circles1.size(); i++)
            {
                cv::Point center(cvRound(circles1[i][0]), cvRound(circles1[i][1]));
                int radius = cvRound(circles1[i][2]);
                if((cvRound(circles1[i][0])-radius)>0&&(cvRound(circles1[i][1])-radius)>0&&(cvRound(circles1[i][0])+radius)<srcImage.cols&&(cvRound(circles1[i][1])+radius)<srcImage.rows)
                {
                    //绘制圆心
                    circle(srcImage, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
                    //绘制圆轮廓
                    count++;
                    circle(srcImage, center, radius, cv::Scalar(155, 50, 255), 3, 8, 0);
                }
            }
            if(count>=2)
            {
                circleattrs.push_back(true);
            }
            else
            {
                circleattrs.push_back(false);
            }

        }

        QVector<QString> info[4];
        cv::Mat dye_img=dyeing(result,circles,circleattrs,info);

        cv::cvtColor(oldimage, oldimage, cv::COLOR_BGR2RGB);
        cv::cvtColor(dye_img, dye_img, cv::COLOR_BGR2RGB);
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

        oPic[0] = QImage((const unsigned char*)dye_img.data, dye_img.cols, dye_img.rows, dye_img.cols*dye_img.channels(), QImage::Format_RGB888);
        oPic[1] = QImage((const unsigned char*)oldimage.data, oldimage.cols, oldimage.rows, oldimage.cols*oldimage.channels(), QImage::Format_RGB888);
        oPic[2] = QImage((const unsigned char*)image.data, image.cols, image.rows, image.cols*image.channels(), QImage::Format_RGB888);

        oPic[0] = oPic[0].scaled(dst_img->size(),Qt::IgnoreAspectRatio);
        oPic[1] = oPic[1].scaled(src_img->size(),Qt::IgnoreAspectRatio);
        oPic[2] = oPic[2].scaled(mid_img->size(),Qt::IgnoreAspectRatio);
        dst_img->setPixmap(QPixmap::fromImage(oPic[0]));
        src_img->setPixmap(QPixmap::fromImage(oPic[1]));
        mid_img->setPixmap(QPixmap::fromImage(oPic[2]));

        //QVector<int>::iterator iter[4];
        int len=info[0].size();
        pos->clearContents();
        for(int i=0;i<len;i++){
            int iRow =pos->rowCount();
            if(iRow<i+1)pos->setRowCount(iRow + 1);
            pos->setItem(i,0,new QTableWidgetItem(info[0][i]));
            pos->setItem(i,1,new QTableWidgetItem(info[1][i]));
            pos->setItem(i,2,new QTableWidgetItem(info[2][i]));
            pos->setItem(i,3,new QTableWidgetItem(info[3][i]));
        }



    }
}
