#include "database.h"
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>
#include <QDebug>
#include <QDir>

Database::Database(QObject *parent) : QObject(parent) {}

Database::~Database() {
    if(db.isOpen()) {
        db.close();
    }
}

bool Database::initDatabase() {
    if (!QSqlDatabase::contains("my_connection")) {
        db = QSqlDatabase::addDatabase("QSQLITE", "my_connection");
        QString dbPath = QDir::currentPath() + "/keyboard_trainer.db";
        db.setDatabaseName(dbPath);

        if (!db.open()) {
            qDebug() << "Error opening database:" << db.lastError().text();
            return false;
        }
    } else {
        db = QSqlDatabase::database("my_connection");
    }

    QSqlQuery query(db);
    bool ok = query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username TEXT UNIQUE NOT NULL, "
               "password_hash TEXT NOT NULL, "
               "font TEXT DEFAULT NULL, "
               "font_color TEXT DEFAULT '#FFFFFF', "
               "font_size INTEGER DEFAULT 16, "
               "letter_spacing INTEGER DEFAULT 2, "
               "word_spacing INTEGER DEFAULT 2, "
               "font_weight INTEGER DEFAULT 500, "
               "line_height INTEGER DEFAULT 20,"
               "caret_smooth TEXT DEFAULT off,"
               "caret_style TEXT DEFAULT ▮"
               ")");
    if (!ok) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }

    ok = query.exec(R"(
        CREATE TABLE IF NOT EXISTS typing_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            session_date DATETIME DEFAULT (datetime('now', 'localtime')),
            wpm REAL NOT NULL,
            accuracy REAL NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        )
    )");
    if (!ok) {
        qDebug() << "Error creating typing_sessions table:" << query.lastError().text();
        return false;
    }

    return true;
}

QString Database::hashPassword(const QString &password) {
    QString salt = "some_random_salt";
    return QString(QCryptographicHash::hash((password + salt).toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool Database::createUser(const QString &username, const QString &password) {
    if(userExists(username)) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password_hash) VALUES (:username, :hash)");
    query.bindValue(":username", username);
    query.bindValue(":hash", hashPassword(password));
    return query.exec();
}

bool Database::authenticateUser(const QString &username, const QString &password) {
    QSqlQuery query(db);
    query.prepare("SELECT password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if(!query.exec() || !query.next()) {
        return false;
    }

    QString storedHash = query.value(0).toString();
    return storedHash == hashPassword(password);
}

bool Database::userExists(const QString &username) {
    QSqlQuery query(db);
    query.prepare("SELECT 1 FROM users WHERE username = :username LIMIT 1");
    query.bindValue(":username", username);

    return query.exec() && query.next();
}

bool Database::updateUserSetting(const QString &username, const QString &settingName, const QVariant &value) {
    static const QSet<QString> allowedColumns = {"font", "font_color", "font_size", "letter_spacing",
        "word_spacing", "font_weight", "line_height","caret_smooth", "caret_style"};
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

UserSettings Database::getUserSettings(const QString &username) {
    UserSettings settings;
    QSqlQuery query(db);
    query.prepare("SELECT font, font_color, font_size, letter_spacing, word_spacing, font_weight, "
                  "line_height, caret_smooth, caret_style FROM users WHERE username = :username LIMIT 1");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        settings.font = query.value(0).toString();
        settings.font_color = QColor(query.value(1).toString());
        settings.font_size = query.value(2).toInt();
        settings.letter_spacing = query.value(3).toInt();
        settings.word_spacing = query.value(4).toInt();
        settings.font_weight = query.value(5).toInt();
        settings.line_height = query.value(6).toInt();
        settings.caret_smooth = query.value(7).toString();
        settings.caret_style = query.value(8).toString();
    } else {
        qDebug() << "Failed to get user settings:" << query.lastError().text();
    }

    return settings;
}

bool Database::saveTypingSession(const QString &username, double wpm, double accuracy) {
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec() || !query.next()) {
        qDebug() << "User not found for saving session:" << query.lastError().text();
        return false;
    }

    int user_id = query.value(0).toInt();

    // Вставляем новую запись
    query.prepare("INSERT INTO typing_sessions (user_id, wpm, accuracy) VALUES (:user_id, :wpm, :accuracy)");
    query.bindValue(":user_id", user_id);
    query.bindValue(":wpm", wpm);
    query.bindValue(":accuracy", accuracy);

    if (!query.exec()) {
        qDebug() << "Failed to save typing session:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<QPair<QDateTime, double>> Database::getTypingSessionsForUser(const QString &username) {
    QVector<QPair<QDateTime, double>> result;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT ts.session_date, ts.wpm
        FROM typing_sessions ts
        JOIN users u ON ts.user_id = u.id
        WHERE u.username = :username
        ORDER BY ts.session_date ASC
    )");
    query.bindValue(":username", username);

    if (query.exec()) {
        while (query.next()) {
            QDateTime date = query.value(0).toDateTime();
            double wpm = query.value(1).toDouble();
            result.append(qMakePair(date, wpm));
        }
    } else {
        qDebug() << "Failed to get typing sessions:" << query.lastError().text();
    }

    return result;
}
