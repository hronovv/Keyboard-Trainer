#include "createaccountdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QAction>
#include <QIcon>
#include <QFont>
#include <QMessageBox>

CreateAccountDialog::CreateAccountDialog(Database &db, QWidget *parent)
    : QDialog(parent), database(db)
{
    setWindowTitle("Создание аккаунта");
    setFixedSize(400, 300);

    QLabel *titleLabel = new QLabel("Создание нового аккаунта", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont font = titleLabel->font();
    font.setPointSize(14);
    font.setBold(true);
    titleLabel->setFont(font);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Имя пользователя");

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QAction *togglePasswordAction = passwordEdit->addAction(QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);
    togglePasswordAction->setCheckable(true);
    connect(togglePasswordAction, &QAction::toggled, [this](bool checked) {
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    QPushButton *createAccountButton = new QPushButton("Создать аккаунт", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    layout->addWidget(titleLabel);
    layout->addSpacing(10);
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(confirmPasswordEdit);
    layout->addSpacing(10);
    layout->addWidget(createAccountButton);

    connect(createAccountButton, &QPushButton::clicked, this, &CreateAccountDialog::onCreateAccountClicked);
}

void CreateAccountDialog::onCreateAccountClicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля.");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают.");
        return;
    }

    if (database.userExists(username)) {
        QMessageBox::warning(this, "Ошибка", "Пользователь с таким именем уже существует.");
        return;
    }

    if (database.createUser(username, password)) {
        QMessageBox::information(this, "Успех", "Аккаунт успешно создан.");
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать аккаунт.");
    }
}

QString CreateAccountDialog::getUsername() const {
    return usernameEdit->text();
}
