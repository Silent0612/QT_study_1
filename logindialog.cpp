#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMovie>
#include <QMessageBox>
#include <QDate>
#include <QFileDialog>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent),db(Database::getInstance())
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    //初始化登录界面
    loginInit();
    //默认显示登录界面
    ui->stackedWidget->setCurrentWidget(ui->page_login);
    //设置随机种子
    srand(time(NULL));
    //给lineEdit_regist_verification_exp控件添加事件过滤器
    ui->lineEdit_regist_verification_exp->installEventFilter(this);
    //给lineEdit_retrieve_verification_exp控件添加事件过滤器
    ui->lineEdit_retrieve_verification_exp->installEventFilter(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::loginInit()
{
    //设置窗口标题
    setWindowTitle("粤嵌考勤系统V1.0");
    //设置窗口图标
    setWindowIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\window.png"));
    //显示logo
    ui->label_logo->setScaledContents(true);
    ui->label_logo->setPixmap(QPixmap("D:\\demo\\QtProject\\AttendanceServer\\imgs\\logo.jpeg"));
    //显示动画
    QMovie *movie = new QMovie("D:\\demo\\QtProject\\AttendanceServer\\imgs\\login.gif");
    ui->label_movie->setScaledContents(true);
    ui->label_movie->setMovie(movie);
    movie->start();
    //显示默认头像
    ui->label_icon->setScaledContents(true);
    ui->label_icon->setPixmap(QPixmap("D:\\demo\\QtProject\\AttendanceServer\\imgs\\icon1_default.png"));

    //显示登录图标
    ui->label_login_user->setScaledContents(true);
    ui->label_login_password->setScaledContents(true);
    ui->label_login_user->setPixmap(QPixmap("D:\\demo\\QtProject\\AttendanceServer\\imgs\\user.jpg"));
    ui->label_login_password->setPixmap(QPixmap("D:\\demo\\QtProject\\AttendanceServer\\imgs\\password.jpg"));

    //显示登录密码显示按钮图标以及密码输入框状态初始化
    ui->lineEdit_passwd->setEchoMode(QLineEdit::Password);
    ui->pushButton_login_displaypasswd->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\displaypassword.png"));

    //显示注册密码显示按钮图标以及密码输入框状态初始化
    ui->lineEdit_regist_passwd->setEchoMode(QLineEdit::Password);
    ui->lineEdit_regist_passwd2->setEchoMode(QLineEdit::Password);
    ui->pushButton_register_displaypd1->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\displaypassword.png"));
    ui->pushButton_register_displaypd2->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\displaypassword.png"));

    //注册界面显示标题
    ui->label_regist_tital->setScaledContents(true);
    ui->label_regist_tital->setPixmap(QPixmap("D:\\demo\\QtProject\\AttendanceServer\\imgs\\register.png"));

    //找回密码界面显示标题
    ui->label_findpasswd_tital->setScaledContents(true);
    ui->label_findpasswd_tital->setPixmap(QPixmap("D:\\demo\\QtProject\\AttendanceServer\\imgs\\findpassword.jpg"));

    //显示当前工作目录
    qDebug() << QDir::currentPath();

    //实例化一个定时器对象
    timer = new QTimer(this);
    //连接信号与槽
    connect(timer,SIGNAL(timeout()),this,SLOT(slotTimerOut()));
    //加载配置信息
    loadSetting();
}

QString LoginDialog::generateRandomExpression()
{
    //生成两个操作数(100以内)，以及一个运算符(+或-)
    return QString("%1%2%3").arg(rand()%100).arg(rand()%2?"+":"-").arg(rand()%100);
}

void LoginDialog::loadSetting()
{
    //实例化一个配置文件对象
    setting = new QSettings("../setting.ini",QSettings::IniFormat,this);
    if(setting == nullptr)
    {
        qDebug() << "setting create error";
    }
    //读取配置
    bool is_remember = setting->value("State/remember").toBool();
    bool is_auto = setting->value("State/auto").toBool();
    //设置界面上自动登录和记住密码的状态
    ui->checkBox_auto_login->setChecked(is_auto);
    ui->checkBox_remember->setChecked(is_remember);
    //记住密码：自动获取账号密码
    if(is_remember)
    {
        ui->lineEdit_id->setText(setting->value("User/id").toString());
        ui->lineEdit_passwd->setText(setting->value("User/passwd").toString());
        //将焦点交给登录按钮
        ui->btn_login->setFocus();
    }
    //自动登录
    if(is_auto)
    {
        //启动定时器，2秒以后再登录
        timer->start(2000);
    }
}

bool LoginDialog::eventFilter(QObject * obj, QEvent * ev)
{
    if(ui->lineEdit_regist_verification_exp == obj && ev->type() == QEvent::MouseButtonPress)
    {
        ui->lineEdit_regist_verification_exp->setText(generateRandomExpression());
    }
    if(ui->lineEdit_retrieve_verification_exp == obj && ev->type() == QEvent::MouseButtonPress)
    {
        ui->lineEdit_retrieve_verification_exp->setText(generateRandomExpression());
    }
    return QWidget::eventFilter(obj,ev);
}

void LoginDialog::on_btn_regist_clicked()
{
    //注册页面显示默认头像
    ui->label_regist_icon->setScaledContents(true);
    ui->label_regist_icon->setPixmap(QPixmap("D:\\demo\\QtProject\\AttendanceServer\\imgs\\icon1_default.png"));
    //清空保存的头像路径
    regist_icon.clear();
    //切换到注册页面
    ui->stackedWidget->setCurrentWidget(ui->page_regist);
    //清空登录界面的密码
    ui->lineEdit_passwd->clear();
    //生成随机表达式
    ui->lineEdit_regist_verification_exp->setText(generateRandomExpression());
}


void LoginDialog::on_btn_retrieve_password_clicked()
{
    //切换到找回密码页面
    ui->stackedWidget->setCurrentWidget(ui->page_retrieve);
    //清空登录界面的密码
    ui->lineEdit_passwd->clear();
    //生成随机表达式
    ui->lineEdit_retrieve_verification_exp->setText(generateRandomExpression());
}


void LoginDialog::on_btn_regist_back_clicked()
{
    //返回登录界面
    ui->stackedWidget->setCurrentWidget(ui->page_login);
    //清空注册页面中输入的个人信息
    ui->lineEdit_regist_id->clear();
    ui->lineEdit_regist_id_card->clear();
    ui->lineEdit_regist_name->clear();
    ui->lineEdit_regist_passwd->clear();
    ui->lineEdit_regist_passwd2->clear();
    ui->lineEdit_regist_tel->clear();
    ui->lineEdit_regist_verification->clear();
}


void LoginDialog::on_btn_retrieve_back_clicked()
{
    //返回登录界面
    ui->stackedWidget->setCurrentWidget(ui->page_login);
    //清空找回密码中输入的个人信息
    ui->lineEdit_retrieve_id->clear();
    ui->lineEdit_retrieve_id_card->clear();
    ui->lineEdit_retrieve_name->clear();
    ui->lineEdit_retrieve_tel->clear();

}


void LoginDialog::on_btn_login_clicked()
{
    //获取用户输入的账号密码
    QString id = ui->lineEdit_id->text();
    QString passwd = ui->lineEdit_passwd->text();
    //判断账号密码是否为空
    if(id.isEmpty() or passwd.isEmpty())
    {
        //用户名和密码不能为空
        QMessageBox::warning(this,"警告","账号和密码不能为空！");
        return;
    }
    //检查账号和密码的位数是否小于6位
    if(id.length() < 6 or passwd.length() < 6)
    {
        QMessageBox::critical(this,"错误","登录失败，账号或密码错误！");
        ui->lineEdit_passwd->clear();
        return;
    }
    //验证账号和密码是否正确
    if(db.userLogin(id,passwd))
    {
        //检查界面上自动登录和记住密码的状态，并更新配置文件中的数据
        bool is_remember = ui->checkBox_remember->isChecked();
        bool is_auto = ui->checkBox_auto_login->isChecked();
        setting->setValue("State/remember",is_remember);
        setting->setValue("State/auto",is_auto);
        if(is_remember)
        {
            //记住密码
            setting->setValue("User/id",id);
            setting->setValue("User/passwd",passwd);
        }
        QMessageBox::information(this,"成功","登录成功，祝您使用愉快！");
        //清空密码
        ui->lineEdit_passwd->clear();
        //关闭登录界面
        close();
        //跳转其他页面
        emit loginSuccess(id);
    }
    else
    {
        QMessageBox::critical(this,"错误","登录失败，账号或密码错误！");
        ui->lineEdit_passwd->clear();
        return;
    }
}


void LoginDialog::on_btn_regist_regist_clicked()
{
    //获取用户输入的数据
    QString id = ui->lineEdit_regist_id->text();
    QString passwd = ui->lineEdit_regist_passwd->text();
    QString passwd2 = ui->lineEdit_regist_passwd2->text();
    QString name = ui->lineEdit_regist_name->text();
    QString id_card = ui->lineEdit_regist_id_card->text();
    QString tel = ui->lineEdit_regist_tel->text();
    QString verification = ui->lineEdit_regist_verification->text();
    //判断数据是否为空
    if(id.isEmpty() or passwd.isEmpty() or passwd2.isEmpty() or name.isEmpty()
        or id_card.isEmpty() or tel.isEmpty() or verification.isEmpty())
    {
        QMessageBox::warning(this,"警告","请输入完整的信息后再试！");
        return;
    }
    //判断数据长度是否正确
    if(id.length() != 12)
    {
        QMessageBox::warning(this,"警告","请输入正确的账号！");
        return;
    }
    if(passwd.length() < 6 or passwd2.length() < 6)
    {
        QMessageBox::warning(this,"警告","请输入正确的密码！");
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
    //判断两次输入的密码是否相同
    if(passwd != passwd2)
    {
        QMessageBox::warning(this,"警告","两次输入的密码不一致，请重新输入！");
        ui->lineEdit_regist_passwd2->clear();
        return;
    }
    //根据用户提供的身份证号码计算出用户的年龄和性别
    //456123200705161234 [6]-[13]：出生年月日  [16]：如果为奇数-男 为偶数-女
    //提取用户的出生年月日
    bool ok;
    int u_year = id_card.mid(6,4).toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"警告","请输入正确的证件号码！");
        return;
    }
    int u_month = id_card.mid(10,2).toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"警告","请输入正确的证件号码！");
        return;
    }
    int u_day = id_card.mid(12,2).toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"警告","请输入正确的证件号码！");
        return;
    }
    //获取当前系统日期的年月日
    QDate c_date = QDate::currentDate();
    int c_year = c_date.year();
    int c_month = c_date.month();
    int c_day = c_date.day();
    //计算年龄
    int age = c_year - u_year;
    //如果用户还没有满周岁，年龄减一
    if(c_month < u_month)
    {
        //还没到满周岁的月份
        age--;
    }
    else if(c_month == u_month and c_day < u_day)
    {
        //在这个月满周岁，但是还没有到
        age--;
    }
    //计算用户的性别
    if(id_card.at(16).digitValue() == -1 or age <= 0 or age > 150)
    {
        //身份证号码倒数第二位不是数字或者年龄计算出来的结果不正常
        QMessageBox::warning(this,"警告","请输入正确的证件号码！");
        return;
    }
    QString gender = id_card.at(16).digitValue()%2 ? "男":"女";
    //计算验证码是否正确
    //获取随机表达式
    QString exp = ui->lineEdit_regist_verification_exp->text();
    //判断是加法运算还是减法运算符
    if(exp.contains("+"))
    {
        //加法
        int opt1 = exp.split("+").at(0).toInt();
        int opt2 = exp.split("+").at(1).toInt();
        //判断用户输入的计算结果是否正确
        if(verification.toInt() != opt1+opt2)
        {
            //验证信息错误
            QMessageBox::warning(this,"错误","验证失败，请输入正确的计算结果！");
            ui->lineEdit_regist_verification->clear();
            //刷新验证码
            ui->lineEdit_regist_verification_exp->setText(generateRandomExpression());
            return;
        }
    }
    else
    {
        //减法
        int opt1 = exp.split("-").at(0).toInt();
        int opt2 = exp.split("-").at(1).toInt();
        //判断用户输入的计算结果是否正确
        if(verification.toInt() != opt1-opt2)
        {
            //验证信息错误
            QMessageBox::warning(this,"错误","验证失败，请输入正确的计算结果！");
            ui->lineEdit_regist_verification->clear();
            //刷新验证码
            ui->lineEdit_regist_verification_exp->setText(generateRandomExpression());
            return;
        }
    }
    //检查用户是否选择了头像
    if(regist_icon.isEmpty())
    {
        //提示用户选择头像
        QMessageBox::warning(this,"警告","您还没有选择头像，请选择完头像后再试！");
        return;
    }
    //注册
    ok = db.userRegist(id,passwd,name,gender,age,QString("%1-%2-%3")
                    .arg(u_year).arg(u_month).arg(u_day),id_card,tel,regist_icon);
    if(!ok)
    {
        QMessageBox::critical(this,"失败","注册失败，请检查信息是否正确！");
        //重新生成验证表达式
        ui->lineEdit_regist_verification_exp->setText(generateRandomExpression());
        ui->lineEdit_regist_verification->clear();
        return;
    }
    //注册成功
    QMessageBox::information(this,"成功","注册成功，请妥善保管账号和密码！");
    //返回登录界面
    on_btn_regist_back_clicked();
    //将用户注册的账号填到登录界面中去
    ui->lineEdit_id->setText(id);
    //将焦点交给密码输入框
    ui->lineEdit_passwd->setFocus();
}


