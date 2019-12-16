#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include<algorithm>

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
    fileFull = QFileDialog::getOpenFileName(this,tr("file"),"/",tr("text(*.jpg)"));  //获取整个文件名

    if(!fileFull.isNull())
    {
        oldimage = cv::imread(fileFull.toStdString(), 1);
        cv::Mat image;
        cv::cvtColor(oldimage, image, cv::COLOR_BGR2RGB);
        cv::Mat result;
        cv::medianBlur(image, result,15);
        cv::Mat gray, out;
        cv::cvtColor(result, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, out, 10, 200, 3, false);
        cv::imshow("out", out);
        oPic = QImage((const unsigned char*)image.data, image.cols, image.rows, image.cols*image.channels(), QImage::Format_RGB888);
        //namedWindow("1");
        ui->oldpic->setPixmap(QPixmap::fromImage(oPic));
        ui->oldpic->resize(oPic.size());
    }
}
