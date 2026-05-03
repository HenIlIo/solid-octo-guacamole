#include "mytcpserver.h"
#include "server_functions.h"
#include <QDebug>

MyTcpServer* MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return &instance;
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started";
    }
}

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();
    clientSocket->write("Hello, World!!! I am echo server!\r\n");

    // currentTaskType = 0 (нет активного задания),
    // currentTaskNum = 0, currentCorrectAnswer = 0
    mSessions[clientSocket->socketDescriptor()] = UserSession{
        "", "", ROLE_USER, nullptr,
        0,  // currentTaskType — нет активного задания
        0,  // currentTaskNum
        0   // currentCorrectAnswer
    };

    connect(clientSocket, &QTcpSocket::readyRead,
            this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected,
            this, &MyTcpServer::slotClientDisconnected);
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QString res = "";
    while(clientSocket->bytesAvailable() > 0)
    {
        QByteArray array = clientSocket->readAll();
        qDebug() << array << "\n";
        if(array == "\x01")
        {
            clientSocket->write(res.toUtf8());
            res = "";
        }
        else
            res.append(array);
    }

    QString command = res.trimmed();
    if (!command.isEmpty()) {
        UserSession &session = mSessions[clientSocket->socketDescriptor()];
        QString response = processCommand(command, session);
        clientSocket->write(response.toUtf8());
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    long idsock = clientSocket->socketDescriptor();
    //if (clientSocket) {
        mSessions.remove(idsock);
        clientSocket->close();
        clientSocket->deleteLater();
    //}
}
