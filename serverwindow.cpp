#include "serverwindow.h"
#include "ui_serverwindow.h"
#include <QDebug>
#include <QThread>
#include <QSqlRecord>
#include <QMetaType>
#include <QDate>
#include <QMessageBox>

serverWindow::serverWindow(QWidget *parent)
    : QMainWindow(parent),is_regist(false),db(Database::getInstance()),is_take_a_pic(false)
    , ui(new Ui::serverWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("cv::Mat&");
    //实例化登录界面
    login = new LoginDialog();
    login->show();
    //链接信号与槽
    connect(login, SIGNAL(loginSuccess(QString)), this, SLOT(init(QString)));
}

serverWindow::~serverWindow()
{
    delete ui;
    delete login;
}

void serverWindow::init(QString id)
{
    //设置标题
    setWindowTitle("粤嵌考勤系统V1.0");
    //设置图标
    setWindowIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\window.png"));
    //设置id
    this->id = id;
    //显示主窗口
    show();
    connect(&server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    server.listen(QHostAddress::Any, 45678);

    //
    ui->btn_close_camera->setDisabled(true);
    ui->btn_take_a_picture->setDisabled(true);

    //
    QThread * thread = new QThread(this);
    face_object.moveToThread(thread);

    connect(this,SIGNAL(queryFace(cv::Mat&)),&face_object,SLOT(faceQuery(cv::Mat&)),Qt::DirectConnection);

    connect(&face_object, SIGNAL(sendFaceID(int64_t)), this, SLOT(slotSendResult(int64_t)));

    for(int i = 18; i <= 65; i++)
    {
        ui->comboBox_regist_age->addItem(QString::number(i));
        ui->comboBox_manage_age->addItem(QString::number(i));
    }
    ui->comboBox_manage_age->setCurrentText("22");

    ui->comboBox_regist_id->addItems(db.getAllUserID());

    //实例化表格模型
    t_employee_model = new QStandardItemModel;
    t_attendance_model = new QStandardItemModel;
    //设置表头
    t_employee_model->setHorizontalHeaderLabels({"工号","密码","姓名","性别","年龄","生日","证件号码","联系方式","头像路径","入职时间",
                                                 "部门","职位","人脸id"});
    t_attendance_model->setHorizontalHeaderLabels({"编号","工号","姓名","性别","年龄","部门","职位","打卡时间"});
    //启用表格排序功能
    ui->tableView_employees_info->setSortingEnabled(true);
    ui->tableView_attendance_record->setSortingEnabled(true);
    //禁用表格编辑功能
    ui->tableView_employees_info->setEditTriggers(QAbstractItemView::QAbstractItemView::NoEditTriggers);
    ui->tableView_attendance_record->setEditTriggers(QAbstractItemView::QAbstractItemView::NoEditTriggers);
    //界面中的表格绑定模型
    ui->tableView_employees_info->setModel(t_employee_model);
    ui->tableView_attendance_record->setModel(t_attendance_model);
    ui->tabWidget->setCurrentIndex(0);
    on_btn_manage_update_clicked();
}

void serverWindow::slotNewConnection()
{
    client = server.nextPendingConnection();
    connect(client, SIGNAL(readyRead()), this, SLOT(slotClientReadyRead()));
    qDebug() << "Client connected!";
}

void serverWindow::slotClientReadyRead()
{
    QDataStream data_stream(client);
    data_stream.setVersion(QDataStream::Qt_6_9);
    static quint64 data_size = 0;
    if(data_size == 0)
    {
        if(client->bytesAvailable() < (quint64)sizeof(data_size))
        {
            qDebug() << "数据长度不足8字节！";
            return;
        }
        data_stream >> data_size;
    }
    qDebug() << "data_size=" << data_size;
    //接收后续的有效图像数据
    if(client->bytesAvailable() < data_size)
    {
        //客户端的数据还没发完，等客户端数据都发完以后再一次性读取
        qDebug() << "数据还没传完！";
        return;
    }
    QByteArray pic_data;
    data_stream >> pic_data;

    data_size = 0;
    if(pic_data.size() == 0)
    {
        qDebug() << "图片数据为空！";
        return;
    }
    if(is_regist)
    {
        QPixmap pixmap;
        if(!pixmap.loadFromData(pic_data, "jpg"))
        {
            qDebug() << "图片数据加载失败！";
            return;
        }
        pixmap.scaled(ui->label_icon->size());
        ui->label_icon->setPixmap(pixmap);
        head_data = pic_data;
        return;
    }
    std::vector<uchar> decode;
    decode.resize(pic_data.size());
    memcpy(decode.data(), pic_data.data(), pic_data.size());
    cv::Mat face_img = cv::imdecode(decode, cv::IMREAD_COLOR);
    emit queryFace(face_img);
}

void serverWindow::slotSendResult(int64_t face_id)
{
    QString send_data;
    if(-1 == face_id)
    {
        send_data = QString(R"({"user_id":"-1","name":"","department":"","position":""})");
        client->write(send_data.toUtf8());
        return;
    }
    QStringList u_info = db.getUserInfoByFaceID(face_id);
    if(!u_info.isEmpty())
    {
        //查询到了记录，获取这一条记录的所有信息
        //按照Json数据格式打包数据，发送给客户端
        send_data = QString("{\"user_id\":\"%1\",\"name\":\"%2\",\"department\":\"%3\",\"position\":\"%4\"}")
                        .arg(u_info[0]).arg(u_info[1]).arg(u_info[2]).arg(u_info[3]);
        qDebug() << u_info[0] << u_info[1] << u_info[2] << u_info[3];
        //发送
        client->write(send_data.toUtf8());
        //将考勤记录保存到数据库中去
        db.addAttendanceRecord(u_info[0]);

    }
    else
    {
        //没查询到记录
        //准备空结果按照Json格式使用多行字符串发送给客户端
        send_data = QString(R"({"user_id":"-1","name":"","department":"","position":""})");
        //发送
        client->write(send_data.toUtf8());
    }
}

void serverWindow::on_btn_open_camera_clicked()
{
    if(client->isWritable())
    {
        client->write("REGIST");
        is_regist = true;
        ui->btn_open_camera->setEnabled(false);
        ui->btn_close_camera->setEnabled(true);
        ui->btn_take_a_picture->setEnabled(true);
    }
}


void serverWindow::on_btn_close_camera_clicked()
{
    if(client->isWritable())
    {
        //发送数据给客户端：当前模式为识别模式，发送灰度图数据
        client->write("RECOGNITION");
        is_regist = false;
        ui->btn_open_camera->setEnabled(true);
        ui->btn_close_camera->setEnabled(false);
        ui->btn_take_a_picture->setEnabled(false);
    }
}


void serverWindow::on_comboBox_regist_id_currentTextChanged(const QString &select_id)
{
    //从数据库中查询出该用户的信息，并显示到界面上去，方便编辑
    USER_INFO u_info = db.getUserAllInfo(select_id);
    ui->lineEdit_regist_passwd->setText(u_info.passwd);
    ui->lineEdit_regist_name->setText(u_info.name);
    ui->lineEdit_regist_id_card->setText(u_info.id_card);
    ui->comboBox_regist_gender->setCurrentText(u_info.gender);
    ui->comboBox_regist_age->setCurrentText(QString::number(u_info.age));
    ui->lineEdit_regist_tel->setText(u_info.tel);
    QStringList date_list = u_info.birthday.split("-");
    ui->dateEdit_regist_birthday->setDate(QDate(date_list.at(0).toInt(),date_list.at(1).toInt(),date_list.at(2).toInt()));
    if(u_info.join_date.isEmpty())
    {
        ui->dateEdit_regist_entry_time->setDate(QDate::currentDate());
    }
    else
    {
        date_list = u_info.join_date.split("-");
        ui->dateEdit_regist_entry_time->setDate(QDate(date_list.at(0).toInt(),date_list.at(1).toInt(),date_list.at(2).toInt()));
    }
    ui->lineEdit_regist_department->setText(u_info.department);
    ui->lineEdit_regist_position->setText(u_info.position);
    ui->lineEdit_regist_salary->setText(QString::number(u_info.salary));
}


void serverWindow::on_btn_take_a_picture_clicked()
{
    //拍照，关闭摄像头，停止画面
    on_btn_close_camera_clicked();
    is_take_a_pic = true;
}


void serverWindow::on_btn_regist_ok_clicked()
{
    //点击确定，保存数据
    //获取员工信息
    QString u_id = ui->comboBox_regist_id->currentText();
    QString passwd = ui->lineEdit_regist_passwd->text();
    QString name = ui->lineEdit_regist_name->text();
    QString id_card = ui->lineEdit_regist_id_card->text();
    QString gender = ui->comboBox_regist_gender->currentText();
    int age = ui->comboBox_regist_age->currentText().toInt();
    QString tel = ui->lineEdit_regist_tel->text();
    QString birthday = ui->dateEdit_regist_birthday->date().toString("yyyy-MM-dd");
    QString join_date = ui->dateEdit_regist_entry_time->date().toString("yyyy-MM-dd");
    QString department = ui->lineEdit_regist_department->text();
    QString position = ui->lineEdit_regist_position->text();
    bool ok;
    float salary = ui->lineEdit_regist_salary->text().toFloat(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"警告","请输入正确的工资！");
        return;
    }
    if(passwd.isEmpty() or name.isEmpty() or id_card.isEmpty() or tel.isEmpty() or department.isEmpty() or position.isEmpty())
    {
        QMessageBox::warning(this,"警告","请输入完整的信息后再试！");
        return;
    }
    if(passwd.length() < 6)
    {
        QMessageBox::warning(this,"警告","请输入6位以上的密码！");
        return;
    }
    if(id_card.length() != 18)
    {
        QMessageBox::warning(this,"警告","请输入正确的证件号码！");
        return;
    }
    if(tel.length() != 11)
    {
        QMessageBox::warning(this,"警告","请输入正确的手机号码！");
        return;
    }
    // 检测用户是否录入过人脸
    int64_t faceId = db.isRegisted(u_id);
    if(faceId == -1)
    {
        // 查询错误或用户不存在，做相应处理
        QMessageBox::warning(this, "错误", "用户信息查询失败或用户不存在");
        return;
    }
    if(faceId == -2)
    {
        //未录入人脸（-2表示NULL）
        if(!is_take_a_pic)
        {
            QMessageBox::warning(this, "警告", "新员工必须录入人脸数据，请先拍照！");
            return;
        }
    }
    int64_t face_id = -1;
    if(is_take_a_pic)
    {
        std::vector<uchar> decode;
        decode.resize(head_data.size());
        memcpy(decode.data(), head_data.data(), head_data.size());
        cv::Mat face_img = cv::imdecode(decode, cv::IMREAD_COLOR);
        face_id = face_object.faceRegister(face_img);
        if(-1 == face_id)
        {
            QMessageBox::warning(this,"警告","录入人脸数据失败，请重新拍照！");
            is_take_a_pic = false;
            return;
        }
        //人脸注册成功，保存用户的照片
        cv::imwrite(QString("../../user_imgs/%1.jpg").arg(id).toUtf8().data(),face_img);
    }
    if(!db.updateUserAllInfo(USER_INFO(u_id,passwd,name,gender,age,birthday,id_card,tel,QString("../../user_imgs/%1.jpg").arg(id),
                                        join_date,department,position,salary,face_id)))
    {
        //更新员工信息失败
        QMessageBox::warning(this,"警告","员工信息更新失败，请检查数据是否正确，或联系管理员！");
        return;
    }

    //员工注册成功
    QMessageBox::information(this,"成功","员工信息录入成功！");
}


