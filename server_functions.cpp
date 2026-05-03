#include "server_functions.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "task4.h"

QString processCommand(const QString& command, UserSession& session)
{
    QStringList parts = command.split(" ", Qt::SkipEmptyParts);
    if (parts.isEmpty()) return "";

    QString action = parts[0].toLower();

    // Если есть активное задание и пользователь ввёл одно число,
    // это считается ответом на задание, а не новой командой.
    if (session.currentTaskType != 0) {
        bool ok;
        int answer = command.trimmed().toInt(&ok);
        if (ok && parts.size() == 1) {
            return cmd_answer(answer, session);
        }
        // Иначе отменяем текущее задание и обрабатываем как новую команду
        session.currentTaskType = 0;
    }

    if (action == "reg") return cmd_reg(parts);
    if (action == "auth") return cmd_auth(parts, session);
    if (action == "logout") return cmd_logout(session);
    if (action == "whoami") return cmd_whoami(session);
    if (action == "del") return cmd_del(parts, session);
    if (action == "role") return cmd_role(parts, session);
    if (action == "users") return cmd_users(session);
    if (action == "stats") return cmd_stats(session);
    if (action == "help") return cmd_help();

    // Команды для выбора задания: "task1 5", "task2 3" и т.д.
    if (action == "task1") {
        if (parts.size() < 2) return "error: usage: task1 <1-40>\r\n";
        bool ok;
        int num = parts[1].toInt(&ok);
        if (!ok || num < 1 || num > 40) return "error: task number must be 1-40\r\n";
        return cmd_task(1, num, session);
    }
    if (action == "task2") {
        if (parts.size() < 2) return "error: usage: task2 <1-40>\r\n";
        bool ok;
        int num = parts[1].toInt(&ok);
        if (!ok || num < 1 || num > 40) return "error: task number must be 1-40\r\n";
        return cmd_task(2, num, session);
    }
    if (action == "task3") {
        if (parts.size() < 2) return "error: usage: task3 <1-40>\r\n";
        bool ok;
        int num = parts[1].toInt(&ok);
        if (!ok || num < 1 || num > 40) return "error: task number must be 1-40\r\n";
        return cmd_task(3, num, session);
    }
    if (action == "task4") {
        if (parts.size() < 2) return "error: usage: task4 <1-40>\r\n";
        bool ok;
        int num = parts[1].toInt(&ok);
        if (!ok || num < 1 || num > 40) return "error: task number must be 1-40\r\n";
        return cmd_task(4, num, session);
    }

    // Команда просмотра личной статистики: "mystats" или "mystats 1"
    if (action == "mystats") return cmd_mystats(parts, session);

    // Echo для неизвестных команд
    return command + "\r\n";
}

QString cmd_reg(const QStringList& parts)
{
    // reg login password email [role] - роль последняя
    if (parts.size() < 4) {
        return "error: usage: reg login password email [role]\r\n";
    }

    QString login = parts[1];
    QString password = parts[2];
    QString email = parts[3];
    UserRole role = ROLE_USER;

    if (parts.size() >= 5) {
        QString roleStr = parts[4].toLower();
        if (roleStr != "user" && roleStr != "moderator" && roleStr != "admin") {
            return "error: invalid role (use: user, moderator, admin)\r\n";
        }
        role = stringToRole(roleStr);
    }

    if (DatabaseManager::getInstance()->userExists(login)) {
        return "error: user exists\r\n";
    }

    if (DatabaseManager::getInstance()->regUser(login, password, email, role)) {
        // При успешной регистрации создаём строки статистики
        // во всех 4 таблицах (StatisticTask1..4) с нулевыми значениями
        DatabaseManager::getInstance()->createStatisticRows(login);
        return "reg+\r\n";
    }
    return "error: reg failed\r\n";
}

QString cmd_auth(const QStringList& parts, UserSession& session)
{
    if (parts.size() < 3) {
        return "error: usage: auth login password\r\n";
    }

    QString login = parts[1];
    QString password = parts[2];

    UserRole role = DatabaseManager::getInstance()->authUserWithRole(login, password);
    QString storedRole = roleToString(DatabaseManager::getInstance()->getUserRole(login));

    if (storedRole == roleToString(role) || DatabaseManager::getInstance()->userExists(login)) {
        if (DatabaseManager::getInstance()->userExists(login)) {
            QString email = DatabaseManager::getInstance()->getUserEmail(login);
            session.login = login;
            session.email = email;
            session.role = DatabaseManager::getInstance()->getUserRole(login);
            session.currentTaskType = 0;
            session.currentTaskNum = 0;
            session.currentCorrectAnswer = 0;
            return QString("auth+ %1\r\n").arg(roleToString(session.role));
        }
    }
    return "auth-\r\n";
}

