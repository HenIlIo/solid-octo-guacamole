#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QString>
#include "server_functions.h"

MyTcpServer::~MyTcpServer()
{

    mTcpServer->close();
    //server_status=0;
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent){
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        //server_status=1;
        qDebug() << "server is started";
    }
}

void MyTcpServer::slotNewConnection(){
 //   if(server_status==1){
        mTcpSocket = mTcpServer->nextPendingConnection();
        mTcpSocket->write("Hello, World!!! I am echo server!\r\n");
        connect(mTcpSocket, &QTcpSocket::readyRead,this,&MyTcpServer::slotServerRead);
        connect(mTcpSocket,&QTcpSocket::disconnected,this,&MyTcpServer::slotClientDisconnected);
   // }
}

void MyTcpServer::slotServerRead(){
    // Читаем всё, что пришло от клиента
    while(mTcpSocket->bytesAvailable() > 0)
    {
        QByteArray array = mTcpSocket->readAll();
        qDebug() << "Raw data:" << array;

        // Преобразуем в строку и убираем переносы \r\n (trimmed)
        QString request = QString::fromUtf8(array).trimmed();

        // Если прислали пустую строку (просто нажали Enter), игнорируем
        if (request.isEmpty()) continue;

        // Отдаем запрос нашему парсеру
        QString response = parse(request);

        // Отправляем ответ парсера обратно клиенту
        mTcpSocket->write(response.toUtf8() + "\r\n");
    }
}

void MyTcpServer::slotClientDisconnected(){
    mTcpSocket->close();
}
