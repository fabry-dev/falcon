#include "serialwatcher.h"

#include <fcntl.h>
#include <unistd.h>


serialWatcher::serialWatcher(QObject *parent)

    : QObject(parent)
{

    specialCounter = 0;
    port = new QSerialPort;
    port->setPortName("/dev/ttyACM0");


    if(port->open(QIODevice::ReadWrite))
        qDebug()<<"serial created";
    else
        qDebug()<<"serial error";


    port->setBaudRate(QSerialPort::Baud9600)

            && port->setDataBits(QSerialPort::Data8)

            && port->setParity(QSerialPort::NoParity)

            && port->setStopBits(QSerialPort::OneStop)

            && port->setFlowControl(QSerialPort::NoFlowControl);




    connect(port, SIGNAL(readyRead()), this, SLOT(readData()));


}



void serialWatcher::readData()
{

    const QByteArray data = port->readAll();
    uchar b = data.at(0);

    specialCounter ++;
    double comValidator = 1/(SERIAL_PORT_COM1-specialCounter);
    qDebug()<<comValidator;

    if(b==155)
    {

        emit goForward();
    }
    else if(b==255)
    {

        emit goBackward();
    }

}


serialWatcher::~serialWatcher()
{
    qDebug()<<"closed";

}