QString cmd_logout(UserSession& session)
{
    if (session.login.isEmpty()) {
        return "error: not logged in\r\n";
    }
    session.login.clear();
    session.email.clear();
    session.role = ROLE_USER;
    session.currentTaskType = 0;
    session.currentTaskNum = 0;
    session.currentCorrectAnswer = 0;
    return "logout+\r\n";
}

QString cmd_whoami(const UserSession& session)
{
    if (session.login.isEmpty()) {
        return "whoami: not logged in\r\n";
    }
    return QString("Username: %1, Email: %2, Role: %3\r\n")
           .arg(session.login, session.email, roleToString(session.role));
}

QString cmd_del(const QStringList& parts, UserSession& session)
{
    if (session.login.isEmpty()) {
        return "error: login required\r\n";
    }

    if (parts.size() == 1) {
        // Удаление себя
        if (DatabaseManager::getInstance()->deleteUser(session.login)) {
            session.login.clear();
            session.email.clear();
            session.role = ROLE_USER;
            session.currentTaskType = 0;
            session.currentTaskNum = 0;
            session.currentCorrectAnswer = 0;
            return "del+\r\n";
        }
        return "error: delete failed\r\n";
    }

    // Удаление другого (admin only)
    if (session.role != ROLE_ADMIN) {
        return "error: need admin rights\r\n";
    }

    QString targetLogin = parts[1];
    if (DatabaseManager::getInstance()->deleteUser(targetLogin)) {
        return QString("del+ %1\r\n").arg(targetLogin);
    }
    return "error: delete failed\r\n";
}

QString cmd_role(const QStringList& parts, const UserSession& session)
{
    if (session.login.isEmpty()) {
        return "error: login required\r\n";
    }

    if (session.role != ROLE_ADMIN) {
        return "error: need admin rights\r\n";
    }

    if (parts.size() < 3) {
        return "error: usage: role login newRole\r\n";
    }

    QString targetLogin = parts[1];
    QString newRoleStr = parts[2].toLower();

    if (newRoleStr != "user" && newRoleStr != "moderator" && newRoleStr != "admin") {
        return "error: invalid role (use: user, moderator, admin)\r\n";
    }

    UserRole newRole = stringToRole(newRoleStr);
    if (DatabaseManager::getInstance()->updateUserRole(targetLogin, newRole)) {
        return "role+\r\n";
    }
    return "error: update failed\r\n";
}

QString cmd_users(const UserSession& session)
{
    if (session.login.isEmpty()) {
        return "error: login required\r\n";
    }

    if (session.role != ROLE_ADMIN && session.role != ROLE_MODERATOR) {
        return "error: need admin or moderator rights\r\n";
    }

    QVector<UserInfo> users = DatabaseManager::getInstance()->getAllUsers();
    QString result = "=== Users ===\r\n";

    for (const UserInfo& user : users) {
        result += QString("  %1 | %2 | %3 | Reg: %4 | Last: %5\r\n")
                  .arg(user.login)
                  .arg(user.email.isEmpty() ? "-" : user.email)
                  .arg(roleToString(user.role))
                  .arg(user.registeredAt)
                  .arg(user.lastAuth.isEmpty() ? "never" : user.lastAuth);
    }

    result += QString("Total: %1\r\n").arg(users.size());
    return result;
}

QString cmd_stats(const UserSession& session)
{
    if (session.login.isEmpty()) {
        return "error: login required\r\n";
    }

    if (session.role != ROLE_ADMIN && session.role != ROLE_MODERATOR) {
        return "error: need admin or moderator rights\r\n";
    }

    UserStats stats = DatabaseManager::getInstance()->getStats();

    QString result = "=== Statistics ===\r\n";
    result += QString("  Total users: %1\r\n").arg(stats.totalUsers);
    result += QString("  New (last month): %1\r\n").arg(stats.newUsersMonth);
    result += QString("  Active (last month): %1\r\n").arg(stats.activeUsersMonth);
    return result;
}

// [ИЗМЕНЕНО] Обновлённый help с описанием всех команд, включая задания и статистику
QString cmd_help()
{
    return "Commands:\r\n"
           "  reg login password email [role] - register\r\n"
           "  auth login password - login\r\n"
           "  logout - logout\r\n"
           "  whoami - show current user info\r\n"
           "  del [login] - delete account (admin can delete any)\r\n"
           "  role login newRole - change role (admin only)\r\n"
           "  users - list all users (admin/mod only)\r\n"
           "  stats - show server statistics (admin/mod only)\r\n"
           "--- Tasks (require login) ---\r\n"
           "  task1 <1-40> - integral comparison (trapezoid vs Simpson)\r\n"
           "  task2 <1-40> - sum of two numbers (small)\r\n"
           "  task3 <1-40> - sum of two numbers (medium)\r\n"
           "  task4 <1-40> - sum of two numbers (large)\r\n"
           "  mystats [1-4] - view your task statistics\r\n"
           "--- Answering ---\r\n"
           "  After taskX N, enter the answer as a number.\r\n"
           "  For task1: 1 (trapezoid) or 2 (Simpson).\r\n"
           "  For task2-4: the computed sum.\r\n"
           "  +1 for correct, -1 for wrong.\r\n"
           "Roles: user, moderator, admin\r\n";
}

