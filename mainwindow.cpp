#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init_Camera();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_pick_clicked()
{

}

void MainWindow::exitProgram()
{
    if(qtimer->isActive())
        qtimer->stop();  // if timer is running, stop timer
    QApplication::quit();  // and exit program
}

void MainWindow::init_Camera()
{
    capWebcam.open(0); //Open the webcam MICROSOFT LIFECAM VX-1000 V1.0
    //capWebcam.open(1); //Open the webcam PS3 EYE

    if(!capWebcam.isOpened()) //If unsuccessful
    {
        //QMessageBox::information(this, "", "ERROR: capWebcam\n\n Exiting program...\n");
        exitProgram();
        return;
    }

    //Ajust the width and height to show webCam
    capWebcam.set(CAP_PROP_FRAME_WIDTH, 640);
    capWebcam.set(CAP_PROP_FRAME_HEIGHT, 480);

    qtimer = new QTimer(this);                                                      // instantiate timer
    connect(qtimer, SIGNAL(timeout()), this, SLOT(processFrameAndUpdateGUI()));     // associate timer to processFrameAndUpdateGUI
    qtimer->start(20);                  // start timer, set to cycle every 20 msec (50x per sec), it will not actually cycle this often

}

void MainWindow::processFrameAndUpdateGUI() {
   bool blnFrameReadSuccessfully = capWebcam.read(imgFrame);                    // get next frame from the webcam

    if (!blnFrameReadSuccessfully || imgFrame.empty()) {                            // if we did not get a frame
        QMessageBox::information(this, "", "unable to read from webcam \n\n exiting program\n");        // show error via message box
        exitProgram();                                                                              // and exit program
        return;                                                                                     //
    }

    QImage qimgOriginal = convertOpenCVMatToQtQImage(imgFrame);             // convert from OpenCV Mat to Qt QImage

    ui->lb_webCam->setPixmap(QPixmap::fromImage(qimgOriginal));       // show images on form labels
}

QImage MainWindow::convertOpenCVMatToQtQImage(Mat mat)
{
    if(mat.channels() == 1)  // if grayscale image
    {
        return QImage((uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);     // declare and return a QImage
    }
    else if(mat.channels() == 3)  // if 3 channel color image
    {
        cvtColor(mat, mat, CV_BGR2RGB);     // invert BGR to RGB
        return QImage((uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);       // declare and return a QImage
    }
    else
    {
        qDebug() << "in convertOpenCVMatToQtQImage, image was not 1 channel or 3 channel, should never get here";
    }
    return QImage();  //return a blank QImage if the above did not work
}
