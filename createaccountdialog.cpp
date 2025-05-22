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
    resize(505, 375);

    // Градиентный фон всего окна
    setStyleSheet(R"(
        CreateAccountDialog {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea,
                stop:1 #764ba2
            );
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            color: #333;
        }
    )");

    // Основной контейнер с белым полупрозрачным фоном
    QWidget *formContainer = new QWidget(this);
    formContainer->setObjectName("formContainer");
    formContainer->setFixedSize(480, 350);

    QVBoxLayout *containerLayout = new QVBoxLayout(formContainer);
    containerLayout->setContentsMargins(35, 30, 35, 30);
    containerLayout->setSpacing(20);

    // Заголовок
    QLabel *titleLabel = new QLabel("Создание нового аккаунта", formContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #4a4a4a;");

    // Поля ввода
    usernameEdit = new QLineEdit(formContainer);
    usernameEdit->setPlaceholderText("Имя пользователя");
    usernameEdit->setObjectName("inputField");

    passwordEdit = new QLineEdit(formContainer);
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setObjectName("inputField");

    confirmPasswordEdit = new QLineEdit(formContainer);
    confirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setObjectName("inputField");

    // Иконка "глазик" для показа/скрытия пароля (passwordEdit)
    QIcon eyeOpenedIcon(QStringLiteral("/Users/hronov/Documents/Keyboard Trainer/icons/opened-eye.svg"));
    QIcon eyeClosedIcon(QStringLiteral("/Users/hronov/Documents/Keyboard Trainer/icons/closed-eye.svg"));

    QAction *togglePasswordAction = passwordEdit->addAction(eyeClosedIcon, QLineEdit::TrailingPosition);
    togglePasswordAction->setCheckable(true);

    connect(togglePasswordAction, &QAction::toggled, this, [=](bool checked){
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        togglePasswordAction->setIcon(checked ? eyeOpenedIcon : eyeClosedIcon);
    });

    // Иконка "глазик" для confirmPasswordEdit
    QAction *toggleConfirmPasswordAction = confirmPasswordEdit->addAction(eyeClosedIcon, QLineEdit::TrailingPosition);
    toggleConfirmPasswordAction->setCheckable(true);

    connect(toggleConfirmPasswordAction, &QAction::toggled, this, [=](bool checked){
        confirmPasswordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        toggleConfirmPasswordAction->setIcon(checked ? eyeOpenedIcon : eyeClosedIcon);
    });

    // Кнопка создания аккаунта
    QPushButton *createAccountButton = new QPushButton("Создать аккаунт", formContainer);
    createAccountButton->setObjectName("primaryButton");
    createAccountButton->setDefault(true);
    createAccountButton->setCursor(Qt::PointingHandCursor);

    // Собираем layout
    containerLayout->addWidget(titleLabel);
    containerLayout->addWidget(usernameEdit);
    containerLayout->addWidget(passwordEdit);
    containerLayout->addWidget(confirmPasswordEdit);
    containerLayout->addWidget(createAccountButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addWidget(formContainer, 0, Qt::AlignHCenter);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0,0,0,0);

    // Общие стили для элементов во всём окне (перекрытие setStyleSheet выше)
    setStyleSheet(R"(
        #formContainer {
            background: rgba(255, 255, 255, 0.94);
            border-radius: 20px;

        }

        #inputField {
            border: 1.8px solid #ccc;
            border-radius: 12px;
            padding: 12px 15px;
            font-size: 15px;
            background: white;
            color: #222;

        }
        #inputField:focus {
            border-color: #667eea;
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
    )");

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
