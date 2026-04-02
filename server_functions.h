#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <QString>
#include <QStringList>
#include <QTcpSocket>
#include "databasemanager.h"

// Структура сессии пользователя
struct UserSession {
    QString login;
    QString email;
    UserRole role;
    QTcpSocket* msock;
};

// Обработка команды
QString processCommand(const QString& command, UserSession& session);

// Отдельные функции команд
QString cmd_reg(const QStringList& parts);
QString cmd_auth(const QStringList& parts, UserSession& session);
QString cmd_logout(UserSession& session);
QString cmd_whoami(const UserSession& session);
QString cmd_del(const QStringList& parts, UserSession& session);
QString cmd_role(const QStringList& parts, const UserSession& session);
QString cmd_users(const UserSession& session);
QString cmd_stats(const UserSession& session);
QString cmd_help();

#endif // SERVER_FUNCTIONS_H
