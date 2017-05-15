#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

     //connect(serial, SIGNAL(readyRead()), this, SLOT(myReadData()));

    //serial = new QSerialPort(this);
   // bluetooth();
    //serial_idCheck();

    init_Camera();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_serial()
{
    QByteArray serialData;
    bool arduino_is_available = false;
    QString arduino_port_name;

    //  For each available serial port
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
    //  check if the serialport has both a product identifier and a vendor identifier
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
            //  check if the product ID and the vendor ID match those of the arduino uno
            if((serialPortInfo.productIdentifier() == arduino_product_id) && (serialPortInfo.vendorIdentifier() == arduino_vendor_id)){
                   arduino_is_available = true; //    arduino uno is available on this port
                   arduino_port_name = serialPortInfo.portName();
            }
        }
   }

    /*
         *  Open and configure the arduino port if available
         */
   if(arduino_is_available){
        qDebug() << "Found the arduino port...\n";
        serial->setPortName(arduino_port_name);
        //serial->open(QSerialPort::ReadWrite);
        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
   }
   else{
        qDebug() << "Couldn't find the correct port for the arduino.\n";
        QMessageBox::information(this, "Serial Port Error", "Couldn't open serial port to arduino.");
   }

}

void MainWindow::on_pushButton_pick_clicked()
{

}

//void MainWindow::bluetooth()
//{
//    QStringList portSerial;
//    int myPortSerial;
//    portSerial = LoadPorts();
//    uint numPortSerial = portSerial.size();

//    qDebug() << "INDICE" << "\t\tDISPOSITIVO" << endl;
//    if (numPortSerial > 0) {
//        for(uint indice=0; indice<numPortSerial; indice++) {
//            qDebug() << "[" << indice << "]\t\t" << portSerial.value(indice);
//        }
//    }
//    else {
//        qDebug() << "Nenhuma porta serial detectada!\n";
//        exit(1);
//    }
//}

//QStringList MainWindow::LoadPorts()
//{
//    QStringList device;

//    foreach (const QSerialPortInfo &devinfo, QSerialPortInfo::availablePorts()) {

//        QSerialPort devserial;
//        devserial.setPort(devinfo);

//        if (devserial.open(QIODevice::ReadWrite)) {
//            qDebug() << "\nPorta\t:" << devinfo.portName();
//            qDebug() << "Descrição\t:" << devinfo.description();
//            qDebug() << "Fabricante\t:" << devinfo.manufacturer() << "\n";
//            device << devinfo.portName();
//            devserial.close();
//        }
//    }
//    return device;

//}

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

void MainWindow::on_pushButton_start_activity_clicked()
{
    QByteArray serialData;
    init_serial();
    serialData[0] = '!';
    serial->write(serialData);
}



void MainWindow::myReadData(){
   qint64 ret;
   QByteArray serialData;
   char serialBuffer[10];
   int value;
   if(ret = (serial->readLine(serialBuffer, 10))){
             value = atoi(serialBuffer);
    }
   serialData[0] = 1;
   serial->write(serialData);
}

void MainWindow::on_pushButton_stop_activity_clicked()
{
        if(serial->isOpen()){
             serial->close(); //    Close the serial port if it's open.
             qDebug()<<"Port closed";
        }
}

void MainWindow::serial_idCheck(){
    /*
         *  Testing code, prints the description, vendor id, and product id of all ports.
         *  Used it to determine the values for the arduino uno.
         *
         *                                                               */
        qDebug() << "Number of ports: " << QSerialPortInfo::availablePorts().length() << "\n";
        foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
            qDebug() << "Description: " << serialPortInfo.description() << "\n";
            qDebug() << "Has vendor id?: " << serialPortInfo.hasVendorIdentifier() << "\n";
            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier() << "\n";
            qDebug() << "Has product id?: " << serialPortInfo.hasProductIdentifier() << "\n";
            qDebug() << "Product ID: " << serialPortInfo.productIdentifier() << "\n";
        }
}