void serverWindow::on_btn_manage_update_clicked()
{
    t_employee_model->removeRows(0, t_employee_model->rowCount());
    QList<USER_INFO> info_list = db.getAllUserInfo();
    //
    for(int i = 0; i< info_list.length(); i++)
    {
        QList<QStandardItem*> row_data;
        row_data << new QStandardItem(info_list.at(i).id)
                 << new QStandardItem(info_list.at(i).passwd)
                 << new QStandardItem(info_list.at(i).name)
                 << new QStandardItem(info_list.at(i).gender)
                 << new QStandardItem(QString::number(info_list.at(i).age))
                 << new QStandardItem(info_list.at(i).birthday)
                 << new QStandardItem(info_list.at(i).id_card)
                 << new QStandardItem(info_list.at(i).tel)
                 << new QStandardItem(info_list.at(i).icon_path)
                 << new QStandardItem(info_list.at(i).join_date)
                 << new QStandardItem(info_list.at(i).department)
                 << new QStandardItem(info_list.at(i).position)
                 << new QStandardItem(QString::number(info_list.at(i).face_id));
        t_employee_model->appendRow(row_data);
    }
}


void serverWindow::on_btn_record_update_clicked()
{
    //获取所有考勤记录信息并显示到界面上去
    //先清空所有数据
    t_attendance_model->removeRows(0,t_attendance_model->rowCount());
    //从数据库中查询所有记录
    QList<ATTENDANCE_INFO> info_list = db.getAllAttendanceInfo();
    //添加数据到界面上去
    for(int i = 0; i < info_list.length(); i++)
    {
        QList<QStandardItem*> row_data;
        row_data << new QStandardItem(info_list.at(i).id)
                 << new QStandardItem(info_list.at(i).employee_id)
                 << new QStandardItem(info_list.at(i).name)
                 << new QStandardItem(info_list.at(i).gender)
                 << new QStandardItem(QString::number(info_list.at(i).age))
                 << new QStandardItem(info_list.at(i).department)
                 << new QStandardItem(info_list.at(i).position)
                 << new QStandardItem(info_list.at(i).attendance_time);
        t_attendance_model->appendRow(row_data);
    }
}


void serverWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 0)
    {
        //刷新员工信息
        on_btn_manage_update_clicked();
    }
    else if(index == 2)
    {
        //刷新考勤记录信息
        on_btn_record_update_clicked();
    }
}

