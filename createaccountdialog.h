#ifndef CREATEACCOUNTDIALOG_H
#define CREATEACCOUNTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "database.h"

class CreateAccountDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateAccountDialog(Database &db, QWidget *parent = nullptr);
    QString getUsername() const;

    private slots:
        void onCreateAccountClicked();

private:
    Database &database; // Ссылка на объект Database
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
};

#endif // CREATEACCOUNTDIALOG_H
