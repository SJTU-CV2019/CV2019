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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString fileFull;
    fileFull = QFileDialog::getOpenFileName(this,tr("file"),"/",tr("text(*.bmp)"));  //获取整个文件名

    if(!fileFull.isNull())
    {
        oldimage = cv::imread(fileFull.toStdString(), 1);
        cv::Mat image;
        cv::cvtColor(oldimage, image, cv::COLOR_BGR2RGB);
        //image - RGB

        std::vector<cv::Mat> channels;
        cv::split(oldimage, channels);
        int rows = oldimage.rows;
        int cols = oldimage.cols;
        for (int i = 0; i < rows; i++)
        {
            for (int j =0; j < cols; j++)
            {
                if(channels[0].at<uchar>(i,j) < 80 && channels[1].at<uchar>(i,j) < 80 && channels[2].at<uchar>(i,j) < 80)
                {
                    channels[0].at<uchar>(i,j) = 0;
                    channels[1].at<uchar>(i,j) = 0;
                    channels[2].at<uchar>(i,j) = 0;
                }
                else
                {
                    channels[0].at<uchar>(i,j) = 255;
                    channels[1].at<uchar>(i,j) = 255;
                    channels[2].at<uchar>(i,j) = 255;
                }
            }
        }
        cv::Mat result;
        cv::merge(channels, result);
        //分轨道处理，消除背景影响
        //cv::medianBlur(result, result,3);


        QSize laSize=ui->label->size();
        QImage label1 = QImage((const unsigned char*)result.data, result.cols, result.rows, result.cols*result.channels(), QImage::Format_RGB888);
        label1 = label1.scaled(laSize,Qt::IgnoreAspectRatio);
        ui->label->setPixmap(QPixmap::fromImage(label1));
        //显示上述处理后图片

        cv::Mat gray, out, out1, out2;
        cv::cvtColor(result, gray, cv::COLOR_RGB2GRAY);
        //gray - 灰度图像
        cv::Canny(gray, out, 5, 250, 3, false);
//        cv::Sobel(gray, out1, gray.depth(), 1, 0);
//        cv::Sobel(gray, out2, gray.depth(), 0, 1);
//        cv::addWeighted(out1, 1, out2, 1, 0.0, out);
        cv::resize(out, out, cv::Size(500,500));
        cv::imshow("out", out);

        //out - 边缘处理
        cv::Mat arr = gray.clone();
        cv::resize(gray, gray, cv::Size(500,500));
        cv::imshow("gray", gray);



        cv::GaussianBlur(arr, arr, cv::Size(5,5), 3, 3);  //用高斯模糊平滑图像，去除不必要的噪点（如边缘突起）
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(arr, circles, cv::HOUGH_GRADIENT, 1, arr.rows/5, 150, 100, 0, 0);
        //霍夫变换找圆， circles中存储圆心和半径

        for (size_t i = 0; i < circles.size(); i++)
        {
            cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            //绘制圆心
            circle(out, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
            //绘制圆轮廓
            circle(out, center, radius, cv::Scalar(255, 50, 0), 3, 8, 0);
        }




        laSize=ui->oldpic->size();
        oPic = QImage((const unsigned char*)image.data, image.cols, image.rows, image.cols*image.channels(), QImage::Format_RGB888);
        //namedWindow("1");
        oPic = oPic.scaled(laSize,Qt::IgnoreAspectRatio);
        ui->oldpic->setPixmap(QPixmap::fromImage(oPic));
    }
}

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
                if(channels[0].at<uchar>(i,j) < 80 && channels[1].at<uchar>(i,j) < 80 && channels[2].at<uchar>(i,j) < 80)
                {
                    channels[0].at<uchar>(i,j) = 0;
                    channels[1].at<uchar>(i,j) = 0;
                    channels[2].at<uchar>(i,j) = 0;
                }
//                else
//                {
//                    channels[0].at<uchar>(i,j) = 255;
//                    channels[1].at<uchar>(i,j) = 255;
//                    channels[2].at<uchar>(i,j) = 255;
//                }
            }
        }
        cv::Mat result;
        cv::merge(channels, result);


        cv::Mat gray;
        cv::cvtColor(result, gray, cv::COLOR_RGB2GRAY);
        cv::Mat arr = gray.clone();
        cv::resize(gray, gray, cv::Size(500,500));
        cv::imshow("gray", gray);

        cv::GaussianBlur(arr, arr, cv::Size(3, 3), 3, 3);  //用高斯模糊平滑图像，去除不必要的噪点（如边缘突起）
        cv::imshow("arr", arr);
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(arr, circles, cv::HOUGH_GRADIENT, 1, arr.rows / 10, 200, 30, 0, 0);
        //霍夫变换找圆， circles中存储圆心和半径

        for (size_t i = 0; i < circles.size(); i++)
        {
            cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            //绘制圆心
            circle(image, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
            //绘制圆轮廓
            circle(image, center, radius, cv::Scalar(255, 50, 0), 3, 8, 0);
        }

        std::vector<cv::Mat> cuttingcircles;
        for (size_t i = 0; i < circles.size(); i++)
        {
            cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            cv::Mat cuttingimage= cv::Mat(oldimage, cv::Rect(cvRound(circles[i][0])-radius,cvRound(circles[i][1])-radius,2*radius,2*radius)).clone();
            cuttingcircles.push_back(cuttingimage);
        }
        for(size_t i = 0; i < cuttingcircles.size(); i++)
        {
            cv::Mat srcImage=cuttingcircles[i];
            cv::Mat midImage, dstImage;
            imshow("【原始图】", srcImage);

            cv::cvtColor(srcImage, midImage, cv::COLOR_RGB2GRAY);//转化边缘检测后的图为灰度图
            GaussianBlur(midImage, midImage, cv::Size(9, 9), 2, 2);
            std::vector<cv::Vec3f> circles1;
            HoughCircles(midImage, circles1, cv::HOUGH_GRADIENT, 2, midImage.rows/20, 100, 100, 0, 0);
            //依次在图中绘制出圆
            for (size_t i = 0; i < circles1.size(); i++)
            {
                cv::Point center(cvRound(circles1[i][0]), cvRound(circles1[i][1]));
                int radius = cvRound(circles1[i][2]);
                //绘制圆心
                circle(srcImage, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
                //绘制圆轮廓
                circle(srcImage, center, radius, cv::Scalar(155, 50, 255), 3, 8, 0);
            }

            imshow("【效果图】"+std::to_string(i), srcImage);

        }

        imshow("Output",image);

    }
}
