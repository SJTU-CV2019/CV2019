#ifndef DYEING_H
#define DYEING_H
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
#include <QQueue>
#include <QSet>
#include <QtAlgorithms>
#include <vector>
#include <QVector>

#define ALPHA 0.8

#define MAX_SIZE 1000

/*
在mainwindow.cpp加上
	#include "dyeing.h"
代码最后加上
	cv::Mat dye_img=dyeing(result,circles,circleattrs);
	QSize laSize=ui->oldpic->size();
	oPic = QImage((const unsigned char*)dye_img.data, dye_img.cols, dye_img.rows, dye_img.cols*dye_img.channels(), QImage::Format_RGB888);
	oPic = oPic.scaled(laSize,Qt::IgnoreAspectRatio);
	ui->oldpic->setPixmap(QPixmap::fromImage(oPic));
*/

//如果像素超出范围，返回false
//否则返回true
bool checkboundary(int x_pos,int y_pos,int row,int col,std::vector<cv::Mat> &channels){
    if(x_pos<0||x_pos>=col||y_pos<0||y_pos>=row){return false;}
    if(channels[0].at<uchar>(y_pos,x_pos)!=0||channels[1].at<uchar>(y_pos,x_pos)!=0||channels[2].at<uchar>(y_pos,x_pos)!=0){return true;}
    else return false;
}

//如果存在重复的像素，返回false
//否则返回true
bool checkrepeat(int pos,QSet<int> &dyed){
    if(dyed.find(pos)==dyed.end())return true;
    else return false;
}



