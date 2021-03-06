#include "showrunner.h"

#include <fcntl.h>
#include <unistd.h>
#include <qstringlist.h>
#include <linux/input.h>
#include "qbytearray.h"
#include "qprocess.h"
#include "qtextcodec.h"
#define TIMEOUT 10000

#define FULLSCREEN true
#define SHOW true



QString keycodelist(int scancode){
    QString ret="";
    //return (unsigned char)scancode;
    switch(scancode){
    case 0x02: ret ='1';break;
    case 0x03: ret ='2';break;
    case 0x04: ret ='3';break;
    case 0x05: ret ='4';break;
    case 0x06: ret ='5';break;
    case 0x07: ret ='6';break;
    case 0x08: ret ='7';break;
    case 0x09: ret ='8';break;
    case 0x0a: ret ='9';break;
    case 0x0b: ret ='0';break;
    case 0x0c: ret ='-';break;

    case 0x10: ret ='q';break;
    case 0x11: ret ='w';break;
    case 0x12: ret ='e';break;
    case 0x13: ret ='r';break;
    case 0x14: ret ='t';break;
    case 0x15: ret ='y';break;
    case 0x16: ret ='u';break;
    case 0x17: ret ='i';break;
    case 0x18: ret ='o';break;
    case 0x19: ret ='p';break;

    case 0x1e: ret ='a';break;
    case 0x1f: ret ='s';break;
    case 0x20: ret ='d';break;
    case 0x21: ret ='f';break;
    case 0x22: ret ='g';break;
    case 0x23: ret ='h';break;
    case 0x24: ret ='j';break;
    case 0x25: ret ='k';break;
    case 0x26: ret ='l';break;

    case 0x2c: ret ='z';break;
    case 0x2d: ret ='x';break;
    case 0x2e: ret ='c';break;
    case 0x2f: ret ='v';break;
    case 0x30: ret ='b';break;
    case 0x31: ret ='n';break;
    case 0x32: ret ='m';break;
    default: break;
    }
    return ret.toUpper();
}




showRunner::showRunner(QObject *parent, QList<QWidget *> widgetList, QString PATH, int speed, serialWatcher *serialwatch)
    : QObject(parent),widgetList(widgetList),PATH(PATH),speed(speed),serialwatch(serialwatch)
{

    // videoThread.start();

    codeBuf.clear();

    readCodes();



    auto file = new QFile();
    file->setFileName(fileName);

    if( !file->exists() ){
        qWarning("file does not exist");
        //return;
    }

    qDebug()<<fileName.toUtf8().data();
    fd = open(fileName.toUtf8().data(), O_RDONLY|O_NONBLOCK);
    if( fd==-1 ){
        qWarning("can not open RFID socket");
        //return;
    }

    notifier = new QSocketNotifier( fd,
                                    QSocketNotifier::Read,
                                    this);

    connect( notifier,
             &QSocketNotifier::activated,
             this,
             &showRunner::handle_readNotification );


    RFIDtimeout = new QTimer(this);
    RFIDtimeout->setSingleShot(true);
    connect(RFIDtimeout,SIGNAL(timeout()),this,SLOT(onTimeout()));





    for(int i = 0;i<widgetList.size();i++)
    {
        bgImg.load(PATH+"insert"+QString::number(i)+".png");
        QLabel *lbl = new QLabel(widgetList[i]);
        bgLbls.push_back(lbl);
        lbl->resize(1920,1080);
        lbl->setPixmap(bgImg);
        lbl->hide();
    }



    stopShow();
    //startShow(0);

}





void showRunner::readCodes()
{
    codes.clear();
    QFile file(PATH+"codes.cfg");
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug()<<"no config file";

    }
    else
    {

        QTextStream in(&file);

        QString  line;
        QString paramName,paramValue;
        QStringList params;

        while(!in.atEnd())
        {
            line = in.readLine();
            if (line!="")
                codes.append(line);

        }
        file.close();

    }

    qDebug()<<codes.size()<<" RFID code(s) loaded.";

}





#define imgCount 0

#define videoCount 2

