#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include<QtCore>
#include<QtCore/QDebug>
#include<QMessageBox>

#include <QtSerialPort/QSerialPort>

#include <QTime>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>


#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public slots:
    void processFrameAndUpdateGUI();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void init_serial();
    void serial_idCheck();
    void on_pushButton_pick_clicked();
    void myReadData();
    void on_pushButton_start_activity_clicked();

    void on_pushButton_stop_activity_clicked();


private:
    Ui::MainWindow *ui;

    QSerialPort *serial;
    static const quint16 arduino_vendor_id = 9025;      //Arduino Mega vendor = 9025   Arduino Nano vendor = 6790
    static const quint16 arduino_product_id = 16;       //Arduino Mega product =16     Arduino Nano product = 29987

    //void bluetooth();
    //QStringList LoadPorts();


    void exitProgram();
    void init_Camera();

    Mat imgFrame;

    VideoCapture capWebcam;   /// Capture object to use with webcam
    QTimer* qtimer;  ///Timer for processFrameAndUpdateGUI()

    QImage convertOpenCVMatToQtQImage(Mat mat);


};

#endif // MAINWINDOW_H
