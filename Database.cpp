#include "Database.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QDateTime>

Database::Database(QObject *parent)
    : QObject{parent}
{
    if(!init())
    {
        exit(-1);
    }
}

//析构函数
Database::~Database()
{
    m_db->close();
    delete m_db;
}

Database &Database::getInstance()
{
    static Database db;
    return db;
}

bool Database::userRegist(QString id, QString passwd, QString name, QString gender, int age, QString birthday, QString id_card, QString tel, QString icon_path)
{
    QString sql_str = QString("insert into t_employee(id,passwd,name,gender,"
                              "age,birthday,id_card,tel,icon_path) values("
                              "'%1','%2','%3','%4',%5,'%6','%7','%8','%9');").arg(id).arg(passwd)
                          .arg(name).arg(gender).arg(age).arg(birthday)
                          .arg(id_card).arg(tel).arg(icon_path);
    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "insert into t_employee error:"+query.lastError().text();
        return false;
    }
    return true;
}

QString Database::getUserIcon(QString id)
{
    QString sql_str = QString("select icon_path from t_employee where id='%1';").arg(id);
    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "select icon_path from t_employee error:"+query.lastError().text();
        return "";
    }
    //判断是否有查询到结果
    if(query.next())
    {
        //查询到了结果，返回数据
        return query.value(0).toString();
    }
    return "";
}

bool Database::userLogin(QString id, QString passwd)
{
    QString sql_str = QString("select * from t_employee where id='%1' and passwd='%2';").arg(id).arg(passwd);
    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "user login error:"+query.lastError().text();
    }

    if(query.next())
    {
        return true;
    }
    return false;
}

QString Database::retrieve_password(QString id, QString name, QString id_card, QString tel)
{
    QString sql_str = QString("select passwd from t_employee where id='%1' and name='%2' and id_card='%3' and tel='%4';").arg(id).arg(name).arg(id_card).arg(tel);
    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "retrieve_password error:"+query.lastError().text();
    }
    if(query.next())
    {
        return query.value(0).toString();
    }
    return "";
}

QStringList Database::getAllUserID()
{
    QString sql_str = "select id from t_employee;";
    QSqlQuery query(*m_db);
    QStringList id_list;
    if(!query.exec(sql_str))
    {
        qDebug() << "select all user_id error:"+query.lastError().text();
        return id_list;
    }
    while(query.next())
    {
        id_list << query.value(0).toString();
    }
    return id_list;
}

USER_INFO Database::getUserAllInfo(QString id)
{
    QString sql_str = "select * from t_employee;";
    QSqlQuery query(*m_db);
    USER_INFO u_info;
    if(!query.exec(sql_str))
    {
        qDebug() << "select * from t_employee error:"+query.lastError().text();
        return u_info;
    }
    if(query.next())
    {
        u_info.id = query.value(0).toString();
        u_info.passwd = query.value(1).toString();
        u_info.name = query.value(2).toString();
        u_info.gender = query.value(3).toString();
        u_info.age = query.value(4).toInt();
        u_info.birthday = query.value(5).toString();
        u_info.id_card = query.value(6).toString();
        u_info.tel = query.value(7).toString();
        u_info.icon_path = query.value(8).toString();
        u_info.join_date = query.value(9).toString();
        u_info.department = query.value(10).toString();
        u_info.position = query.value(11).toString();
        u_info.salary = query.value(12).toFloat();
        u_info.face_id = query.value(13).toInt();
    }
    return u_info;
}

QStringList Database::getUserInfoByFaceID(int64_t face_id)
{
    QString sql_str = QString("select id,name,department,position from t_employee where face_id=%1;").arg(face_id);
    QSqlQuery query(*m_db);
    QStringList info_list;
    if(!query.exec(sql_str))
    {
        qDebug() << "select user_info by face_id from t_employee error:"+query.lastError().text();
        return info_list;
    }
    if(query.next())
    {
        for(int i = 0; i < 4; i++)
        {
            info_list << query.value(i).toString();
        }
    }
    return info_list;
}

int64_t Database::isRegisted(QString id)
{
    QString sql_str = QString("select face_id from t_employee where id='%1';").arg(id);
    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "select face_id from t_employee error:"+query.lastError().text();
        return -1;
    }
    if(query.next())
    {
        // 检查字段是否为 NULL
        if (query.value(0).isNull())
        {
            //用特定值表示 NULL
            return -2;
        }
        else
        {
            return query.value(0).toInt();
        }
    }
    return -1;
}

