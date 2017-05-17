#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#define iLowH_YELLOW    22
#define iHighH_YELLOW   38
#define iLowH_GREEN     38
#define iHighH_GREEN    75
#define iLowH_BLUE      75
#define iHighH_BLUE     130
#define iLowH_RED       160
#define iHighH_RED      179
#define YELLOW          0
#define GREEN           1
#define BLUE            2
#define RED             3

using namespace std;
static int YposX, YposY, GposX, GposY, BposX, BposY, RposX, RposY;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init_Camera();
}

MainWindow::~MainWindow()
{
    //Color coordinates
    cout << "Yx: " << YposX << " Yy: " << YposY << endl
         << "Gx: " << GposX << " Gy: " << GposY << endl
         << "Bx: " << BposX << " By: " << BposY << endl
         << "Rx: " << RposX << " Ry: " << RposY << endl;

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

    static uint8_t color = YELLOW;
    Scalar line_color;
    int iLowH = 0;
    int iHighH = 179;
    int iLowS = 160;
    int iHighS = 250;
    int iLowV = 70;
    int iHighV = 200;

    int iLastX = -1;
    int iLastY = -1;
    int posX = 0, posY = 0;


    if(color == YELLOW){
        iLowH = iLowH_YELLOW;
        iHighH = iHighH_YELLOW;
        iLowS = 160;
        iHighS = 250;
        iLowV = 70;
        iHighV = 200;
        line_color = Scalar(0, 255, 255);
    }
    if(color == GREEN){
        iLowH = iLowH_GREEN;
        iHighH = iHighH_GREEN;
        iLowS = 120;
        iHighS = 221;
        iLowV = 24;
        iHighV = 255;
        line_color = Scalar(0, 255, 0);
    }
    if(color == BLUE){
        iLowH = iLowH_BLUE;
        iHighH = iHighH_BLUE;
        iLowS = 129;
        iHighS = 255;
        iLowV = 23;
        iHighV = 226;
        line_color = Scalar(255, 0, 0);
    }
    if(color == RED){
        iLowH = iLowH_RED;
        iHighH = iHighH_RED;
        iLowS = 195;
        iHighS = 237;
        iLowV = 36;
        iHighV = 181;
        line_color = Scalar(0, 0, 255);
    }

    //Mat imgLines = Mat::zeros( imgFrame.size(), CV_8UC3 );

    vector<vector<Point>> contours;

    Mat imgHSV;

    cvtColor(imgFrame, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

    Mat imgThresholded;

    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image


    //morphological opening (remove small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (fill small holes in the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );


    findContours(imgThresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);



    // Draw black contours on a white image
    drawContours(imgFrame, contours, -1, Scalar(iHighH, iLowS, iLowV), 2);



    //Calculate the moments of the thresholded image
    Moments oMoments = moments(imgThresholded);

    double dM01 = oMoments.m01;
    double dM10 = oMoments.m10;
    double dArea = oMoments.m00;

    // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
    if (dArea > 10000)
    {
        //calculate the position of the ball
        posX = dM10 / dArea;
        posY = dM01 / dArea;


        if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
        {
            //Draw a line from the previous point to the current point
            line(imgFrame, Point(posX, posY), Point(iLastX, iLastY), line_color, 2);
        }

        iLastX = posX;
        iLastY = posY;

        if(color == YELLOW){
            YposX = posX;
            YposY = posY;
        }
        if(color == GREEN){
            GposX = posX;
            GposY = posY;
        }
        if(color == BLUE){
            BposX = posX;
            BposY = posY;
        }
        if(color == RED){
            RposX = posX;
            RposY = posY;
        }
    }

    color++;
    if(color > 3) color = 0;

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
