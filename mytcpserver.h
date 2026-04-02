#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include "server_functions.h"

class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    static MyTcpServer* getInstance();
    ~MyTcpServer();

private:
    MyTcpServer(QObject *parent = nullptr);
    MyTcpServer(const MyTcpServer&) = delete;
    MyTcpServer& operator=(const MyTcpServer&) = delete;

    QTcpServer *mTcpServer;
    QMap<long, UserSession> mSessions;

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();
};

#endif // MYTCPSERVER_H
