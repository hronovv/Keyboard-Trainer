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
#include <QtSvgWidgets/qsvgwidget.h>

LoginDialog::LoginDialog(Database &db, QWidget *parent) :
    QDialog(parent), database(db), loggedIn(false)
{
    setWindowTitle("Вход в систему");
    resize(425, 365);

    // Основной контейнер с темным фоном
    QWidget *formContainer = new QWidget(this);
    formContainer->setObjectName("formContainer");
    formContainer->setFixedSize(400, 340);

    QVBoxLayout *containerLayout = new QVBoxLayout(formContainer);
    containerLayout->setContentsMargins(35, 30, 35, 30);
    containerLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Клавиатурный тренажер", formContainer);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #d8dee9;");

    QLabel *subtitleLabel = new QLabel("Пожалуйста, войдите в свой аккаунт", formContainer);
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(15);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #d8dee9;");

    usernameEdit = new QLineEdit(formContainer);
    usernameEdit->setPlaceholderText("Имя пользователя");
    usernameEdit->setObjectName("inputField");

    passwordEdit = new QLineEdit(formContainer);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setObjectName("inputField");

    // Иконка "глазик"
    QIcon eyeOpenedIcon(QStringLiteral("/Users/hronov/Documents/Keyboard Trainer/icons/opened-eye.svg"));
    QIcon eyeClosedIcon(QStringLiteral("/Users/hronov/Documents/Keyboard Trainer/icons/closed-eye.svg"));

    QAction *togglePasswordAction = passwordEdit->addAction(eyeClosedIcon, QLineEdit::TrailingPosition);
    togglePasswordAction->setCheckable(true);
    connect(togglePasswordAction, &QAction::toggled, this, [=](bool checked){
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        togglePasswordAction->setIcon(checked ? eyeOpenedIcon : eyeClosedIcon);
    });

    QPushButton *loginButton = new QPushButton("Войти", formContainer);
    loginButton->setObjectName("primaryButton");
    loginButton->setDefault(true);
    loginButton->setCursor(Qt::PointingHandCursor);

    QPushButton *createAccountButton = new QPushButton("Создать аккаунт", formContainer);
    createAccountButton->setObjectName("secondaryButton");
    createAccountButton->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(loginButton);
    buttonsLayout->addWidget(createAccountButton);
    buttonsLayout->setSpacing(20);

    containerLayout->addWidget(titleLabel);
    containerLayout->addWidget(subtitleLabel);
    containerLayout->addWidget(usernameEdit);
    containerLayout->addWidget(passwordEdit);
    containerLayout->addLayout(buttonsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addWidget(formContainer, 0, Qt::AlignHCenter);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Применяем стиль в точности как ваш темный стиль
    setStyleSheet(R"(
        QDialog {
            background-color: #2e3440;
            color: #d8dee9;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana;
        }
        #formContainer {
            background-color: #3b4252;
            border-radius: 12px;
        }
        #inputField {
            background-color: #3b4252;
            border: 1px solid #4c566a;
            border-radius: 5px;
            padding: 6px 8px;
            color: #eceff4;
            font-size: 14px;
            margin-bottom: 10px;
        }
        #inputField:focus {
            border: 1px solid #88c0d0;
            background-color: #434c5e;
            outline: none;
        }
        QPushButton#primaryButton {
            background-color: #667eea;
            color: white;
            border-radius: 14px;
            font-weight: 600;
            font-size: 16px;
            padding: 12px 28px;
            border: none;
        }
        QPushButton#primaryButton:hover {
            background-color: #556cd6;
        }
        QPushButton#primaryButton:pressed {
            background-color: #4455b2;
        }
        QPushButton#secondaryButton {
            background-color: transparent;
            border: 2.2px solid #667eea;
            color: #667eea;
            border-radius: 14px;
            font-weight: 600;
            font-size: 16px;
            padding: 12px 26px;
        }
        QPushButton#secondaryButton:hover {
            background-color: #667eea;
            color: white;
        }
        QPushButton#secondaryButton:pressed {
            background-color: #556cd6;
            border-color: #4455b2;
        }
    )");

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
