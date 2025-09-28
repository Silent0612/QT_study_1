#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <opencv.hpp>
#include <QSqlTableModel>
#include "Database.h"
#include "faceobject.h"
#include <QStandardItemModel>


QT_BEGIN_NAMESPACE
namespace Ui {
class serverWindow;
}
QT_END_NAMESPACE

class serverWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void queryFace(cv::Mat&);


public:
    serverWindow(QWidget *parent = nullptr);
    ~serverWindow();

private slots:
    void init(QString);
    void slotNewConnection();   //客户端连接服务器
    void slotClientReadyRead(); //处理客户端发送过来的数据
    void slotSendResult(int64_t face_id);//发送结果给客户端

    void on_btn_open_camera_clicked();

    void on_btn_close_camera_clicked();

    void on_comboBox_regist_id_currentTextChanged(const QString &select_id);




    void on_btn_take_a_picture_clicked();

    void on_btn_regist_ok_clicked();

    void on_btn_manage_update_clicked();

    void on_btn_record_update_clicked();

    void on_tabWidget_currentChanged(int index);

private:
    Ui::serverWindow *ui;
    LoginDialog * login;
    QString id;
    QTcpServer server;
    QTcpSocket * client;
    bool is_regist;
    faceobject face_object;

    QSqlTableModel model_employee;
    QSqlTableModel model_attendance;
    Database &db;

    QByteArray head_data;
    bool is_take_a_pic;
    QStandardItemModel *t_employee_model;  //员工信息表格模型
    QStandardItemModel *t_attendance_model;  //考勤记录表格模型
};
#endif // SERVERWINDOW_H
