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

    if (action == "reg") return cmd_reg(parts);
    if (action == "auth") return cmd_auth(parts, session);
    if (action == "logout") return cmd_logout(session);
    if (action == "whoami") return cmd_whoami(session);
    if (action == "del") return cmd_del(parts, session);
    if (action == "role") return cmd_role(parts, session);
    if (action == "users") return cmd_users(session);
    if (action == "stats") return cmd_stats(session);
    if (action == "task1") return task1_execute();
    if (action == "task2") return task2_execute();
    if (action == "task3") return task3_execute();
    if (action == "task4") return task4_execute();
    if (action == "help") return cmd_help();

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
           "  stats - show statistics (admin/mod only)\r\n"
           "--- Tasks ---\r\n"
           "  task1/2/3/4 - task stubs\r\n"
           "Roles: user, moderator, admin\r\n";
}
