#include <QApplication>
#include "qwindow.h"
#include "qscreen.h"
#include "qwidget.h"
#include "slidewindow.h"
#include "qdebug.h"
#include "videoplayer.h"
#include "slidevideo.h"
#include "showrunner.h"
#include "serialwatcher.h"
#include "videoplayer.h"
#define PATH_DEFAULT (QString)"/home/fred/Dropbox/Taf/Cassiopee/falcon/files/"

#define imgCount 5
#define defaultSpeed (55)

int main(int argc, char *argv[])

{
    QApplication a(argc, argv);

    QCursor cursor(Qt::BlankCursor);
    a.setOverrideCursor(cursor);
    a.changeOverrideCursor(cursor);



    QString PATH;
    QStringList params = a.arguments();
    if(params.size()>1)
        PATH = params[1];
    else
        PATH=PATH_DEFAULT;


    int speed = defaultSpeed;


    QFile file(PATH+"config.cfg");
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug()<<"no config file";

    }
    else
    {

        QTextStream in(&file);

        QString  line;
        QString paramName,paramValue;
        QStringList params;

        while(!in.atEnd()) {
            line = in.readLine();
            line = (line.split("#"))[0];
            params = line.split("=");
            if(params.size()>=2)
            {
                paramName = params[0];
                paramValue = params[1];

                if (paramName=="SPEED")
                {
                    bool test;
                    speed = paramValue.toInt(&test);
                    if(!test)
                        speed = defaultSpeed;
                }

                else
                    qDebug()<<paramName<<" - "<<paramValue;
            }
        }
        file.close();

    }







    serialWatcher * serialwatch = new serialWatcher(NULL);



    QWidget w1;
    QWidget w2;
    QWidget w0;


    w1.setAttribute(Qt::WA_DeleteOnClose);
    w0.setAttribute(Qt::WA_DeleteOnClose);
    w2.setAttribute(Qt::WA_DeleteOnClose);

    QList<QWidget*> ws;


    ws.push_back(&w0);
    ws.push_back(&w1);
    ws.push_back(&w2);


    qDebug()<<a.screens().size()<<" screens detected";
    std::vector<QScreen*> screens;

    for(int i = 0;i<a.screens().size();i++)
    {

        screens.push_back(a.screens().at(i));



        ws[i]->setGeometry(screens[i]->geometry().x(),screens[i]->geometry().y(),1920,1080);
        //w1.setGeometry(screen1->geometry().x(),screen1->geometry().y(),1920,1080);
        //w2.setGeometry(screen2->geometry().x(),screen2->geometry().y(),1920,1080);


    }

    new showRunner(NULL,ws,PATH,speed,serialwatch);

    return a.exec();
}
