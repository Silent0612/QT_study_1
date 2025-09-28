#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QDebug>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent),recognition_finished(true),is_regist(false)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    if(!init())
    {
        exit(-1);
    }
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

bool ClientWindow::init()
{
    //打开摄像头
    if(!capture.open(0))
    {
        qDebug() <<"打开摄像头失败！";
        return false;
    }
    //启动定时器，获取摄像头画面数据
    startTimer(100);
    //加载级联分类器
    cascade.load("D:/opencv452/etc/haarcascades/haarcascade_frontalface_alt2.xml");
    //设置定时器的超时时间(每三秒连接一次服务器)
    timer_connect_to_server.setInterval(3000);
    //连接信号与槽:当定时器的定时时间到了就连接服务器
    connect(&timer_connect_to_server, SIGNAL(timeout()), this, SLOT(slotConnectToServer()));
    //当客户端与服务器断开连接，套接字就会发出disconnected（）的信号
    connect(&socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    //当客户端成功连接到服务器，套接字就会发出 connected() 的信号
    connect(&socket,SIGNAL(connected()),this,SLOT(slotConnected()));
    //当服务器发送了数据过来的时候，套接字会发出 readyRead() 的信号
    connect(&socket,SIGNAL(readyRead()),this,SLOT(slotSocketReadyRead()));
    //如果客户端没有连上服务器，就启动定时器连接服务器
    if(socket.state() != QAbstractSocket::ConnectedState)
    {
        slotDisconnected();
    }
    ui->label_verify_icon->hide();
    //
    connect(&timer_next, SIGNAL(timeout()), this, SLOT(slotTimerNext()));
    return true;
}

void ClientWindow::timerEvent(QTimerEvent *e)
{
    //显示摄像头
    cv::Mat frame;
    if(!capture.isOpened())
    {
        qDebug() << "摄像头没有打开!";
        return;
    }
    if(!capture.read(frame))
    {
        qDebug() << "读取摄像头数据失败！";
        return;
    }
    if(frame.data == nullptr)
    {
        qDebug() << "没有读到摄像头数据!";
        return;
    }
    //检测人脸
    //将图片转为灰度图
    cv::Mat frame_gary;
    cv::cvtColor(frame, frame_gary, cv::COLOR_BGR2GRAY);
    std::vector<cv::Rect> faces;
    cascade.detectMultiScale(frame_gary, faces);
    if(faces.size() > 0)
    {
        //至少识别到一个人脸,标记每个人脸位置，遍历容器
        for(unsigned int i = 0; i < faces.size(); i++)
        {
            //获取一个人脸矩形区域
            cv::Rect rect = faces.at(i);
            //
            //cv::rectangle(frame, rect, cv::Scalar(0, 0, 255));
            //
            ui->label_facial_area->resize(rect.width+40, rect.height+40);
            ui->label_facial_area->move(rect.x, rect.y);
        }
        //将含有人脸数据的画面发送给服务器
        //如果服务器处理完了前面发送的数据，发送新的数据
        if((recognition_finished or is_regist) and !is_verify_success)
        {
            //修改界面中结果界面的内容
            ui->label_verify_text->setText("识别中...");
            ui->label_verify_icon->hide();
            ui->widget_verify_rest->show();
            //发送画面给服务器
            //将opencv::Mat数据转换为Qt::QByteArray数据，再按照jpg的图片格式发送给服务器
            std::vector<uchar> buf; //用来保存转换后的数据
            if(is_regist)
            {
                cv::imencode(".jpg",frame,buf);
            }
            else
            {
                cv::imencode(".jpg",frame_gary,buf);
            }
            //将图像数据转换为二进制数据
            QByteArray byte_data((const char*)buf.data(),buf.size());
            //计算数据包的大小
            quint64 data_size = byte_data.size();
            QByteArray send_data;   //用来保存最终发送的数据
            //构建一个数据流对象(方便添加数据头等信息 -> 打包数据)，并绑定到字节数据对象
            QDataStream data_stream(&send_data,QIODevice::WriteOnly);
            //考虑服务器版本兼容性的问题，在打包数据的时候，指定Qt版本
            data_stream.setVersion(QDataStream::Qt_6_9);
            //打包数据
            data_stream << data_size << byte_data;
            //发送数据
            socket.write(send_data);
            //等待服务器处理完数据再发送数据
            recognition_finished = false;
        }

    }
    else
    {
        //没有检测到人脸，将人脸框恢复初识位置
        ui->label_facial_area->resize(260,260);
        ui->label_facial_area->move(110,30);
    }
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step1(),QImage::Format_RGB888);
    ui->label_video->setPixmap(QPixmap::fromImage(img));
}

