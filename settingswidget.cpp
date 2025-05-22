#include "settingswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QMessageBox>
#include <QScrollArea>  // добавляем

SettingsWidget::SettingsWidget(Database &db, QString username, QWidget *parent)
    : QWidget(parent), database_(db), username_(username) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setStyleSheet(R"(
        QWidget {
            background-color: #2E2E2E;
            color: #F0F0F0;
            font-family: "Arial Black";
            font-size: 16px;
        }
        QLabel {
            font-weight: 600;
            min-width: 200px;
        }
        QSpinBox, QFontComboBox {
            font-size: 16px;
            min-width: 150px;
            min-height: 36px;
            padding: 6px 8px;
            border: 1px solid #555;
            border-radius: 5px;
            background-color: #454545;
            color: #F0F0F0;
        }
        QSpinBox::up-button, QSpinBox::down-button {
            width: 24px;
            height: 24px;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            font-weight: 700;
            font-size: 17px;
            min-width: 120px;
            min-height: 42px;
            border: none;
            border-radius: 8px;
            padding: 8px 15px;
            margin: 10px 10px 10px 0;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3e8e41;
        }
        QPushButton#cancelButton {
            background-color: #d9534f;
        }
        QPushButton#cancelButton:hover {
            background-color: #c9302c;
        }
        QPushButton#cancelButton:pressed {
            background-color: #ac2925;
        }
        QPushButton#colorButton {
            min-width: 150px;
            background-color: #3a87ad;
        }
        QPushButton#colorButton:hover {
            background-color: #337ab7;
        }
        QPushButton#colorButton:pressed {
            background-color: #286090;
        }
    )");

    // УБРАТЬ setFixedSize(1600, 1600); - чтобы окно могло менять размер

    // Создаем основной layout для виджета с прокруткой (scroll area)
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // Создаем виджет контейнер для содержимого
    QWidget *contentWidget = new QWidget;
    contentWidget->setStyleSheet("background-color: transparent;"); // на всякий случай
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 30, 40, 30);
    contentLayout->setSpacing(20);

    letterSpacingSpinBox_ = new QSpinBox(this);
    letterSpacingSpinBox_->setRange(0, 20);
    wordSpacingSpinBox_ = new QSpinBox(this);
    wordSpacingSpinBox_->setRange(0, 50);

    fontComboBox_ = new QFontComboBox(this);
    fontComboBox_->setMinimumWidth(250);

    fontSizeSpinBox_ = new QSpinBox(this);
    fontSizeSpinBox_->setRange(8, 72);

    fontWeightSpinBox_ = new QSpinBox(this);
    fontWeightSpinBox_->setRange(500, 900);
    fontWeightSpinBox_->setSingleStep(100);

    lineHeightSpinBox_ = new QSpinBox(this);
    lineHeightSpinBox_->setRange(10, 100);

    colorButton_ = new QPushButton("Выбрать цвет", this);
    colorButton_->setObjectName("colorButton");

    connect(colorButton_, &QPushButton::clicked, this, [this]() {
        QColor chosen = QColorDialog::getColor(currentColor_, this, "Выберите цвет текста");
        if (chosen.isValid()) {
            currentColor_ = chosen;
            colorButton_->setStyleSheet(QString("background-color: %1; color: white; font-weight: 700;")
                .arg(chosen.name()));
        }
    });

    saveButton_ = new QPushButton("Сохранить", this);
    saveButton_->setObjectName("saveButton");

    cancelButton_ = new QPushButton("Отмена", this);
    cancelButton_->setObjectName("cancelButton");

    connect(saveButton_, &QPushButton::clicked, this, &SettingsWidget::saveSettings);
    connect(cancelButton_, &QPushButton::clicked, this, &SettingsWidget::cancel);

    auto addLabeledWidget = [&](const QString &labelText, QWidget* widget){
        QHBoxLayout *hlayout = new QHBoxLayout();
        QLabel *label = new QLabel(labelText);
        label->setMinimumWidth(220);
        hlayout->addWidget(label);
        hlayout->addWidget(widget);
        contentLayout->addLayout(hlayout);
    };

    addLabeledWidget("Отступ между буквами:", letterSpacingSpinBox_);
    addLabeledWidget("Отступ между словами:", wordSpacingSpinBox_);
    addLabeledWidget("Шрифт:", fontComboBox_);
    addLabeledWidget("Размер шрифта:", fontSizeSpinBox_);
    addLabeledWidget("Жирность шрифта:", fontWeightSpinBox_);
    addLabeledWidget("Межстрочный интервал:", lineHeightSpinBox_);
    addLabeledWidget("Цвет текста:", colorButton_);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(saveButton_);
    buttonsLayout->addWidget(cancelButton_);
    contentLayout->addLayout(buttonsLayout);

    contentLayout->addStretch();

    // Создаём QScrollArea и кладём в неё наш contentWidget с настройками
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);

    mainLayout->addWidget(scrollArea);

    loadSettings();
}

void SettingsWidget::applySettingsToUI(const UserSettings &settings) {
    letterSpacingSpinBox_->setValue(settings.letter_spacing);
    wordSpacingSpinBox_->setValue(settings.word_spacing);
    fontComboBox_->setCurrentFont(QFont(settings.font));
    fontSizeSpinBox_->setValue(settings.font_size);
    fontWeightSpinBox_->setValue(settings.font_weight);
    lineHeightSpinBox_->setValue(settings.line_height);
    currentColor_ = settings.font_color;
    colorButton_->setStyleSheet(QString("background-color: %1").arg(currentColor_.name()));
}

UserSettings SettingsWidget::getSettingsFromUI() const {
    UserSettings s;
    s.letter_spacing = letterSpacingSpinBox_->value();
    s.word_spacing = wordSpacingSpinBox_->value();
    s.font = fontComboBox_->currentFont().family();
    s.font_size = fontSizeSpinBox_->value();
    s.font_weight = fontWeightSpinBox_->value();
    s.line_height = lineHeightSpinBox_->value();
    s.font_color = currentColor_;
    return s;
}

void SettingsWidget::loadSettings() {
    if (username_.isEmpty()) return;
    UserSettings s = database_.getUserSettings(username_);
    applySettingsToUI(s);
}

void SettingsWidget::saveSettings() {
    UserSettings s = getSettingsFromUI();
    if (username_.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пользователь не выбран");
        return;
    }
    bool success = true;
    success &= database_.updateUserSetting(username_, "font", s.font);
    success &= database_.updateUserSetting(username_, "font_color", s.font_color.name());
    success &= database_.updateUserSetting(username_, "font_size", s.font_size);
    success &= database_.updateUserSetting(username_, "letter_spacing", s.letter_spacing);
    success &= database_.updateUserSetting(username_, "word_spacing", s.word_spacing);
    success &= database_.updateUserSetting(username_, "font_weight", s.font_weight);
    success &= database_.updateUserSetting(username_, "line_height", s.line_height);
    if (!success) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить настройки");
        return;
    }
    emit settingsChanged(s);
    this->hide();
}

void SettingsWidget::cancel() {
    this->hide();
}

void SettingsWidget::setUsername(const QString& username) {
    username_ = username;
}