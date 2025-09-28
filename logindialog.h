#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "database.h"
#include <QSettings>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

signals:
    void loginSuccess(QString id);  //自定义信号，用户登录成功

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    void loginInit();    //初始化登录界面
    QString generateRandomExpression(); //用来生成一个随机表达式
    void loadSetting(); //加载配置文件
    virtual bool eventFilter(QObject * obj, QEvent * ev) override;

private slots:
    void on_btn_regist_clicked();

    void on_btn_retrieve_password_clicked();

    void on_btn_regist_back_clicked();

    void on_btn_retrieve_back_clicked();

    void on_btn_login_clicked();

    void on_btn_regist_regist_clicked();

    void on_btn_regist_select_icon_clicked();

    void on_lineEdit_id_textChanged(const QString &arg1);

    void slotTimerOut();    //定时器超时槽函数

    void on_btn_retrieve_ok_clicked();

    void on_pushButton_login_displaypasswd_toggled(bool checked);

    void on_pushButton_register_displaypd1_toggled(bool checked);


    void on_pushButton_register_displaypd2_toggled(bool checked);

private:
    Ui::LoginDialog *ui;
    Database &db;
    QString regist_icon;    //用户注册时选择的头像
    QSettings *setting;     //配置文件对象指针
    QTimer *timer;      //定时器对象指针(用来定时自动登录)
};
#endif // LOGINDIALOG_H
