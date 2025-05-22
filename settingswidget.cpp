#include "settingswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QtCore/qabstractanimation.h>

SettingsWidget::SettingsWidget(Database &db, QString username, QWidget *parent)
    : QWidget(parent), database_(db), username_(username) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    setStyleSheet(R"(
        QWidget {
            background-color: #2e3440;
            color: #d8dee9;
            font-family: Tahoma, Geneva, Verdana, sans-serif;
            font-size: 14px;
        }
        QLabel {
            font-weight: 600;
            min-width: 200px;
            color: #d8dee9;
        }
        QSpinBox, QFontComboBox {
            background-color: #3b4252;
            border: 1px solid #4c566a;
            border-radius: 5px;
            padding: 6px 8px;
            color: #eceff4;
            font-size: 14px;
            min-width: 150px;
            min-height: 36px;
        }
        QSpinBox::up-button, QSpinBox::down-button {
            width: 24px;
            height: 24px;
        }
        QPushButton {
            background-color: #81a1c1;
            color: #2e3440;
            font-weight: 700;
            font-size: 15px;
            min-width: 120px;
            min-height: 42px;
            border: none;
            border-radius: 8px;
            padding: 8px 15px;
            margin: 10px 10px 10px 0;
        }
        QPushButton:hover {
            background-color: #5e81ac;
        }
        QPushButton:pressed {
            background-color: #4c699a;
        }
        QPushButton#cancelButton {
            background-color: #bf616a;
            color: #eceff4;
        }
        QPushButton#cancelButton:hover {
            background-color: #a14c54;
        }
        QPushButton#cancelButton:pressed {
            background-color: #843d41;
        }
        QPushButton#colorButton {
            min-width: 150px;
            background-color: #88c0d0;
            color: #2e3440;
        }
        QPushButton#colorButton:hover {
            background-color: #81a1c1;
        }
        QPushButton#colorButton:pressed {
            background-color: #6793a8;
        }
        QScrollArea {
            background-color: transparent;
        }
        QScrollBar:vertical {
            background: #3b4252;
            width: 10px;
            margin: 15px 0 15px 0;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #81a1c1;
            min-height: 30px;
            border-radius: 5px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    QWidget *contentWidget = new QWidget;
    contentWidget->setStyleSheet("background-color: transparent;");
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
            colorButton_->setStyleSheet(QString("background-color: %1; color: #2e3440; font-weight: 700;")
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

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

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
    QPropertyAnimation *anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        this->hide();
        this->setWindowOpacity(1.0); // Чтобы при следующем показе непрозрачность была нормальной
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);

}

void SettingsWidget::setUsername(const QString& username) {
    username_ = username;
}