void showRunner::startShow(int show)
{

    codeBuf.clear();


    if(activeShow == show)
    {
        qDebug()<<"reset time out";
        RFIDtimeout->start(TIMEOUT);//just restart
        return;
    }
    else if(activeShow != -1)//already playing something, let us stop first.
    {

        stopShow();
        return;
    }



    activeShow = show;



    QString contentPath = "content"+QString::number(show)+"/";


    for(auto lbl:bgLbls)
    {
        lbl->hide();
    }
    qDebug()<<"start time out count";
    RFIDtimeout->start(TIMEOUT);

    for(auto w:widgetList)
    {
        if(SHOW)
        {
            if(FULLSCREEN)
            {
                w->showFullScreen();
                w->showFullScreen();
                w->showFullScreen();
            }
            else
            {
                w->show();
                w->show();
                w->show();
            }
        }
    }
    std::vector<int> x0s;
    QStringList names;
    QImage buf;
    int totalWidth = 0;


    for(int i =0;i<imgCount;i++)
    {
        QString filename = PATH+contentPath+"img"+QString::number(i)+".jpg";
        buf.load(filename);
        buf = buf.scaledToHeight(1080);
        x0s.push_back(totalWidth);
        qDebug()<<totalWidth;
        totalWidth+= buf.width();
        names.push_back((QString)contentPath+"img"+QString::number(i)+".jpg");
    }



    std::vector<int>videoPos;
    std::vector<int>videoWidth;

    std::vector<QString>videoNames;

    for (int i = 0;i<videoCount;i++)
    {
        videoPos.push_back(totalWidth);
        QString videoName = PATH+contentPath+"video"+QString::number(i)+".mov";
        // videoName = (QString)"/home/fred/Downloads/content0/"+"video"+QString::number(i)+".mov";
        videoNames.push_back(videoName);
        videoWidth.push_back(getVideoWidth(videoName));
        totalWidth+=videoWidth.at(i);

        qDebug()<<videoName<<videoPos.at(i);
    }



    if(totalWidth<widgetList.size()*1920)
        totalWidth = widgetList.size()*1920;

    for(int i =0;i<imgCount;i++)
    {
        slideWindow *sw =   new slideWindow(NULL,PATH,widgetList,x0s.at(i),totalWidth,names.at(i),speed,i);
        connect(serialwatch,SIGNAL(goBackward()),sw,SLOT(goBackward()));
        connect(serialwatch,SIGNAL(goForward()),sw,SLOT(goForward()));


        photos.push_back(sw);

    }


    for (int i = 0;i<videoCount;i++)
    {
        slidevideo *sv1 = new slidevideo(NULL,PATH,widgetList,videoPos.at(i),videoWidth.at(i),totalWidth,videoNames.at(i),speed);
        //sv1->moveToThread(&videoThread);
        videos.push_back(sv1);
        connect(serialwatch,SIGNAL(goForward()),sv1,SLOT(goForward()));
        connect(serialwatch,SIGNAL(goBackward()),sv1,SLOT(goBackward()));
        //
    }





}


void showRunner::stopShow()
{

    if(activeShow == -1)
        return;

    qDebug()<<"stop show";

    activeShow = -1;
    codeBuf.clear();
    for(auto w:widgetList)
    {
        if(SHOW)
        {
            if(FULLSCREEN)
            {
                w->showFullScreen();
                w->showFullScreen();
                w->showFullScreen();
            }
            else
            {
                w->show();
                w->show();
                w->show();
            }
        }
    }

    for(auto lbl:bgLbls)
    {
        lbl->show();
    }

    RFIDtimeout->stop();

    for (auto v:videos)
    {
        v->shutdown();
    }

    for (auto p:photos)
    {
        p->deleteLater();
    }

    photos.clear();
    videos.clear();





}



showRunner::~showRunner()
{
    if( fd>=0 ){
        close(fd);
    }
}


void showRunner::onTimeout()
{
    qDebug()<<"time out RFID";
    stopShow();

}


int showRunner::getVideoWidth(QString name)
{
    QProcess process;
    process.start("mediainfo --Inform=\"Video;%Width%\" "+name);
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();
    QStringList params = stdout.split("\n");
    bool test=false;int val;
    if(params.size()>0)
    {
        val = params[0].toInt(&test);
    }

    if(test)
        return val;
    else
        return 1920;//default value
}





void showRunner::handle_readNotification(int /*socket*/)
{
    // qDebug()<<"notifs";

    struct input_event ev;


    while(read(fd, &ev, sizeof(struct input_event))>0)
    {
        // qDebug()<<ev.code;

        if((ev.type == 1)&&(ev.value==0))
        {
            if((ev.code == 96)||(ev.code == 28))
            {
                //qDebug()<<"enter";

                bool okcode=false;
                for(int i = 0;i<codes.size();i++)
                {
                    if(codeBuf2 == codes[i])
                    {
                        qDebug()<<"code"<<i;

                        codeBuf2 = "";
                        startShow(i);
                        okcode = true;
                        break;
                    }




                }



                if(not okcode)
                {
                    QString sbuf="";
                    /* for(auto b:codeBuf)
                    {

                        sbuf.append(QString::number((int)b)+" ");
                    }
                    qDebug()<<sbuf;*/

                    qDebug()<<"FORCE reset time out";
                    RFIDtimeout->start(TIMEOUT);//just restart


                }

                codeBuf.clear();
                codeBuf2.clear();
            }
            else
            {
                int buf = ev.code;

                // codeBuf.push_back(buf);
                codeBuf2.append(keycodelist(buf));
            }
        }
    }

}