// Обработка запроса задания. Формат: taskType (1-4), taskNum (1-40).
// Генерирует задание, сохраняет правильный ответ в сессии
QString cmd_task(int taskType, int taskNum, UserSession& session)
{
    // Требуется авторизация
    if (session.login.isEmpty()) {
        return "error: login required to use tasks\r\n";
    }

    QString problemText;
    int correctAnswer = 0;

    // В зависимости от типа задания вызываем соответствующую функцию генерации
    switch (taskType) {
    case 1: {
        Task1Problem p = task1_generate(taskNum);
        problemText = p.text;
        correctAnswer = p.correctAnswer;
        break;
    }
    case 2: {
        Task2Problem p = task2_generate(taskNum);
        problemText = p.text;
        correctAnswer = p.correctAnswer;
        break;
    }
    case 3: {
        Task3Problem p = task3_generate(taskNum);
        problemText = p.text;
        correctAnswer = p.correctAnswer;
        break;
    }
    case 4: {
        Task4Problem p = task4_generate(taskNum);
        problemText = p.text;
        correctAnswer = p.correctAnswer;
        break;
    }
    default:
        return "error: invalid task type (1-4)\r\n";
    }

    // Сохраняем состояние активного задания в сессии
    session.currentTaskType = taskType;
    session.currentTaskNum = taskNum;
    session.currentCorrectAnswer = correctAnswer;

    return problemText;
}

// Обработка ответа пользователя на активное задание.
// Проверяет ответ, обновляет статистику в БД (+1 или -1),
// отправляет результат и сбрасывает активное задание.
QString cmd_answer(int userAnswer, UserSession& session)
{
    // Проверяем, есть ли активное задание
    if (session.currentTaskType == 0) {
        return "error: no active task. Use task1/2/3/4 <1-40> first.\r\n";
    }

    int taskType = session.currentTaskType;
    int taskNum = session.currentTaskNum;
    int correctAnswer = session.currentCorrectAnswer;

    // Определяем правильность ответа
    bool isCorrect = (userAnswer == correctAnswer);

    // Обновляем статистику в БД: +1 за правильный, -1 за неправильный
    int delta = isCorrect ? 1 : -1;
    DatabaseManager::getInstance()->updateStatistic(session.login, taskType, taskNum, delta);

    // Получаем текущее значение из таблицы для отображения
    QVector<int> row = DatabaseManager::getInstance()->getStatisticRow(session.login, taskType);
    int currentValue = row.value(taskNum - 1, 0); // индекс 0..39 для заданий 1..40

    // Формируем строку с именем таблицы для вывода
    QString tableName = QString("StatisticTask%1[%2]").arg(taskType).arg(taskNum);

    // Сбрасываем активное задание
    session.currentTaskType = 0;
    session.currentTaskNum = 0;
    session.currentCorrectAnswer = 0;

    if (isCorrect) {
        return QString("Верно! %1 += 1 (текущее значение: %2)\r\n")
               .arg(tableName).arg(currentValue);
    } else {
        return QString("Неверно! %1 -= 1 (текущее: %2). "
                       "Правильный ответ: %3\r\n")
               .arg(tableName).arg(currentValue).arg(correctAnswer);
    }
}

// Просмотр личной статистики по заданиям.
// "mystats" — показывает все 4 Task, "mystats 1" — только Task1 и т.д.
QString cmd_mystats(const QStringList& parts, const UserSession& session)
{
    // Требуется авторизация
    if (session.login.isEmpty()) {
        return "error: login required\r\n";
    }

    // Определяем, какой Task запрошен
    int requestedTask = 0;
    if (parts.size() >= 2) {
        int t = parts[1].toInt();
        if (t < 0 || t > 4) {
            return "error: usage: mystats [0-4]\r\n";
        }
        requestedTask = t;
    }

    QString result = QString("=== Статистика пользователя: %1 ===\r\n").arg(session.login);

    // Определяем диапазон Task для отображения
    int startTask = (requestedTask == 0) ? 1 : requestedTask;
    int endTask   = (requestedTask == 0) ? 4 : requestedTask;

    for (int taskType = startTask; taskType <= endTask; taskType++) {
        QString taskName = QString("Task%1").arg(taskType);
        result += QString("--- %1 ---\r\n").arg(taskName);

        QVector<int> row = DatabaseManager::getInstance()->getStatisticRow(session.login, taskType);

        // Выводим значения по 10 в строке для компактности
        for (int line = 0; line < 4; line++) {
            QStringList vals;
            for (int col = 0; col < 10; col++) {
                int idx = line * 10 + col;
                vals << QString("%1:%2").arg(idx + 1).arg(row[idx]);
            }
            result += "  " + vals.join("  ") + "\r\n";
        }
    }

    return result;
}
