#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

//员工信息结构体
// 员工工号	密码	姓名	性别	年龄	出生年月日	证件号码	联系方式	头像路径	入职时间	部门	职位	工资	人脸id
// id	passwd	name	gender	age	birthday	id_card	tel	icon_path	join_date	department	position	salary	face_id
struct USER_INFO
{
    USER_INFO(){}
    USER_INFO(QString id,QString passwd,QString name,QString gender,int age,QString birthday,
              QString id_card,QString tel,QString icon_path,QString join_date,QString department,
              QString position,float salary,int face_id):id(id),passwd(passwd),name(name),gender(gender),
        age(age),birthday(birthday),id_card(id_card),tel(tel),icon_path(icon_path),join_date(join_date),
        department(department),position(position),salary(salary),face_id(face_id){}
    QString id;
    QString passwd;
    QString name;
    QString gender;
    int age;
    QString birthday;
    QString id_card;
    QString tel;
    QString icon_path;
    QString join_date;
    QString department;
    QString position;
    float salary;
    int face_id;
};

//考勤信息结构体
struct ATTENDANCE_INFO
{
    ATTENDANCE_INFO(){}
    ATTENDANCE_INFO(QString id,QString employee_id,QString name,QString gender,int age,QString department,
                    QString position,QString attendance_time):id(id),employee_id(employee_id),name(name),
        gender(gender),age(age),department(department),position(position),attendance_time(attendance_time){}
    QString id;
    QString employee_id;
    QString name;
    QString gender;
    int age;
    QString department;
    QString position;
    QString attendance_time;
};


class Database : public QObject
{
    Q_OBJECT
public:
    //析构函数
    ~Database();
    //提供一个获取唯一实例的方法
    static Database & getInstance();

    //用户注册
    //员工工号 密码 姓名 性别 年龄 出生年月日 证件号码 联系方式 头像路径
    //id passwd name gender age birthday id_card tel icon_path
    bool userRegist(QString id,QString passwd,QString name,QString gender,int age,QString birthday,
                    QString id_card,QString tel,QString icon_path);

    //查询用户的头像
    QString getUserIcon(QString id);

    //用户登录
    bool userLogin(QString id,QString passwd);

    //用户找回密码
    QString retrieve_password(QString id, QString name, QString id_card, QString tel);

    //获取所有的员工id
    QStringList getAllUserID();

    //获取某个员工的所有信息
    USER_INFO getUserAllInfo(QString id);

    //根据人脸id查询用户数据
    QStringList getUserInfoByFaceID(int64_t face_id);

    //判断员工是否注册过人脸
    int64_t isRegisted(QString id);

    //更新用户信息
    bool updateUserAllInfo(USER_INFO u_info);

    //添加一条考勤记录
    bool addAttendanceRecord(QString employee_id);

    //获取所有用户信息
    QList<USER_INFO> getAllUserInfo();

    //获取所有考勤信息
    QList<ATTENDANCE_INFO> getAllAttendanceInfo();



private:
    //初始化数据库
    bool init();


private:
    explicit Database(QObject *parent = nullptr);

    Database(const Database &) = delete;

    Database & operator=(const Database &) = delete;

private:
    QSqlDatabase * m_db;
};

#endif // DATABASE_H
