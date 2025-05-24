#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QColor>
#include <QtSql/qsqldatabase.h>
#include <QCryptographicHash>

struct UserSettings {
    QString font;
    QColor font_color;
    int font_size;
    int letter_spacing;
    int word_spacing;
    int font_weight;
    int line_height;
    QString caret_smooth;
    QString caret_style;
};

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool initDatabase();
    bool createUser(const QString &username, const QString &password);
    bool authenticateUser(const QString &username, const QString &password);
    bool userExists(const QString &username);
    bool updateUserSetting(const QString &username, const QString &settingName, const QVariant &value);
    UserSettings getUserSettings(const QString &username);
    bool saveTypingSession(const QString &username, double wpm, double accuracy);
    QVector<QPair<QDateTime, double>> getTypingSessionsForUser(const QString &username);


private:
    QSqlDatabase db;
    QString hashPassword(const QString &password);
};

#endif // DATABASE_H