void ClientWindow::slotConnectToServer()
{
    socket.connectToHost("127.0.0.1", 45678);
    ui->label_verify_text->setText("连接中...");
    ui->label_verify_icon->hide();
    ui->widget_verify_rest->show();
    qDebug() << "slotConnectToServer";
}

void ClientWindow::slotDisconnected()
{
    slotConnectToServer();
    timer_connect_to_server.start();
    qDebug() << "slotDisconnected";
}

void ClientWindow::slotConnected()
{
    ui->widget_verify_rest->hide();
    timer_connect_to_server.stop();
    qDebug() << "slotConnected";
}

void ClientWindow::slotSocketReadyRead()
{
    recognition_finished = true;
    //
    QByteArray rest = socket.readAll();

    if(QString(rest).contains("REGIST"))
    {
        is_regist = true;
        qDebug() << "注冊模式";
        return;
    }
    else if(QString(rest).contains("RECOGNITION"))
    {
        is_regist = false;
        qDebug() << "识别模式";
    }
    QJsonParseError err;    //用来保存分析错误时的信息
    QJsonDocument doc = QJsonDocument::fromJson(rest,&err);
    if(err.error != QJsonParseError::NoError)
    {

        qDebug() << doc.toJson(QJsonDocument::Indented);
        qDebug() << "Json数据解析错误：" << err.errorString();
        qDebug() << QString(rest);
        return;
    }
    timer_next.start(3000);
    QJsonObject json_obj = doc.object();
    QString user_id = json_obj.value("user_id").toString();
    if(user_id == "-1")
    {
        qDebug() << "认证失败！";
        //显示认证失败的结果
        ui->label_verify_text->setText("认证失败");
        is_verify_success = false;
        //显示认证失败时的图标
        ui->label_verify_icon->setStyleSheet("border-image: url(:../../imgs/no.png);");
        ui->label_verify_icon->show();
        return;
    }
    //识别成功，显示员工信息
    ui->label_id->setText(user_id);
    ui->label_name->setText(json_obj.value("name").toString());
    ui->label_department->setText(json_obj.value("department").toString());
    ui->label_position->setText(json_obj.value("position").toString());
    //显示认证成功的文字
    ui->label_verify_text->setText("认证成功");
    is_verify_success = true;
    //显示认证成功时的图标
    ui->label_verify_icon->setStyleSheet("border-image: url(:../../imgs/ok.png);");
    ui->label_verify_icon->show();
    //显示这个员工的人脸图片
    ui->label_employee_icon->setScaledContents(true);
    ui->label_employee_icon->setPixmap(QPixmap(QString("D:/demo/QtProject/AttendanceServer/user_imgs/%1.jpg").arg(user_id)));

}

void ClientWindow::slotTimerNext()
{
    //认证完成，清空认证结果
    ui->label_id->clear();
    ui->label_name->clear();
    ui->label_department->clear();
    ui->label_position->clear();
    ui->label_employee_icon->clear();
    //隐藏认证结果图标
    ui->label_verify_icon->hide();
    ui->widget_verify_rest->hide();
    //停止定时器
    timer_next.stop();
    is_verify_success = false;
}
