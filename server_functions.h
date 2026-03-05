#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <QString>
#include <QStringList>
#include <QDebug>

/**
 * @brief Главная функция парсинга входящих сообщений.
 * @param message Входящая строка от клиента.
 * @return Ответ сервера для отправки клиенту.
 */
QString parse(const QString& message);

/**
 * @brief Обработчик регистрации.
 * @param args Список аргументов: [логин, пароль, email].
 * @return Строка ответа (успех или ошибка).
 */
QString handleReg(const QStringList& args);

/**
 * @brief Обработчик авторизации.
 * @param args Список аргументов: [логин, пароль].
 * @return Строка ответа (успех или ошибка).
 */
QString handleAuth(const QStringList& args);

#endif // SERVER_FUNCTIONS_H
