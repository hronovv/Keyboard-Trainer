#include "logindialog.h"
#include "createaccountdialog.h"
#include "database.h"
#include <QAction>
#include <QFont>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

LoginDialog::LoginDialog(Database &db, QWidget *parent) :
    QDialog(parent), database(db), loggedIn(false)
{
    setWindowTitle("Вход в систему");
    setFixedSize(400, 300);

    // Заголовок
    QLabel *titleLabel = new QLabel("Клавиатурный тренажер", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // Поля ввода
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Имя пользователя");

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);

    // Глазик
    QAction *togglePasswordAction = passwordEdit->addAction(QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);
    togglePasswordAction->setCheckable(true);
    connect(togglePasswordAction, &QAction::toggled, [this](bool checked) {
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    // Кнопки
    QPushButton *loginButton = new QPushButton("Войти", this);
    loginButton->setDefault(true);
    QPushButton *createAccountButton = new QPushButton("Создать аккаунт", this);

    // Разметка
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(createAccountButton);

    mainLayout->addLayout(buttonLayout);

    // Сигналы
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(createAccountButton, &QPushButton::clicked, this, &LoginDialog::showCreateAccountDialog);
}

void LoginDialog::showCreateAccountDialog() {
    CreateAccountDialog dialog(database, this);
    if (dialog.exec() == QDialog::Accepted) {
        usernameEdit->setText(dialog.getUsername());
        passwordEdit->clear();
    }
}

void LoginDialog::onLoginClicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя пользователя и пароль");
        return;
    }

    if (database.authenticateUser(username, password)) {
        this->username = username;
        loggedIn = true;
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверное имя пользователя или пароль");
    }
}

QString LoginDialog::getUsername() const { return username; }

bool LoginDialog::isLoggedIn() const { return loggedIn; }
