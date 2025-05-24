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

    // Основной контейнер с прозрачным тёмным фоном
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
    titleLabel->setStyleSheet("color: #d8dee9;");

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

    // Иконка для показа/скрытия пароля
    QIcon eyeOpenedIcon(QStringLiteral("/Users/hronov/Documents/Keyboard Trainer/icons/opened-eye.svg"));
    QIcon eyeClosedIcon(QStringLiteral("/Users/hronov/Documents/Keyboard Trainer/icons/closed-eye.svg"));

    QAction *togglePasswordAction = passwordEdit->addAction(eyeClosedIcon, QLineEdit::TrailingPosition);
    togglePasswordAction->setCheckable(true);
    connect(togglePasswordAction, &QAction::toggled, this, [=](bool checked){
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        togglePasswordAction->setIcon(checked ? eyeOpenedIcon : eyeClosedIcon);
    });

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

    // Добавляем виджеты в layout
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

    // Применяем стиль с тёмной темой, как в вашем примере
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
            background-color: #434c5e;
            border: 1px solid #4c566a;
            border-radius: 5px;
            padding: 6px 8px;
            color: #eceff4;
            font-size: 14px;
            margin-bottom: 10px;
        }
        #inputField:focus {
            border: 1px solid #88c0d0;
            background-color: #4c566a;
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