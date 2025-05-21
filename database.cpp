#include "database.h"
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>
#include <QDebug>
#include <QDir>

Database::Database(QObject *parent) : QObject(parent)
{
}

Database::~Database()
{
    if(db.isOpen()) {
        db.close();
    }
}

bool Database::initDatabase()
{
    // Уникальное имя соединения
    if (!QSqlDatabase::contains("my_connection")) {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_connection");
        QString dbPath = QDir::currentPath() + "/keyboard_trainer.db";
        db.setDatabaseName(dbPath);

        if (!db.open()) {
            qDebug() << "Error opening database:" << db.lastError().text();
            return false;
        }
    } else {
        db = QSqlDatabase::database("my_connection"); // Получаем существующее соединение
    }

    // Создаем таблицу пользователей
    QSqlQuery query(db); // Указываем соединение для запроса
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username TEXT UNIQUE NOT NULL, "
               "password_hash TEXT NOT NULL, "
               "font TEXT DEFAULT NULL)")) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }

    return true;
}

QString Database::hashPassword(const QString &password)
{
    QString salt = "some_random_salt"; // Замените на более сложный механизм генерации соли
    return QString(QCryptographicHash::hash((password + salt).toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool Database::createUser(const QString &username, const QString &password)
{
    if(userExists(username)) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password_hash) VALUES (:username, :hash)");
    query.bindValue(":username", username);
    query.bindValue(":hash", hashPassword(password));


    return query.exec();
}


bool Database::authenticateUser(const QString &username, const QString &password)
{
    QSqlQuery query(db); // Указываем соединение для запроса
    query.prepare("SELECT password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if(!query.exec() || !query.next()) {
        return false;
    }

    QString storedHash = query.value(0).toString();
    return storedHash == hashPassword(password);
}

bool Database::userExists(const QString &username)
{
    QSqlQuery query(db);
    query.prepare("SELECT 1 FROM users WHERE username = :username LIMIT 1");
    query.bindValue(":username", username);

    return query.exec() && query.next();
}

bool Database::updateUserSetting(const QString &username, const QString &settingName, const QVariant &value)
{
    if (settingName.isEmpty())
        return false;

    static const QSet<QString> allowedColumns = {"font", "font_color"};

    if (!allowedColumns.contains(settingName)) {
        qDebug() << "Invalid setting name:" << settingName;
        return false;
    }

    QString sql = QString("UPDATE users SET %1 = :value WHERE username = :username").arg(settingName);
    QSqlQuery query(db);
    if (!query.prepare(sql)) {
        qDebug() << "Failed to prepare query:" << query.lastError().text();
        return false;
    }

    query.bindValue(":value", value);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Failed to update user setting:" << query.lastError().text();
        return false;
    }

    return true;
}

UserSettings Database::getUserSettings(const QString &username)
{
    UserSettings settings;

    QSqlQuery query(db);
    query.prepare("SELECT font, font_color FROM users WHERE username = :username LIMIT 1");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        settings.font = query.value(0).toString();
        settings.font_color = query.value(1).toString();
    } else {
        qDebug() << "Failed to get user settings:" << query.lastError().text();
    }

    return settings;
}
