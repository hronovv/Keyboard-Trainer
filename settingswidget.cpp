#include "settingswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QScrollArea>
#include <QtCore/qabstractanimation.h>

SettingsWidget::SettingsWidget(Database &db, QString username, QWidget *parent)
    : QWidget(parent), database_(db), username_(username) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    setStyleSheet(R"(
        QWidget {
            background-color: #2e3440;
            color: #d8dee9;
            font-family: Tahoma, Geneva, Verdana;
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

    // --- Контент с настройками (прокручиваемый) ---
    QWidget *contentWidget = new QWidget;
    contentWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 30, 40, 30);
    contentLayout->setSpacing(20);

    // --- Вспомогательная лямбда для создания групп кнопок ---
    auto createOptionButtons = [&](const QString& labelText, const QString& description, const QStringList& options,
        QWidget* &widgetHolder, QList<QPushButton*> &buttonGroup, const QString& checkedStyle) -> QVBoxLayout* {
        QVBoxLayout* vlay = new QVBoxLayout();

        QLabel* headerLabel = new QLabel(labelText);
        headerLabel->setStyleSheet("font-weight: 700; font-size: 16px; margin-bottom: 3px;");
        vlay->addWidget(headerLabel);

        QLabel* descLabel = new QLabel(description);
        descLabel->setStyleSheet("font-weight: 400; font-size: 12px; color: #888;");
        descLabel->setWordWrap(true);
        descLabel->setContentsMargins(0,0,0,10);
        vlay->addWidget(descLabel);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(8);

        for (const QString& opt : options) {
            QPushButton* btn = new QPushButton(opt);
            btn->setCheckable(true);
            btn->setMinimumHeight(32);
            btn->setStyleSheet(
                "QPushButton {"
                " background-color: #3b4252;"
                " color: #d8dee9;"
                " border-radius: 6px;"
                " padding: 6px 12px;"
                "}"
                "QPushButton:hover {"
                " background-color: #5e81ac;"
                "}"
                "QPushButton:checked {"
                + checkedStyle +
                "}"
            );
            btnLayout->addWidget(btn);
            buttonGroup.append(btn);

            connect(btn, &QPushButton::clicked, this, [btn, &buttonGroup]() {
                for (auto b : buttonGroup)
                    if (b != btn) b->setChecked(false);
            });
        }

        vlay->addLayout(btnLayout);

        widgetHolder = new QWidget(this);
        widgetHolder->setLayout(vlay);
        return vlay;
    };

    // --- Настройки ---
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

    auto addLabeledWidget = [&](const QString &labelText, QWidget* widget) {
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

    // --- Кнопки caret ---
    QWidget* caretSmoothWidget = nullptr;
    caretSmoothButtons_.clear();
    createOptionButtons("Caret", "The caret will move smoothly between letters and words.",
        {"off", "slow", "medium", "fast"}, caretSmoothWidget, caretSmoothButtons_,
        "background-color: #d08770; color: #2e3440; font-weight: 700;");

    QWidget* caretStyleWidget = nullptr;
    caretStyleButtons_.clear();
    createOptionButtons("Caret style", "Change the style of the caret during the test.",
        {"off", "▮", "▯", "_"}, caretStyleWidget, caretStyleButtons_,
        "background-color: #ebcb8b; color: #2e3440; font-weight: 700;");

    // Контейнер для сворачиваемых настроек caret
    QWidget* caretSettingsContainer = new QWidget(this);
    QVBoxLayout* caretSettingsLayout = new QVBoxLayout(caretSettingsContainer);
    caretSettingsLayout->setContentsMargins(0, 0, 0, 0);
    caretSettingsLayout->setSpacing(10);
    caretSettingsLayout->addWidget(caretSmoothWidget);
    caretSettingsLayout->addWidget(caretStyleWidget);

    // Кнопка сворачивания
    QToolButton* toggleCaretButton = new QToolButton(this);
    toggleCaretButton->setText("Caret settings");
    toggleCaretButton->setCheckable(true);
    toggleCaretButton->setChecked(true);
    toggleCaretButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleCaretButton->setArrowType(Qt::DownArrow);

    connect(toggleCaretButton, &QToolButton::toggled, this, [caretSettingsContainer, toggleCaretButton](bool checked){
        caretSettingsContainer->setVisible(checked);
        toggleCaretButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    });

    // Добавляем кнопку и контейнер caret настроек в contentLayout
    contentLayout->addWidget(toggleCaretButton);
    contentLayout->addWidget(caretSettingsContainer);

    // Прокручиваемая область с настройками
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // --- Кнопки сохранения и отмены (фиксированы внизу) ---
    QWidget *buttonsWidget = new QWidget(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(40, 20, 40, 30);
    buttonsLayout->setSpacing(10);

    saveButton_ = new QPushButton("Сохранить", this);
    saveButton_->setObjectName("saveButton");
    cancelButton_ = new QPushButton("Отмена", this);
    cancelButton_->setObjectName("cancelButton");

    buttonsLayout->addWidget(saveButton_);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton_);

    // Добавляем все в главный layout
    mainLayout->addWidget(scrollArea, 1); // stretch для прокрутки
    mainLayout->addWidget(buttonsWidget, 0); // кнопки фиксированной высоты снизу

    // Подключение сигналов для кнопок
    connect(saveButton_, &QPushButton::clicked, this, [this]() {
        if (username_.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Пользователь не выбран");
            return;
        }
        saveSettings();
    });

    connect(cancelButton_, &QPushButton::clicked, this, &SettingsWidget::cancel);

    loadSettings();
}

QString SettingsWidget::getCheckedButtonValue(const QList<QPushButton*> &buttons) const {
    for (QPushButton* btn : buttons)
        if (btn->isChecked())
            return btn->text();
    return QString();
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
    for (QPushButton* btn : caretSmoothButtons_) {
        btn->setChecked(btn->text() == settings.caret_smooth);
    }
    for (QPushButton* btn : caretStyleButtons_) {
        btn->setChecked(btn->text() == settings.caret_style);
    }
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
    s.caret_smooth = getCheckedButtonValue(caretSmoothButtons_);
    s.caret_style = getCheckedButtonValue(caretStyleButtons_);
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
    success &= database_.updateUserSetting(username_, "caret_smooth", s.caret_smooth);
    success &= database_.updateUserSetting(username_, "caret_style", s.caret_style);
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