bool Database::updateUserAllInfo(USER_INFO u_info)
{
    //判断是否要更新face_id
    QString sql_str;
    if(u_info.face_id == -1)
    {
        //不需要更新face_id
        // id	passwd	name	gender	age	birthday	id_card	tel	icon_path	join_date	department	position	salary	face_id
        sql_str = QString("update t_employee set passwd='%2',name='%3',gender='%4',age=%5,birthday='%6',"
                          "id_card='%7',tel='%8',icon_path='%9',join_date='%10',department='%11',position='%12',"
                          "salary=%13 where id='%1';").arg(u_info.id).arg(u_info.passwd)
                      .arg(u_info.name).arg(u_info.gender).arg(u_info.age).arg(u_info.birthday).arg(u_info.id_card).arg(u_info.tel)
                      .arg(u_info.icon_path).arg(u_info.join_date).arg(u_info.department).arg(u_info.position).
                  arg(u_info.salary);
    }
    else
    {
        sql_str = QString("update t_employee set passwd='%2',name='%3',gender='%4',age=%5,birthday='%6',"
                          "id_card='%7',tel='%8',icon_path='%9',join_date='%10',department='%11',position='%12',"
                          "salary=%13,face_id=%14 where id='%1';").arg(u_info.id).arg(u_info.passwd)
                      .arg(u_info.name).arg(u_info.gender).arg(u_info.age).arg(u_info.birthday).arg(u_info.id_card).arg(u_info.tel)
                      .arg(u_info.icon_path).arg(u_info.join_date).arg(u_info.department).arg(u_info.position).
                  arg(u_info.salary).arg(u_info.face_id);
    }
    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "update user all info error:"+query.lastError().text();
        return false;
    }
    return true;
}

bool Database::addAttendanceRecord(QString employee_id)
{
    QString sql_str = QString("insert into t_attendance(employee_id, attendance_time) value('%1','%2');").arg(employee_id).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "insert into t_attendance error:"+query.lastError().text();
        return false;
    }
    return true;
}

QList<USER_INFO> Database::getAllUserInfo()
{
    QString sql_str = "select * from t_employee;";
    QSqlQuery query(*m_db);
    QList<USER_INFO> list_user_info;
    if(!query.exec(sql_str))
    {
        qDebug() << "select All user_info error:"+query.lastError().text();
        return list_user_info;
    }
    while(query.next())
    {
        //将用户的所有信息记录到结构体对象中去
        USER_INFO u_info;
        u_info.id = query.value(0).toString();
        u_info.passwd = query.value(1).toString();
        u_info.name = query.value(2).toString();
        u_info.gender = query.value(3).toString();
        u_info.age = query.value(4).toInt();
        u_info.birthday = query.value(5).toString();
        u_info.id_card = query.value(6).toString();
        u_info.tel = query.value(7).toString();
        u_info.icon_path = query.value(8).toString();
        u_info.join_date = query.value(9).toString();
        u_info.department = query.value(10).toString();
        u_info.position = query.value(11).toString();
        u_info.salary = query.value(12).toFloat();
        u_info.face_id = query.value(13).toInt();
        list_user_info << u_info;
    }
    return list_user_info;

}

QList<ATTENDANCE_INFO> Database::getAllAttendanceInfo()
{
    QString sql_str = "select a.number,e.id,e.name,e.gender,e.age,e.department,e.position,a.attendance_time from t_employee as e, t_attendance as a where e.id = employee_id;";
    QSqlQuery query(*m_db);
    QList<ATTENDANCE_INFO> list_attendance_info;
    if(!query.exec(sql_str))
    {
        qDebug() << "select All attendance_info error:"+query.lastError().text();
        return list_attendance_info;
    }
    while(query.next())
    {
        //将所有考勤记录信息添加到结构体对象中去
        list_attendance_info << ATTENDANCE_INFO(query.value(0).toString(),query.value(1).toString(),query.value(2).toString(),
                                                query.value(3).toString(),query.value(4).toInt(),query.value(5).toString(),
                                                query.value(6).toString(),query.value(7).toString());
    }
    return list_attendance_info;
}

bool Database::init()
{
    //实例化一个数据库链接对象
    m_db = new QSqlDatabase;
    //设置数据库类型
    *m_db = QSqlDatabase::addDatabase("QSQLITE");
    //设置数据库文件的路径名
    m_db->setDatabaseName("../../database.db");
    //打开数据库
    if(!m_db->open())
    {
        qDebug() << "open database error:"+m_db->lastError().text();
        return false;
    }
    //创建关系表
    // 员工工号	密码	姓名	性别	年龄	出生年月日	证件号码	联系方式	头像路径	入职时间	部门	职位	工资	人脸特征
    // id	passwd	name	gender	age	birthday	id_card	tel	icon_path	join_date	department	position	salary	face_id
    QString sql_str =   "create table if not exists t_employee("
                        "id char(12) primary key not null,"
                        "passwd varchar(12) not null,"
                        "name varchar(60) not null,"
                        "gender char(3) not null,"
                        "age int(2) not null,"
                        "birthday date not null,"
                        "id_card char(18) not null unique,"
                        "tel char(11) not null unique,"
                        "icon_path varchar(128) not null,"
                        "join_date date,"
                        "department varchar(60),"
                        "position varchar(60),"
                        "salary float,"
                        "face_id int unique"
                        ");";

    QSqlQuery query(*m_db);
    if(!query.exec(sql_str))
    {
        qDebug() << "create table t_employee error:" + query.lastError().text();
        return false;
    }

    //序号	员工工号	打卡时间
    //number	employee_id	attendance_time
    sql_str = "create table if not exists t_attendance("
              "number integer primary key autoincrement,"
              "employee_id char(12) not null,"
              "attendance_time datetime not null"
              ");";
    if(!query.exec(sql_str))
    {
        qDebug() << "create table t_attendance error:" + query.lastError().text();
        return false;
    }
    qDebug() << "database init success!";
    return true;

}



