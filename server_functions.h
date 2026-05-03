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

    int currentTaskType;    // 0 = нет активного задания, 1-4 = тип задания
    int currentTaskNum;     // 1-40 = номер задания в рамках типа
    int currentCorrectAnswer;
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

// Обработка запроса задания: "task1 5", "task2 3" и т.д.
QString cmd_task(int taskType, int taskNum, UserSession& session);
// Обработка ответа пользователя на активное задание
QString cmd_answer(int userAnswer, UserSession& session);
// Просмотр личной статистики: "mystats" (все) или "mystats 1" (конкретный Task)
QString cmd_mystats(const QStringList& parts, const UserSession& session);

#endif // SERVER_FUNCTIONS_H
