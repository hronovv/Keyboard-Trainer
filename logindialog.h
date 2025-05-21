#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "database.h"

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(Database &db, QWidget *parent = nullptr);
    QString getUsername() const;
    bool isLoggedIn() const;

    private slots:
        void onLoginClicked();
    void showCreateAccountDialog();

private:
    Database &database; // Ссылка на объект Database
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QString username;
    bool loggedIn;
};

#endif // LOGINDIALOG_H