#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QVector>

// +++ Enum для ролей
enum UserRole {
    ROLE_USER,
    ROLE_MODERATOR,
    ROLE_ADMIN
};

UserRole stringToRole(const QString& roleStr);
QString roleToString(UserRole role);

// +++ Структура пользователя для списка
struct UserInfo {
    QString login;
    QString email;
    UserRole role;
    QString registeredAt;
    QString lastAuth;
};

// +++ Структура статистики
struct UserStats {
    int totalUsers;
    int newUsersMonth;      // за последний месяц
    int activeUsersMonth;   // активные за месяц
};

class DatabaseManager {
public:
    static DatabaseManager* getInstance();

    // === Пользователи ===
    bool regUser(const QString& login, const QString& password, const QString& email, UserRole role);
    bool authUser(const QString& login, const QString& password);
    UserRole authUserWithRole(const QString& login, const QString& password);
    UserRole getUserRole(const QString& login);
    QString getUserEmail(const QString& login);
    bool deleteUser(const QString& login);
    bool updateUserRole(const QString& login, UserRole newRole);
    bool userExists(const QString& login);

    // === Для админа/модератора ===
    QVector<UserInfo> getAllUsers();
    UserStats getStats();

    // Создать строки со значениями 0 во всех 4 таблицах статистики для нового пользователя
    bool createStatisticRows(const QString& login);
    // Обновить ячейку: taskNum (1-4), problemNum (1-40), delta (+1 или -1)
    bool updateStatistic(const QString& login, int taskNum, int problemNum, int delta);
    // Получить всю строку статистики (40 значений) для пользователя
    QVector<int> getStatisticRow(const QString& login, int taskNum);
    // Удалить строки статистики пользователя (при удалении аккаунта)
    bool deleteStatisticRows(const QString& login);

private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase db;
    void initDatabase();
    // Создание 4 таблиц статистики (StatisticTask1..4)
    void initStatisticTables();
    void updateLastAuth(const QString& login);
};

#endif // DATABASEMANAGER_H
