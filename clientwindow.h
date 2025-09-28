#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <opencv.hpp>
#include <QTcpSocket>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class ClientWindow;
}
QT_END_NAMESPACE

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private:
    bool init();
    //定时器处理函数
    void timerEvent(QTimerEvent * e);

private slots:
    void slotConnectToServer();
    void slotDisconnected();
    void slotConnected();
    void slotSocketReadyRead();
    void slotTimerNext();




private:
    Ui::ClientWindow *ui;
    cv::VideoCapture capture;
    cv::CascadeClassifier cascade;
    QTcpSocket socket;
    QTimer timer_connect_to_server;
    bool recognition_finished;
    bool is_regist;
    QTimer timer_next;
    bool is_verify_success;
};
#endif // CLIENTWINDOW_H
