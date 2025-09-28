QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 添加opencv和SeetaFace的头文件搜索路径
INCLUDEPATH += D:\opencv452\include
INCLUDEPATH += D:\opencv452\include\opencv2
INCLUDEPATH += D:\SeetaFace\include
INCLUDEPATH += D:\SeetaFace\include\seeta
# 添加opencv和SeetaFace库文件搜索路径
LIBS += D:\opencv452\x64\mingw\lib\libopencv*
LIBS += D:\SeetaFace\lib\libSeeta*


SOURCES += \
    main.cpp \
    clientwindow.cpp

HEADERS += \
    clientwindow.h

FORMS += \
    clientwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src.qrc
