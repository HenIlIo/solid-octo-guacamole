#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QCryptographicHash>

class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

private:
    QTcpServer * mTcpServer;
    QTcpSocket * mTcpSocket;
    //int server_status;
    QSqlDatabase db;
    QString currentSession;
    QString currentRole;

    void initDatabase();

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();
};

#endif // MYTCPSERVER_H
