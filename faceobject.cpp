#include "faceobject.h"

faceobject::faceobject(QObject *parent)
    : QObject{parent}
{
    //
    init();
}

faceobject::~faceobject()
{
    delete face_engine;
}

void faceobject::init()
{
    //实例化一个引擎对象
    //SeetaModelSetting是一个结构体类型用起来不方便，使用它的派生类：ModelSetting
    seeta::ModelSetting FD_model("D:/SeetaFace/bin/model/fd_2_00.dat");    //人脸检测模型
    seeta::ModelSetting PD_model("D:/SeetaFace/bin/model/pd_2_00_pts5.dat");    //人脸特征模型
    seeta::ModelSetting FR_model("D:/SeetaFace/bin/model/fr_2_10.dat");    //人脸识别模型

    face_engine = new seeta::FaceEngine(FD_model, PD_model, FR_model);
    face_engine->Load("../../face.db");
}

int64_t faceobject::faceRegister(cv::Mat &face_img)
{
    SeetaImageData seeta_img;
    seeta_img.data = face_img.data;
    seeta_img.width = face_img.cols;
    seeta_img.height = face_img.rows;
    seeta_img.channels = face_img.channels();

    int64_t face_id = face_engine->Register(seeta_img);
    if(face_id >= 0)
    {
        face_engine->Save("../../face.db");
    }
    return face_id;
}

int64_t faceobject::faceQuery(cv::Mat &face_img)
{
    //将opencv中的Mat数据转换为seeta中的图像数据
    SeetaImageData seeta_img;
    seeta_img.data = face_img.data; //图像数据
    seeta_img.width = face_img.cols;    //图像宽度
    seeta_img.height = face_img.rows;   //图像高度
    seeta_img.channels = face_img.channels();   //颜色通道数

    float similarity = 0;
    int64_t face_id = face_engine->Query(seeta_img, &similarity);
    if(similarity < 0.8)
    {
        face_id = -1;
    }
    emit sendFaceID(face_id);
    return face_id;
}