//已填充未扩张 queue+set
//已填充已扩张 set
//未填充
cv::Mat dyeing(cv::Mat &src_mat,std::vector<cv::Vec3f>& circles,std::vector<bool> &circleattrs,QVector<QString>* info ){



    QSet<int> dyed;    //记录已检查过的像素点
    int rgb[3];     //染料颜色
    std::vector<cv::Mat> channels;
    cv::split(src_mat, channels);

    int row=src_mat.rows;
    int col=src_mat.cols;
    int num=circles.size();
    //row为像素行数
    //col为像素列数
    //num为瓶盖个数
    qDebug()<<"row="<<row<<"col="<<col<<"num="<<num;

    //为正面和反面染色-----------------------
    //------------------------------------

    //得到了原图像和瓶盖的中心位置
    //逐个染色瓶盖
    //位置信息被保存在队列中
    //每次取出一个坐标，放入八个坐标（如果是黑色的像素/已填充的像素，则不放入）
    //循环直到队列为空
    for(int i=0;i<num;i++){

        int x_src=int(circles[i][0]);
        int y_src=int(circles[i][1]);
        int r=int(circles[i][2]);

        //记录数据（正反面）
        info[1].push_back(QString::number(x_src));
        info[2].push_back(QString::number(y_src));
        info[0].push_back(QString::number(r));
        if(circleattrs[i]==true)info[3].push_back("反面");
        else info[3].push_back("正面");


        if(circleattrs[i]==true){
            rgb[0]=1;rgb[1]=0;rgb[2]=0;
        }
        else {
            rgb[0]=0;rgb[1]=1;rgb[2]=0;
        }


        //遍历每个瓶盖
        QQueue<int> dyer;   //通过其中的像素点检查其余像素点

        bool flag=false;

        //对第一个点着色
        if(checkboundary(x_src,y_src,row,col,channels)&&checkrepeat(x_src+y_src*col,dyed)){
            flag=true;
        }
        else if(checkboundary(x_src+r/2,y_src,row,col,channels)&&checkrepeat(x_src+r/2+y_src*col,dyed)){
            flag=true;
            x_src+=r/2;
        }
        else if(checkboundary(x_src-r/2,y_src,row,col,channels)&&checkrepeat(x_src-r/2+y_src*col,dyed)){
            flag=true;
            x_src-=r/2;
        }
        else if(checkboundary(x_src-r/2,y_src,row,col,channels)&&checkrepeat(x_src-r/2+y_src*col,dyed)){
            flag=true;
            x_src-=r/2;
        }
        else if(checkboundary(x_src,y_src+r/2,row,col,channels)&&checkrepeat(x_src+(y_src+r/2)*col,dyed)){
            flag=true;
            y_src+=r/2;
        }
        else if(checkboundary(x_src,y_src-r/2,row,col,channels)&&checkrepeat(x_src+(y_src-r/2)*col,dyed)){
            flag=true;
            y_src-=r/2;
        }


        if(flag==false){qDebug()<<"error circles infomation!"<<endl;}
        dyer.push_back(y_src*col+x_src);
        qDebug()<<"i="<<i<<"x="<<x_src<<"y="<<y_src<<"c="<<circleattrs[i];

        for(int j=0;j<3;j++) channels[j].at<uchar>(y_src,x_src)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_src,x_src)*(1-ALPHA);
        dyed.insert(y_src*col+x_src);

        //放入-取出队列
        while(!dyer.empty()){
            //cnt++;
            //从头部取出像素点
            int x_pos=dyer.front()%col;
            int y_pos=dyer.front()/col;
            dyer.pop_front();

            //取周围的邻点，如果满足条件，着色并放入队列尾部
            //并且要把该点加入着色点集中
            int x_tmp;
            int y_tmp;
            int tmp;

            x_tmp=x_pos;
            y_tmp=y_pos-1;
            tmp=y_tmp*col+x_tmp;
            if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                dyed.insert(tmp);
                dyer.push_back(tmp);
            }

            x_tmp=x_pos-1;
            y_tmp=y_pos;
            tmp=y_tmp*col+x_tmp;
            if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                dyed.insert(tmp);
                dyer.push_back(tmp);
            }

            x_tmp=x_pos+1;
            y_tmp=y_pos;
            tmp=y_tmp*col+x_tmp;
            if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                dyed.insert(tmp);
                dyer.push_back(tmp);
            }

            x_tmp=x_pos;
            y_tmp=y_pos+1;
            tmp=y_tmp*col+x_tmp;
            if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                dyed.insert(tmp);
                dyer.push_back(tmp);
            }

        }
    }

    //------------------------------------
    //------------------------------------



    //为侧面瓶盖染色并记录坐标值---------------
    //------------------------------------

    //找到一个未着色的像素
    //以其为起始填充整篇区域
    //记录整片区域所有的像素
    //如果像素点数量足够，则着色（排除噪音）
    //取像素的平均值得到坐标

    rgb[0]=0;rgb[1]=0;rgb[2]=1;
    for(int j=0;j<row;j++){
        for(int i=0;i<col;i++){
            if(checkrepeat(i+j*col,dyed)&&checkboundary(i,j,row,col,channels)){
                QQueue<int> dyer;   //通过其中的像素点检查其余像素点
                QVector<int> pre;   //记录暂时记忆的像素点，在确定不是噪音后再染色（与正反染色不同）
                dyer.push_back(j*col+i);
                pre.push_back(j*col+i);
                while(!dyer.empty()){
                    //从头部取出像素点
                    int x_pos=dyer.front()%col;
                    int y_pos=dyer.front()/col;
                    dyer.pop_front();
                    int x_tmp;
                    int y_tmp;
                    int tmp;

                    x_tmp=x_pos;
                    y_tmp=y_pos-1;
                    tmp=y_tmp*col+x_tmp;
                    if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                        //for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                        //暂时保存起来并不立刻填色
                        pre.push_back(tmp);
                        dyed.insert(tmp);
                        dyer.push_back(tmp);
                    }

                    x_tmp=x_pos-1;
                    y_tmp=y_pos;
                    tmp=y_tmp*col+x_tmp;
                    if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                        //for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                        pre.push_back(tmp);
                        dyed.insert(tmp);
                        dyer.push_back(tmp);
                    }

                    x_tmp=x_pos+1;
                    y_tmp=y_pos;
                    tmp=y_tmp*col+x_tmp;
                    if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                        //for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                        pre.push_back(tmp);
                        dyed.insert(tmp);
                        dyer.push_back(tmp);
                    }

                    x_tmp=x_pos;
                    y_tmp=y_pos+1;
                    tmp=y_tmp*col+x_tmp;
                    if(checkboundary(x_tmp,y_tmp,row,col,channels)&&checkrepeat(tmp,dyed)){
                        //for(int j=0;j<3;j++) channels[j].at<uchar>(y_tmp,x_tmp)=255*rgb[j]*ALPHA+channels[j].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                        pre.push_back(tmp);
                        dyed.insert(tmp);
                        dyer.push_back(tmp);
                    }

                }

                //这里完成两件事，染色记忆的像素，计算平均位置
                if(pre.size()>MAX_SIZE){
                    int x_avg=0;
                    int y_avg=0;
                    QVector<int>::iterator iter;
                    for (iter=pre.begin();iter!=pre.end();iter++)
                    {
                       int x_tmp=*iter%col;
                       int y_tmp=*iter/col;
                       x_avg+=x_tmp;
                       y_avg+=y_tmp;
                       for(int k=0;k<3;k++) channels[k].at<uchar>(y_tmp,x_tmp)=255*rgb[k]*ALPHA+channels[k].at<uchar>(y_tmp,x_tmp)*(1-ALPHA);
                    }
                    x_avg/=pre.size();
                    y_avg/=pre.size();
                    //qDebug()<<"i="<<i<<"j="<<j<<"size"<<pre.size();
                    qDebug()<<"x="<<x_avg<<"y="<<y_avg<<"size"<<pre.size();
                    info[1].push_back(QString::number(x_avg));
                    info[2].push_back(QString::number(y_avg));
                    info[0].push_back("--");
                    info[3].push_back("侧面");
                }
            }

        }
    }

    //----------------------------------
    //----------------------------------


    //输出图像
    cv::Mat dst_mat;
    cv::merge(channels,dst_mat);
    return dst_mat;
}


#endif // DYEING_H
