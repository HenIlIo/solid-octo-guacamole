#include "server_functions.h"

QString parse(const QString& message) {
    qDebug() << "Received message:" << message;

    if (message.trimmed().isEmpty()) {
        qDebug() << "Error: Empty request";
        return "Error: Empty request";
    }

    // Разбиваем сообщение на части.
    QStringList parts = message.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return "error&bad_format";
    }

    // Извлекаем и удаляем команду из списка, чтобы в parts остались только аргументы
    QString command = parts.takeFirst().toUpper();

    if (command == "REG") {
        return handleReg(parts);
    }
    else if (command == "AUTH") {
        return handleAuth(parts);
    }

    qDebug() << "Error: Unknown command ->" << command;
    return "Error: Unknown command";
}

QString handleReg(const QStringList& args) {
    // Ожидаем 3 аргумента: логин, пароль, email
    if (args.size() != 3) {
        qDebug() << "REG Error: Invalid arguments count";
        return "REG Error: Invalid arguments count";
    }

    QString login = args.at(0).trimmed();
    QString password = args.at(1).trimmed();
    QString email = args.at(2).trimmed();

    // Заглушка: базовая проверка данных
    if (login.isEmpty() || password.isEmpty() || email.isEmpty()) {
        qDebug() << "REG Error: Empty fields provided";
        return "REG Error: Empty fields provided";
    }

    qDebug() << "Attempting to register user:" << login << ", email:" << email;

    /* * TИнтеграция с Singleton базы данных.
     * Когда класс работы с БД будет готов
     * * bool success = DatabaseManager::getInstance()->regUser(login, password, email);
     * if (success) {
     * return "reg+";
     * } else {
     * return "REG ERROR";
     * }
     */

    // Временный успешный ответ (Заглушка)
    return "reg+";
}

QString handleAuth(const QStringList& args) {
    // Ожидаем 2 аргумента: логин, пароль
    if (args.size() != 2) {
        qDebug() << "AUTH Error: Invalid arguments count";
        return "AUTH Error: Invalid arguments count";
    }

    QString login = args.at(0).trimmed();
    QString password = args.at(1).trimmed();

    // Заглушка: базовая проверка данных
    if (login.isEmpty() || password.isEmpty()) {
        qDebug() << "AUTH Error: Empty fields provided";
        return "AUTH Error: Empty fields provided";
    }

    qDebug() << "Attempting to authorize user:" << login;

    /* * Интеграция с Singleton базы данных.
     * * bool success = DatabaseManager::getInstance()->authUser(login, password);
     * if (success) {
     * return "auth+&" + login;
     * } else {
     * return "AUTH ERROR";
     * }
     */

    // Временный успешный ответ (Заглушка)
    return "auth+&" + login;
}
