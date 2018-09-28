#ifndef serialWatcher_H
#define serialWatcher_H

#include <QObject>
#include <QFile>
#include <QSocketNotifier>
#include "qdebug.h"
#include <QSerialPort>
class serialWatcher
      : public QObject
{
    Q_OBJECT



public:
    explicit serialWatcher(QObject *parent = nullptr);
    ~serialWatcher();
private:
    QSerialPort *port;

private slots:
    void readData();

signals:
    void goForward();
};

#endif // serialWatcher_H