void LoginDialog::on_btn_regist_select_icon_clicked()
{
    QString temp = QFileDialog::getOpenFileName(this,"选择头像","../","*.jpg *.jpeg *.png *.bmp");
    if(!temp.isEmpty())
    {
        //保存用户选择的头像，并显示出来
        regist_icon = temp;
        ui->label_regist_icon->setPixmap(QPixmap(regist_icon));
    }
}


void LoginDialog::on_lineEdit_id_textChanged(const QString &id)
{
    //去数据库中查询用户的头像，如果查询到了就显示出来，没查询到就显示默认头像
    QString icon_path = db.getUserIcon(id);
    if(icon_path.isEmpty())
    {
        //显示默认头像
        icon_path = "D:\\demo\\QtProject\\AttendanceServer\\imgs\\icon1_default.png";
    }
    //显示头像
    ui->label_icon->setPixmap(QPixmap(icon_path));
}

void LoginDialog::slotTimerOut()
{
    //停止定时器
    timer->stop();
    //自动登录，给用户反应时间
    if(ui->checkBox_auto_login->isChecked())
    {
        //如果在等待过程中，用户没有取消自动登录，那么就登录
        on_btn_login_clicked();
    }
}


void LoginDialog::on_btn_retrieve_ok_clicked()
{
    QString id = ui->lineEdit_retrieve_id->text();
    QString name = ui->lineEdit_retrieve_name->text();
    QString id_card = ui->lineEdit_retrieve_id_card->text();
    QString tel = ui->lineEdit_retrieve_tel->text();
    QString verification = ui->lineEdit_retrieve_verification->text();

    if(id.isEmpty() or name.isEmpty() or id_card.isEmpty() or tel.isEmpty() or verification.isEmpty())
    {
        QMessageBox::warning(this,"警告","请输入完整的信息后再试！");
        return;
    }

    //判断数据长度是否正确
    if(id.length() != 12)
    {
        QMessageBox::warning(this,"警告","请输入正确的账号！");
        return;
    }
    if(name.length() <= 0)
    {
        QMessageBox::warning(this,"警告","请输入正确的密码！");
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

    //计算验证码是否正确
    //获取随机表达式
    QString exp = ui->lineEdit_retrieve_verification_exp->text();
    //判断是加法运算还是减法运算符
    if(exp.contains("+"))
    {
        int opt1 = exp.split("+").at(0).toInt();
        int opt2 = exp.split("+").at(1).toInt();
        if(verification.toInt() != opt1+opt2)
        {
            //验证信息错误
            QMessageBox::warning(this,"错误","验证失败，请输入正确的计算结果！");
            ui->lineEdit_regist_verification->clear();
            //刷新验证码
            ui->lineEdit_regist_verification_exp->setText(generateRandomExpression());
            return;
        }
    }
    else if(exp.contains("-"))
    {
        int opt1 = exp.split("-").at(0).toInt();
        int opt2 = exp.split("-").at(1).toInt();
        if(verification.toInt() != opt1-opt2)
        {
            //验证信息错误
            QMessageBox::warning(this,"错误","验证失败，请输入正确的计算结果！");
            ui->lineEdit_regist_verification->clear();
            //刷新验证码
            ui->lineEdit_regist_verification_exp->setText(generateRandomExpression());
            return;
        }
    }

    //查询密码
    QString password = db.retrieve_password(id, name, id_card, tel);
    if(!password.isEmpty())
    {
        //找回成功
        QMessageBox::information(this, "成功", QString("找回密码成功!!!\n密码为:'%1'\n请妥善保管密码！").arg(password));
        //清空个人信息
        ui->lineEdit_retrieve_id->clear();
        ui->lineEdit_retrieve_name->clear();
        ui->lineEdit_retrieve_id_card->clear();
        ui->lineEdit_retrieve_tel->clear();
        ui->lineEdit_retrieve_verification->clear();
        //刷新验证码
        ui->lineEdit_retrieve_verification_exp->setText(generateRandomExpression());
    }
    else
    {
        QMessageBox::critical(this,"失败","找回密码失败，请检查信息是否正确！");
        //清空验证码
        ui->lineEdit_retrieve_verification->clear();
        //刷新验证码
        ui->lineEdit_retrieve_verification_exp->setText(generateRandomExpression());
    }

}


void LoginDialog::on_pushButton_login_displaypasswd_toggled(bool checked)
{
    //松开不显示并显示默认图标
    if(!checked)
    {
        ui->pushButton_login_displaypasswd->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\displaypassword.png"));
        ui->lineEdit_passwd->setEchoMode(QLineEdit::Password);
    }
    //按下显示密码并切换图标
    else
    {
        ui->pushButton_login_displaypasswd->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\hidepassword.png"));
        ui->lineEdit_passwd->setEchoMode(QLineEdit::Normal);
    }
}


void LoginDialog::on_pushButton_register_displaypd1_toggled(bool checked)
{
    //松开不显示并显示默认图标
    if(!checked)
    {
        ui->pushButton_register_displaypd1->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\displaypassword.png"));
        ui->lineEdit_regist_passwd->setEchoMode(QLineEdit::Password);
    }
    //按下显示密码并切换图标
    else
    {
        ui->pushButton_register_displaypd1->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\hidepassword.png"));
        ui->lineEdit_regist_passwd->setEchoMode(QLineEdit::Normal);
    }
}




void LoginDialog::on_pushButton_register_displaypd2_toggled(bool checked)
{
    //松开不显示并显示默认图标
    if(!checked)
    {
        ui->pushButton_register_displaypd2->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\displaypassword.png"));
        ui->lineEdit_regist_passwd2->setEchoMode(QLineEdit::Password);
    }
    //按下显示密码并切换图标
    else
    {
        ui->pushButton_register_displaypd2->setIcon(QIcon("D:\\demo\\QtProject\\AttendanceServer\\imgs\\hidepassword.png"));
        ui->lineEdit_regist_passwd2->setEchoMode(QLineEdit::Normal);
    }
}

