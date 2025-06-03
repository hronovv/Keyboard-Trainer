#ifndef PROMOSETTINGSDIALOG_H
#define PROMOSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

class PromoSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit PromoSettingsDialog(QWidget *parent = nullptr)
        : QDialog(parent) {
        setWindowTitle("Настройки генерации текста");
        setModal(true);
        setFixedSize(480, 350);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(25, 20, 25, 20);
        mainLayout->setSpacing(20);


        QLabel* themeLabel = new QLabel("Тематика:", this);
        themeLabel->setStyleSheet("color: #d8dee9; font-size: 16px; font-weight: 600;");
        mainLayout->addWidget(themeLabel);

        themeEdit = new QLineEdit(this);
        themeEdit->setPlaceholderText("Например: программирование");
        themeEdit->setStyleSheet(R"(
            QLineEdit {
                background-color: #3b4252;
                border: 1.5px solid #4c566a;
                border-radius: 8px;
                padding: 10px 14px;
                color: #eceff4;
                font-size: 14px;
            }
            QLineEdit:focus {
                border-color: #88c0d0;
                background-color: #434c5e;
            }
        )");
        themeEdit->setMinimumHeight(40);
        mainLayout->addWidget(themeEdit);


        QLabel* countLabel = new QLabel("Количество слов(примерно):", this);
        countLabel->setStyleSheet("color: #d8dee9; font-size: 16px; font-weight: 600;");
        mainLayout->addWidget(countLabel);

        wordCountSpin = new QSpinBox(this);
        wordCountSpin->setRange(5, 1000);
        wordCountSpin->setValue(100);
        wordCountSpin->setStyleSheet(R"(
            QSpinBox {
                background-color: #3b4252;
                border: 1.5px solid #4c566a;
                border-radius: 8px;
                padding: 10px 14px;
                color: #eceff4;
                font-size: 14px;
                qproperty-alignment: AlignCenter;
            }
            QSpinBox:focus {
                border-color: #88c0d0;
                background-color: #434c5e;
            }
        )");
        wordCountSpin->setMinimumHeight(40);
        mainLayout->addWidget(wordCountSpin);


        QHBoxLayout* buttonsLayout = new QHBoxLayout();
        buttonsLayout->setContentsMargins(0, 20, 0, 0);
        buttonsLayout->setSpacing(25);

        QPushButton* okBtn = new QPushButton("OK", this);
        okBtn->setDefault(true);
        okBtn->setCursor(Qt::PointingHandCursor);
        okBtn->setFixedHeight(48);
        okBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #667eea;
                color: white;
                border-radius: 14px;
                font-weight: bold;
                font-size: 16px;
                padding: 6px 20px;
                border: none;
            }
            QPushButton:hover {
                background-color: #556cd6;
            }
            QPushButton:pressed {
                background-color: #4455b2;
            }
        )");

        QPushButton* cancelBtn = new QPushButton("Отмена", this);
        cancelBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setFixedHeight(48);
        cancelBtn->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                border: 2px solid #667eea;
                color: #667eea;
                border-radius: 14px;
                font-weight: normal;
                font-size: 14px;
                padding: 6px 20px;
            }
            QPushButton:hover {
                background-color: #667eea;
                color: white;
            }
            QPushButton:pressed {
                background-color: #556cd6;
                border-color: #4455b2;
            }
        )");

        buttonsLayout->addStretch();
        buttonsLayout->addWidget(okBtn);
        buttonsLayout->addWidget(cancelBtn);
        buttonsLayout->addStretch();

        mainLayout->addLayout(buttonsLayout);

        connect(okBtn, &QPushButton::clicked, this, &PromoSettingsDialog::onOkClicked);
        connect(cancelBtn, &QPushButton::clicked, this, &PromoSettingsDialog::reject);
    }

    QString getTheme() const { return themeEdit->text().trimmed(); }
    int getWordCount() const { return wordCountSpin->value(); }

private slots:
    void onOkClicked() {
        if (getTheme().isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Пожалуйста, укажите тематику.");
            themeEdit->setFocus();
            return;
        }
        accept();
    }

private:
    QLineEdit* themeEdit;
    QSpinBox* wordCountSpin;
};

#endif // PROMOSETTINGSDIALOG_H