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

    initStatisticTables();
}

// Каждая таблица содержит login (PRIMARY KEY, ссылается на User) и 40 столбцов p1..p40 по умолчанию с 0.
// При правильном ответе значение +1, при неправильном -1.
void DatabaseManager::initStatisticTables()
{
    QSqlQuery query;

    // Генерируем строку с 40 столбцами p1..p40
    QString columns;
    for (int i = 1; i <= 40; i++) {
        if (i > 1) columns += ", ";
        columns += QString("p%1 INTEGER DEFAULT 0").arg(i);
    }

    // Создаём 4 таблицы статистики для Task1..Task4
    for (int taskNum = 1; taskNum <= 4; taskNum++) {
        QString tableName = QString("StatisticTask%1").arg(taskNum);
        QString sql = QString("CREATE TABLE IF NOT EXISTS %1("
                              "login VARCHAR(20) NOT NULL PRIMARY KEY, "
                              "%2, "
                              "FOREIGN KEY(login) REFERENCES User(login))")
                      .arg(tableName).arg(columns);
        if (!query.exec(sql)) {
            qDebug() << "Failed to create table" << tableName << ":" << query.lastError().text();
        }
    }
    qDebug() << "Statistic tables created (StatisticTask1..4)";
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
    deleteStatisticRows(login);

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

// Создание строк со значениями 0 во всех 4 таблицах статистики для нового пользователя.
// Вызывается после успешной регистрации.
bool DatabaseManager::createStatisticRows(const QString& login)
{
    QSqlQuery query;
    bool ok = true;

    for (int taskNum = 1; taskNum <= 4; taskNum++) {
        QString tableName = QString("StatisticTask%1").arg(taskNum);
        // Формируем список из 40 нулей для вставки
        QString values = "'" + login + "'";
        for (int i = 1; i <= 40; i++) {
            values += ", 0";
        }
        QString sql = QString("INSERT INTO %1 (login, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, "
                             "p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, "
                             "p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, "
                             "p31, p32, p33, p34, p35, p36, p37, p38, p39, p40) "
                             "VALUES (%2)")
                      .arg(tableName).arg(values);
        if (!query.exec(sql)) {
            qDebug() << "Failed to create stat row for" << login << "in" << tableName
                     << ":" << query.lastError().text();
            ok = false;
        }
    }
    return ok;
}

// Обновление ячейки в таблице статистики.
// taskNum: 1-4 (какой Task), problemNum: 1-40 (номер задания), delta: +1 или -1.
bool DatabaseManager::updateStatistic(const QString& login, int taskNum, int problemNum, int delta)
{
    QString tableName = QString("StatisticTask%1").arg(taskNum);
    QString column = QString("p%1").arg(problemNum);

    QSqlQuery query;
    QString sql = QString("UPDATE %1 SET %2 = %2 + :delta WHERE login = :login")
                  .arg(tableName).arg(column);
    query.prepare(sql);
    query.bindValue(":delta", delta);
    query.bindValue(":login", login);
    return query.exec();
}

// Получение всей строки статистики (40 значений) для пользователя из указанной таблицы.
QVector<int> DatabaseManager::getStatisticRow(const QString& login, int taskNum)
{
    QVector<int> result(40, 0);
    if (taskNum < 1 || taskNum > 4) return result;

    QString tableName = QString("StatisticTask%1").arg(taskNum);
    QSqlQuery query;
    query.prepare(QString("SELECT p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,"
                         "p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,"
                         "p21,p22,p23,p24,p25,p26,p27,p28,p29,p30,"
                         "p31,p32,p33,p34,p35,p36,p37,p38,p39,p40 "
                         "FROM %1 WHERE login = :login").arg(tableName));
    query.bindValue(":login", login);

    if (query.exec() && query.next()) {
        for (int i = 0; i < 40; i++) {
            result[i] = query.value(i).toInt();
        }
    }
    return result;
}

// Удаление строк статистики пользователя из всех 4 таблиц (при удалении аккаунта).
bool DatabaseManager::deleteStatisticRows(const QString& login)
{
    QSqlQuery query;
    bool ok = true;
    for (int taskNum = 1; taskNum <= 4; taskNum++) {
        QString tableName = QString("StatisticTask%1").arg(taskNum);
        query.prepare(QString("DELETE FROM %1 WHERE login = :login").arg(tableName));
        query.bindValue(":login", login);
        if (!query.exec()) ok = false;
    }
    return ok;
}
