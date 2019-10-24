#ifndef serialWatcher_H
#define serialWatcher_H

#include <QObject>
#include <QFile>
#include <QSocketNotifier>
#include "qdebug.h"
#include <QSerialPort>

#define SERIAL_PORT_COM1 91

class serialWatcher
      : public QObject
{
    Q_OBJECT



public:
    explicit serialWatcher(QObject *parent = nullptr);
    ~serialWatcher();
private:
    QSerialPort *port;
    int specialCounter;

private slots:
    void readData();

signals:
    void goForward();
    void goBackward();
};

#endif // serialWatcher_H
