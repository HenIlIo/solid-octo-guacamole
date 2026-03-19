#include "mytcpserver.h"
#include <QCoreApplication>
#include <QString>
#include <QCryptographicHash>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    // Инициализация базы данных
    initDatabase();

    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started";
        qDebug() << "Default admin: admin/admin123";
    }
}

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
    if (db.isOpen()) {
        db.close();
    }
}

void MyTcpServer::initDatabase()
{
    // создаем базу данных типа QSQLite
    // QSQLite - встроенный драйвер в QT
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("server.db");

    if(!db.open())
        qDebug() << db.lastError().text();

    /*
    создание таблицы
    Название: User
    Поля: login, password, role
    */
    QSqlQuery query;
    //создали таблицу
    query.exec("CREATE TABLE IF NOT EXISTS User("
               "login VARCHAR(20) NOT NULL, "
               "password VARCHAR(128) NOT NULL, "
               "role VARCHAR(20) NOT NULL DEFAULT 'user', "
               "PRIMARY KEY(login))");

    // Создаём администратора по умолчанию
    query.exec("SELECT login FROM User WHERE login='admin'");
    if (!query.next()) {
        // Хэшируем пароль SHA-256
        QByteArray hash = QCryptographicHash::hash("admin123", QCryptographicHash::Sha256);
        query.prepare("INSERT INTO User(login, password, role) VALUES (:login, :password, :role)");
        query.bindValue(":login", "admin");
        query.bindValue(":password", hash.toHex());
        query.bindValue(":role", "admin");
        query.exec();
        qDebug() << "Created default admin";
    }
}

void MyTcpServer::slotNewConnection()
{
    mTcpSocket = mTcpServer->nextPendingConnection();
    mTcpSocket->write("Hello, World!!! I am echo server!\r\n");

    connect(mTcpSocket, &QTcpSocket::readyRead,
            this, &MyTcpServer::slotServerRead);
    connect(mTcpSocket, &QTcpSocket::disconnected,
            this, &MyTcpServer::slotClientDisconnected);
}

void MyTcpServer::slotServerRead()
{
    QString res = "";
    while(mTcpSocket->bytesAvailable()>0)
    {
        QByteArray array = mTcpSocket->readAll();
        qDebug() << array << "\n";
        if(array=="\x01")
        {
            mTcpSocket->write(res.toUtf8());
            res = "";
        }
        else
            res.append(array);
    }

    // Обработка команд (разделитель - пробел)
    QString command = res.trimmed();
    QStringList parts = command.split(" ", Qt::SkipEmptyParts);
    QString action = parts.value(0).toLower();

    // Регистрация: reg login password [role]
    if (action == "reg" && parts.size() >= 3) {
        QString login = parts[1];
        QString password = parts[2];
        QString role = parts.value(3, "user");

        // Проверяем, существует ли пользователь
        QSqlQuery query;
        query.prepare("SELECT login FROM User WHERE login=:login");
        query.bindValue(":login", login);
        query.exec();

        if (query.next()) {
            mTcpSocket->write("error: user exists\r\n");
        } else {
            // Хэшируем пароль
            QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
            query.prepare("INSERT INTO User(login, password, role) VALUES (:login, :password, :role)");
            query.bindValue(":login", login);
            query.bindValue(":password", hash.toHex());
            query.bindValue(":role", role);
            if (query.exec()) {
                mTcpSocket->write("reg+\r\n");
            } else {
                mTcpSocket->write("error: reg failed\r\n");
            }
        }
    }
    // Авторизация: auth login password
    else if (action == "auth" && parts.size() >= 3) {
        QString login = parts[1];
        QString password = parts[2];

        QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
        QSqlQuery query;
        query.prepare("SELECT role FROM User WHERE login=:login AND password=:password");
        query.bindValue(":login", login);
        query.bindValue(":password", hash.toHex());

        if (query.exec() && query.next()) {
            QString role = query.value(0).toString();
            // Сохраняем сессию
            currentSession = login;
            currentRole = role;
            mTcpSocket->write(QString("auth+ %1\r\n").arg(role).toUtf8());
        } else {
            mTcpSocket->write("auth-\r\n");
        }
    }
    // Выход: logout
    else if (action == "logout") {
        if (currentSession.isEmpty()) {
            mTcpSocket->write("error: not logged in\r\n");
        } else {
            currentSession.clear();
            currentRole.clear();
            mTcpSocket->write("logout+\r\n");
        }
    }
    // Информация о себе: whoami
    else if (action == "whoami") {
        if (currentSession.isEmpty()) {
            mTcpSocket->write("whoami: not logged in\r\n");
        } else {
            mTcpSocket->write(QString("whoami: %1 (%2)\r\n")
                           .arg(currentSession, currentRole).toUtf8());
        }
    }
    // Удаление аккаунта: del [login]
    else if (action == "del") {
        if (currentSession.isEmpty()) {
            mTcpSocket->write("error: login required\r\n");
        } else if (parts.size() == 1) {
            // Удаление своего аккаунта
            QSqlQuery query;
            query.prepare("DELETE FROM User WHERE login=:login");
            query.bindValue(":login", currentSession);
            if (query.exec()) {
                currentSession.clear();
                currentRole.clear();
                mTcpSocket->write("del+\r\n");
            } else {
                mTcpSocket->write("error: delete failed\r\n");
            }
        } else if (parts.size() >= 2) {
            // Удаление чужого аккаунта (только admin)
            QString targetLogin = parts[1];
            if (currentRole != "admin") {
                mTcpSocket->write("error: need admin rights\r\n");
            } else {
                QSqlQuery query;
                query.prepare("DELETE FROM User WHERE login=:login");
                query.bindValue(":login", targetLogin);
                if (query.exec()) {
                    mTcpSocket->write(QString("del+ %1\r\n").arg(targetLogin).toUtf8());
                } else {
                    mTcpSocket->write("error: delete failed\r\n");
                }
            }
        }
    }
    // === Смена роли: role targetLogin newRole ===
    else if (action == "role" && parts.size() >= 3) {
        if (currentSession.isEmpty()) {
            mTcpSocket->write("error: login required\r\n");
        } else {
            QString targetLogin = parts[1];
            QString newRole = parts[2];

            // Проверяем права админа
            if (currentRole != "admin") {
                mTcpSocket->write("error: need admin rights\r\n");
            } else if (newRole != "user" && newRole != "moderator" && newRole != "admin") {
                mTcpSocket->write("error: invalid role\r\n");
            } else {
                QSqlQuery query;
                query.prepare("UPDATE User SET role=:role WHERE login=:login");
                query.bindValue(":role", newRole);
                query.bindValue(":login", targetLogin);
                if (query.exec()) {
                    mTcpSocket->write("role+\r\n");
                } else {
                    mTcpSocket->write("error: update failed\r\n");
                }
            }
        }
    }
    // Справка: help
    else if (action == "help") {
        QString help = "Commands:\r\n"
                   "  reg login password [role] - register\r\n"
                   "  auth login password - login\r\n"
                   "  logout - logout\r\n"
                   "  whoami - show current user\r\n"
                   "  del [login] - delete account (admin can delete any)\r\n"
                   "  role login newRole - change role (admin only)\r\n"
                   "Roles: user, moderator, admin\r\n";
        mTcpSocket->write(help.toUtf8());
    }
    // Echo для остальных команд
    else if (!command.isEmpty()) {
        mTcpSocket->write(res.toUtf8());
    }
}

void MyTcpServer::slotClientDisconnected()
{
    currentSession.clear();
    currentRole.clear();
    mTcpSocket->close();
}
