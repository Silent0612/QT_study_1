#ifndef FACEOBJECT_H
#define FACEOBJECT_H

#include <QObject>
#include <opencv.hpp>
#include <seeta/FaceEngine.h>

class faceobject : public QObject
{
    Q_OBJECT
public:
    explicit faceobject(QObject *parent = nullptr);
    ~faceobject();
    void init();


public slots:
    //注册人脸
    int64_t faceRegister(cv::Mat& face_img);
    //查询人脸
    int64_t faceQuery(cv::Mat& face_img);

signals:
    int64_t sendFaceID(int64_t);

private:
    seeta::FaceEngine * face_engine;
};

#endif // FACEOBJECT_H
