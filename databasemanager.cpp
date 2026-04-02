#include "databasemanager.h"
#include <QDateTime>

UserRole stringToRole(const QString& roleStr)
{
    QString lower = roleStr.toLower();
    if (lower == "user") return ROLE_USER;
    if (lower == "moderator") return ROLE_MODERATOR;
    if (lower == "admin") return ROLE_ADMIN;
    return ROLE_USER;
}

QString roleToString(UserRole role)
{
    switch (role) {
        case ROLE_USER: return "user";
        case ROLE_MODERATOR: return "moderator";
        case ROLE_ADMIN: return "admin";
        default: return "user";
    }
}

DatabaseManager* DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return &instance;
}

DatabaseManager::DatabaseManager()
{
    initDatabase();
}

void DatabaseManager::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("server.db");

    if(!db.open())
        qDebug() << db.lastError().text();

    /*
    Таблица User
    Поля: login, password, email, registered_at, last_auth, role
    (роль последняя)
    */
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS User("
               "login VARCHAR(20) NOT NULL, "
               "password VARCHAR(128) NOT NULL, "
               "email VARCHAR(50), "
               "registered_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "last_auth DATETIME, "
               "role VARCHAR(20) NOT NULL DEFAULT 'user', "
               "PRIMARY KEY(login))");

    // Создаём администратора по умолчанию
    query.exec("SELECT login FROM User WHERE login='admin'");
    if (!query.next()) {
        QByteArray hash = QCryptographicHash::hash("admin123", QCryptographicHash::Sha256);
        query.prepare("INSERT INTO User(login, password, email, role) VALUES (:login, :password, :email, :role)");
        query.bindValue(":login", "admin");
        query.bindValue(":password", hash.toHex());
        query.bindValue(":email", "");
        query.bindValue(":role", roleToString(ROLE_ADMIN));
        query.exec();
        qDebug() << "Created default admin";
    }
}

bool DatabaseManager::regUser(const QString& login, const QString& password, const QString& email, UserRole role)
{
    if (userExists(login)) {
        return false;
    }

    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QSqlQuery query;
    query.prepare("INSERT INTO User(login, password, email, role) VALUES (:login, :password, :email, :role)");
    query.bindValue(":login", login);
    query.bindValue(":password", hash.toHex());
    query.bindValue(":email", email);
    query.bindValue(":role", roleToString(role));
    return query.exec();
}

bool DatabaseManager::authUser(const QString& login, const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QSqlQuery query;
    query.prepare("SELECT login FROM User WHERE login=:login AND password=:password");
    query.bindValue(":login", login);
    query.bindValue(":password", hash.toHex());
    bool success = query.exec() && query.next();
    if (success) {
        updateLastAuth(login);
    }
    return success;
}

UserRole DatabaseManager::authUserWithRole(const QString& login, const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QSqlQuery query;
    query.prepare("SELECT role FROM User WHERE login=:login AND password=:password");
    query.bindValue(":login", login);
    query.bindValue(":password", hash.toHex());
    if (query.exec() && query.next()) {
        updateLastAuth(login);
        return stringToRole(query.value(0).toString());
    }
    return ROLE_USER;
}

void DatabaseManager::updateLastAuth(const QString& login)
{
    QSqlQuery query;
    query.prepare("UPDATE User SET last_auth = CURRENT_TIMESTAMP WHERE login=:login");
    query.bindValue(":login", login);
    query.exec();
}

UserRole DatabaseManager::getUserRole(const QString& login)
{
    QSqlQuery query;
    query.prepare("SELECT role FROM User WHERE login=:login");
    query.bindValue(":login", login);
    if (query.exec() && query.next()) {
        return stringToRole(query.value(0).toString());
    }
    return ROLE_USER;
}

QString DatabaseManager::getUserEmail(const QString& login)
{
    QSqlQuery query;
    query.prepare("SELECT email FROM User WHERE login=:login");
    query.bindValue(":login", login);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "";
}

bool DatabaseManager::deleteUser(const QString& login)
{
    QSqlQuery query;
    query.prepare("DELETE FROM User WHERE login=:login");
    query.bindValue(":login", login);
    return query.exec();
}

bool DatabaseManager::updateUserRole(const QString& login, UserRole newRole)
{
    QSqlQuery query;
    query.prepare("UPDATE User SET role=:role WHERE login=:login");
    query.bindValue(":role", roleToString(newRole));
    query.bindValue(":login", login);
    return query.exec();
}

bool DatabaseManager::userExists(const QString& login)
{
    QSqlQuery query;
    query.prepare("SELECT login FROM User WHERE login=:login");
    query.bindValue(":login", login);
    return query.exec() && query.next();
}

QVector<UserInfo> DatabaseManager::getAllUsers()
{
    QVector<UserInfo> users;
    QSqlQuery query;
    query.prepare("SELECT login, email, role, registered_at, last_auth FROM User ORDER BY login");

    if (query.exec()) {
        while (query.next()) {
            UserInfo user;
            user.login = query.value(0).toString();
            user.email = query.value(1).toString();
            user.role = stringToRole(query.value(2).toString());
            user.registeredAt = query.value(3).toString();
            user.lastAuth = query.value(4).toString();
            users.append(user);
        }
    }
    return users;
}

UserStats DatabaseManager::getStats()
{
    UserStats stats = {0, 0, 0};

    // Общее число пользователей
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM User");
    if (query.next()) {
        stats.totalUsers = query.value(0).toInt();
    }

    // Новые за последний месяц
    query.exec("SELECT COUNT(*) FROM User WHERE registered_at >= datetime('now', '-1 month')");
    if (query.next()) {
        stats.newUsersMonth = query.value(0).toInt();
    }

    // Активные за последний месяц
    query.exec("SELECT COUNT(*) FROM User WHERE last_auth >= datetime('now', '-1 month')");
    if (query.next()) {
        stats.activeUsersMonth = query.value(0).toInt();
    }

    return stats;
}
