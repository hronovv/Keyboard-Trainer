#include "window.h"

Window::Window(Database &db, QWidget *parent)
    : QWidget(parent), database_(db) {


    settingsWidget_ = new SettingsWidget(database_, currentUsername_, this);
    settingsWidget_->hide();

    connect(settingsWidget_, &SettingsWidget::settingsChanged, this, [this](const UserSettings &s){
        currentFont_ = QFont(s.font);
        currentFont_.setPointSize(s.font_size);
        currentFont_.setWeight(QFont::Weight(s.font_weight));

        textColor_ = s.font_color;
        fontSize_ = s.font_size;
        letterSpacing_ = s.letter_spacing;
        wordSpacing_ = s.word_spacing;
        fontWeight_ = s.font_weight;
        lineHeight_ = s.line_height;
        caretStyle_ = s.caret_style;

        ApplyTextStyles();
    });


    typing_timer_ = new QTimer(this);
    typing_timer_->setInterval(kIntervalMs);
    connect(typing_timer_, &QTimer::timeout, this, &Window::UpdateWPM);
    elapsed_seconds_ = 0;

    countdownTimer_ = new QTimer(this);
    countdownTimer_->setInterval(1000);

    connect(countdownTimer_, &QTimer::timeout, this, [this]() {
        if (timeRemaining_ > 0) {
        timeRemaining_--;
        timeLabel_->setText(QString("Осталось времени: %1 с").arg(timeRemaining_));
        } else {
            countdownTimer_->stop();
            typing_allowed_ = false;
            StopTypingTimer();
            timeLabel_->setText("Осталось времени: 0 с");
            timeRemaining_ = 1;
            QMessageBox::information(this, "Время вышло", "Время теста закончилось.");
            keyboardWidget_->clearHighlight();
            timeLabel_->setText("");
        }
    });


    generated_text_ = new QLabel(this);
    generated_text_->setObjectName("generatedText");
    generated_text_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    generated_text_->setWordWrap(true);
    generated_text_->setFixedWidth(kTextFieldWidth);
    generated_text_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    generated_text_->setMinimumHeight(kTextFieldMinimumHeigth);

    statusLabel_ = new QLabel("RAW WPM: 0 | Точность: 100% | WPM: 0", this);
    statusLabel_->setObjectName("statusLabel");
    statusLabel_->setAlignment(Qt::AlignBottom | Qt::AlignCenter);

    timeLabel_ = new QLabel(this);
    timeLabel_->setObjectName("timeLabel");
    timeLabel_->setAlignment(Qt::AlignBottom | Qt::AlignCenter);


    keyboardWidget_ = new KeyboardWidget(this);
    connect(keyboardWidget_, &KeyboardWidget::spaceKeyClicked, this, &Window::ShowKeyboardLayoutDialog);
    QString keyboardLayoutPath = "/Users/hronov/Documents/Keyboard Trainer/keyboard_layouts/qwerty.json";
    if (!keyboardWidget_->loadLayoutFromJson(keyboardLayoutPath)) {
        qWarning() << "Failed to load keyboard layout from json";
    }


    auto settings_button = new QPushButton(this);
    settings_button->setStyleSheet("border: none; background: transparent;");
    settings_button->setIcon(QIcon("/Users/hronov/Documents/Keyboard Trainer/icons/settings.svg"));
    settings_button->setIconSize(QSize(46, 46));
    settings_button->setFlat(true);
    settings_button->setCursor(Qt::PointingHandCursor);
    connect(settings_button, &QPushButton::clicked, this, &Window::ShowSettings);

    auto login_button = new QPushButton(this);
    login_button->setStyleSheet("border: none; background: transparent;");
    login_button->setIcon(QIcon("/Users/hronov/Documents/Keyboard Trainer/icons/account.svg"));
    login_button->setIconSize(QSize(44, 45));
    login_button->setFlat(true);
    login_button->setCursor(Qt::PointingHandCursor);
    connect(login_button, &QPushButton::clicked, this, &Window::showLoginDialog);


    auto bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(settings_button);
    bottomLayout->addSpacing(100);
    bottomLayout->addWidget(login_button);

    usernameLabel_ = new QLabel("", this);
    usernameLabel_->setStyleSheet("font-size: 18px; color: #d8dee9; letter-spacing: 2px; word-spacing: 2px;");
    bottomLayout->addWidget(usernameLabel_);
    bottomLayout->setAlignment(Qt::AlignCenter);


    QWidget *categoryWidget = new QWidget(this);
    categoryWidget->setObjectName("categoryWidget");
    QHBoxLayout *categoryLayout = new QHBoxLayout(categoryWidget);
    categoryLayout->setContentsMargins(10, 10, 10, 10);
    categoryLayout->setSpacing(20);
    categoryLayout->setAlignment(Qt::AlignCenter);

    auto addCategoryWidget = [&](const QString &text, bool isButton = false) {
        if (!isButton) {
            QLabel *label = new QLabel(text, categoryWidget);
            label->setStyleSheet(R"(
                color: #eee;
                font-size: 14px;
                padding: 5px 12px;
                background-color: rgba(245, 245, 245, 0.15);
                border-radius: 8px;
                font-weight: 600;
                qproperty-alignment: AlignCenter;
            )");
            categoryLayout->addWidget(label);
        } else {
            QPushButton *button = new QPushButton(text, categoryWidget);
            button->setFlat(true);
            button->setCursor(Qt::PointingHandCursor);
            if (text == "numbers") {
                button->setCheckable(true);
                connect(button, &QPushButton::toggled, this, [this, button](bool checked){
                    if (!currentWordList_.isEmpty()) {
                        numbersModeActive_ = checked;
                        GenerateNewTextFromWordList();
                    } else {
                        QMessageBox::warning(this, "Ошибка", "Сначала выберите список слов.");
                        if (checked) {
                            button->blockSignals(true);
                            button->setChecked(false);
                            button->blockSignals(false);
                        }
                    }
                });
            } else if (text == "punctuation") {
                button->setCheckable(true);
                connect(button, &QPushButton::toggled, this, [this, button](bool checked){
                    if (!currentWordList_.isEmpty()) {
                        punctuationModeActive_ = checked;
                        GenerateNewTextFromWordList();
                    } else {
                        QMessageBox::warning(this, "Ошибка", "Сначала выберите список слов.");
                        if (checked) {
                            button->blockSignals(true);
                            button->setChecked(false);
                            button->blockSignals(false);
                        }
                    }
                });
            } else if (text == "time") {
                button->setCheckable(true);
                connect(button, &QPushButton::toggled, this, [this, button](bool checked){
                    if (checked) {
                        if (currentWordList_.isEmpty()) {
                            QMessageBox::warning(this, "Ошибка", "Сначала выберите список слов.");
                            button->blockSignals(true);
                            button->setChecked(false);
                            button->blockSignals(false);
                            return;
                        }

                        wordsModeActive_ = false;
                        timeModeActive_ = true;
                        ShowTimeSelectorDialog();
                        timeRemaining_ = selectedTimeSeconds_;
                        GenerateNewTextFromWordList();
                    } else {
                        timeModeActive_ = false;
                        countdownTimer_->stop();
                        typing_allowed_ = false;
                        generated_text_->clear();
                        GenerateNewTextFromWordList();
                    }
                });
            }
            button->setStyleSheet(R"(
            QPushButton {
                color: #eee;
                font-size: 14px;
                padding: 5px 12px;
                background-color: rgba(245, 245, 245, 0.15);
                border-radius: 8px;
                font-weight: 600;
                border: none;
                text-align: center;
            }
            QPushButton:checked {
                background-color: #88c0d0;
                color: #2e3440;
                font-weight: 800;
            }
        )");
            categoryLayout->addWidget(button);

            QMap<QString, std::function<void()>> actions = {
                { "words", [this]() { ShowWordSetDialog(); } },
                { "ai", [this]() { Prompt(); } },
                { "language", [this]() { ShowLanguageDialog(); } },
                { "stop", [this]() { DisableTyping(); } },
                { "stats", [this]() { ShowStats(); } },
                { "quote", [this]() { random(); } },
                { "amount", [this]() { ShowAmountSelectorDialog(); }},
            };

            connect(button, &QPushButton::clicked, this, [this, text, actions]() {
                auto it = actions.find(text);
                if (it != actions.end()) {
                    it.value()();
                }
            });
        }
    };


    addCategoryWidget("ai", true);
    addCategoryWidget("language", true);
    addCategoryWidget("punctuation",true);
    addCategoryWidget("numbers",true);
    addCategoryWidget("time", true);
    addCategoryWidget("words", true);
    addCategoryWidget("quote",true);
    addCategoryWidget("amount", true);
    addCategoryWidget("stats", true);

    categoryWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QHBoxLayout *categoryWrapperLayout = new QHBoxLayout();
    categoryWrapperLayout->addStretch();
    categoryWrapperLayout->addWidget(categoryWidget);
    categoryWrapperLayout->addStretch();


    auto main_layout = new QVBoxLayout(this);
    main_layout->addLayout(bottomLayout);
    main_layout->addLayout(categoryWrapperLayout);
    main_layout->addSpacing(50);
    main_layout->addWidget(generated_text_, 0, Qt::AlignHCenter);
    main_layout->addWidget(keyboardWidget_, 0, Qt::AlignHCenter);
    main_layout->addSpacing(20);
    main_layout->addWidget(timeLabel_, 0, Qt::AlignHCenter);
    main_layout->addWidget(statusLabel_, 0, Qt::AlignHCenter);
    main_layout->addStretch();
    main_layout->setContentsMargins(10, 10, 10, 20);
    main_layout->setAlignment(Qt::AlignTop);


    setLayout(main_layout);


    QString globalStyle = R"(
        QWidget {
            background-color: #3b4252;
            color: #d8dee9;
            font-family: Tahoma, Geneva, Verdana;
        }
        QPushButton {
            background-color: #3b4252;
            color: #d8dee9;
            border: 1px solid #4c566a;
            border-radius: 5px;
            padding: 6px 12px;
            font-size: 16px;
            margin: 2px;
        }
        QPushButton:hover {
            background-color: #434c5e;
            border: 1px solid #88c0d0;
            color: #eceff4;
        }
        QPushButton:pressed {
            background-color: #81a1c1;
            color: #2e3440;
        }
        QLabel {
            color: #d8dee9;
            background: transparent
        }
        QLabel#statusLabel {
            font-size: 20px;
            letter-spacing: 2px;
            word-spacing: 2px;
        }
        QLabel#timeLabel {
            font-size: 20px;
            letter-spacing: 2px;
            word-spacing: 2px;
        }
        QLabel#generatedText {
            font-size: 16px;
            letter-spacing: 2px;
            word-spacing: 2px;
            font-weight: 500;
            color: #eceff4;
        }
        QWidget#categoryWidget {
            background-color: rgba(255, 255, 255, 0.15);
            border-radius: 10px;
            padding-left: 15px;
            padding-right: 15px;
        }
        QCheckBox {
        background-color: transparent;
        color: #d8dee9;
        font-size: 14px;
        }
    )";
    setStyleSheet(globalStyle);
}



void Window::SetLanguage(const QString& language) {
    prompt_language_ = language;
}

void Window::Prompt() {
    if (prompt_language_.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите язык для генерации.");
        return;
    }

    try {
        typing_allowed_ = false;

        QString request = QString::fromStdString(
            kPromptTemplatePart1
            + std::to_string(kWordsNumber)
            + kPromptTemplatePart2
            + "IMPORTANT. set-language:" + prompt_language_.toStdString()
        );

        QString result_final = QString::fromStdString(getResponse(request.toStdString()));
        generated_text_->setText(result_final);
        ResetText();

        effect_ = new QGraphicsOpacityEffect(this);
        generated_text_->setGraphicsEffect(effect_);

        animation_ = new QPropertyAnimation(effect_, "opacity");
        animation_->setDuration(kAnimationDurationMs);
        animation_->setStartValue(0.0);
        animation_->setEndValue(1.0);
        animation_->start(QAbstractAnimation::DeleteWhenStopped);

        wordsModeActive_ = false;
        typing_allowed_ = true;

    } catch (const nlohmann::json::type_error&) {
        QMessageBox::warning(this, "Ошибка", "Ошибка обработки JSON-ответа. Попробуйте включить VPN.");
    }
}

void Window::ShowLanguageDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Выберите язык");
    dialog.setModal(true);
    dialog.setFixedSize(kLanguageChoiceWidth, kLanguageChoiceHeight);

    QVBoxLayout layout(&dialog);
    QListWidget list_widget(&dialog);

    for (const auto& lang : kLanguages) {
        list_widget.addItem(QString::fromStdString(lang));
    }

    connect(&list_widget, &QListWidget::itemClicked, this, [&dialog, this](QListWidgetItem* item) {
        SetLanguage(item->text());
        dialog.accept();
    });

    layout.addWidget(&list_widget);

    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);

    dialog.exec();
}

void Window::DisableTyping() {
    if (typing_allowed_) {
        typing_allowed_ = false;
        generated_text_->clear();
        ResetText();
    }
}

void Window::ResetText() {
    targetText_ = generated_text_->text();

    typedChars_.fill('|', targetText_.length());
    errorFlags_.fill(false, targetText_.length());

    currentIndex_ = 0;
    errorCount_ = 0;
    typedCharCount_ = 0;

    statusLabel_->setText("RAW WPM: 0 | Точность: 100% | WPM: 0");

    StopTypingTimer();
}

void Window::StartTypingTimer() {
    if (!typing_timer_->isActive()) {
        elapsed_seconds_ = 0;
        typing_timer_->start();
    }
}

void Window::StopTypingTimer() {
    typing_timer_->stop();
}

void Window::UpdateWPM() {
    elapsed_seconds_ += kUpdateIntervalSec;
    const double minutes = elapsed_seconds_ / kSecondsInMinute;

    if (minutes > 0) {
        double raw_wpm = (static_cast<double>(typedCharCount_) / kWpmCoefficient) / minutes;
        double accuracy = kHundred - ((static_cast<double>(errorCount_) / targetText_.length()) * kHundred);
        accuracy = qMax(accuracy, 0.0);

        statusLabel_->setText(
            QString("RAW WPM: %1 | Точность: %2% | WPM: %3")
            .arg(QString::number(raw_wpm, 'f', 2))
            .arg(QString::number(accuracy, 'f', 2))
            .arg(QString::number(raw_wpm * accuracy / kHundred, 'f', 2))
        );
    }
}

void Window::keyPressEvent(QKeyEvent* event) {
    if (!typing_allowed_) {
        return;
    }

    if (timeModeActive_ && !countdownTimer_->isActive()) {
        StartCountdownTimer();
    }

    if (event->key() == Qt::Key_Tab) {
        event->accept();
        keyboardWidget_->clearHighlight();
        if (timeModeActive_) {
            StopTypingTimer();
            countdownTimer_->stop();
            timeRemaining_ = selectedTimeSeconds_;
            typing_allowed_ = false;
            timeLabel_->setText(QString("Осталось времени: %1 с").arg(timeRemaining_));
        }
        GenerateNewTextFromWordList();
        return;
    }

    typedCharCount_ = 0;

    if (event->key() == Qt::Key_Backspace) {
        if (currentIndex_ > 0) {
            --currentIndex_;
            typedChars_[currentIndex_] = '|';
            errorFlags_[currentIndex_] = false;
            typedCharCount_--;
            if (currentIndex_ < targetText_.length()) {
                QChar nextChar = targetText_.at(currentIndex_);
                keyboardWidget_->highlightKey(nextChar);
            } else {
                keyboardWidget_->clearHighlight();
            }
        }
    } else {
        const QString new_text = event->text();
        if (!new_text.isEmpty() && currentIndex_ < targetText_.length()) {
            if (currentIndex_ == 0)
                StartTypingTimer();

            const QChar expected_char = targetText_.at(currentIndex_);
            const QChar typed_char = new_text.at(0);

            if (typed_char == expected_char) {
                errorFlags_[currentIndex_] = false;
            } else {
                errorFlags_[currentIndex_] = true;
                errorCount_++;
            }

            typedChars_[currentIndex_] = typed_char;
            currentIndex_++;
            if (currentIndex_ < targetText_.length()) {
                QChar nextChar = targetText_.at(currentIndex_);
                keyboardWidget_->highlightKey(nextChar);
            } else {
                keyboardWidget_->clearHighlight();
            }
        }
    }

    QString colored_text;

    for (int i = 0; i < targetText_.length(); ++i) {
        if (typedChars_[i] != '|') {
            QString color = errorFlags_[i] ? "red" : "green";
            QChar target_char = targetText_.at(i);

            if (target_char == ' ' && errorFlags_[i]) {
                colored_text += "<span style='text-decoration: underline; color: red;'> </span>";
            } else {
                colored_text += "<span style='color:" + color + ";'>" + QString(target_char) + "</span>";
            }
            typedCharCount_++;
        } else {
            if (i == currentIndex_) {
                if (caretStyle_ == "off") {
                    colored_text += "<span style='color:" + textColor_.name() + ";'>" + QString(targetText_.at(i)) +
                        "</span>";
                } else if (caretStyle_ == "_") {
                    colored_text += "<span style='text-decoration: underline; color: " + textColor_.name() + ";'>" +
                                    QString(targetText_.at(i)) +
                                    "</span>";
                } else if (caretStyle_ == "▮") {
                    colored_text += "<span style='background-color: rgba(0,0,0,0.4); color:" +
            textColor_.name() + "'>" + QString(targetText_.at(i)) + "</span>";
                }
            } else {
                colored_text += "<span style='color:" + textColor_.name() + ";'>" + QString(targetText_.at(i))
                 + "</span>";
            }
        }
    }

    generated_text_->setText(colored_text);

    if (currentIndex_ == targetText_.length() || timeRemaining_ <= 0) {
        typing_allowed_ = false;
        double minutes = elapsed_seconds_ / kSecondsInMinute;
        double raw_wpm = (typedCharCount_ / kWpmCoefficient) / minutes;
        double accuracy = kHundred - ((double)errorCount_ / targetText_.length() * kHundred);
        accuracy = qMax(accuracy, 0.0);


        if (!currentUsername_.isEmpty()) {
            database_.saveTypingSession(currentUsername_, raw_wpm * accuracy / kHundred, accuracy);
        }

        StopTypingTimer();
        keyboardWidget_->clearHighlight();
    }
}

void Window::ApplyTextStyles() {
    generated_text_->setStyleSheet(QString(
        "font-family: '%7'; "
        "font-size: %1px; "
        "color: %2; "
        "font-weight: %3; "
        "letter-spacing: %4px; "
        "word-spacing: %5px; "
        "line-height: %6px;")
        .arg(fontSize_)
        .arg(textColor_.name())
        .arg(fontWeight_)
        .arg(letterSpacing_)
        .arg(wordSpacing_)
        .arg(lineHeight_)
        .arg(currentFont_.family())
    );
}


void Window::showLoginDialog() {
    if (!database_.initDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        return;
    }

    LoginDialog dialog(database_, this);
    if (dialog.exec() == QDialog::Accepted && dialog.isLoggedIn()) {
        currentUsername_ = dialog.getUsername();
        QMessageBox::information(this, "Успех", "Вы вошли как: " + currentUsername_);
        settingsWidget_->setUsername(currentUsername_);
        LoadUserSettings();
    }
}

void Window::LoadUserSettings() {
    if (currentUsername_.isEmpty())
        return;

    UserSettings settings = database_.getUserSettings(currentUsername_);

    currentFont_ = QFont(settings.font);
    currentFont_.setPointSize(settings.font_size);
    currentFont_.setWeight(QFont::Weight(settings.font_weight));

    textColor_ = settings.font_color;
    fontSize_ = settings.font_size;
    letterSpacing_ = settings.letter_spacing;
    wordSpacing_ = settings.word_spacing;
    fontWeight_ = settings.font_weight;
    lineHeight_ = settings.line_height;
    caretStyle_ = settings.caret_style;
    currentWordAmount_= settings.words_amount;
    keyboardWidget_->loadLayoutFromJson("/Users/hronov/Documents/Keyboard Trainer/keyboard_layouts/" +
        settings.keyboard_layout + ".json");


    ApplyTextStyles();

    usernameLabel_->setText(currentUsername_);
}

void Window::ShowSettings() {
    if (currentUsername_.isEmpty()) {
        QMessageBox::information(this, "Инфо", "Сначала войдите в систему");
        return;
    }

    settingsWidget_->loadSettings();

    QRect screenGeometry;
    if (settingsWidget_->parentWidget()) {
        screenGeometry = settingsWidget_->parentWidget()->screen()->geometry();
    } else {
        screenGeometry = QGuiApplication::primaryScreen()->geometry();
    }

    settingsWidget_->setGeometry(screenGeometry);
    settingsWidget_->show();

    QPropertyAnimation *anim = new QPropertyAnimation(settingsWidget_, "windowOpacity");
    settingsWidget_->setWindowOpacity(0);
    anim->setDuration(300);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void Window::ShowWordSetDialog() {
    QWidget *overlay = new QWidget(this);
    overlay->setObjectName("overlayWidget");
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
    overlay->show();

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(overlay);
    blur->setBlurRadius(40);
    this->setGraphicsEffect(blur);

    QString languagesPath = "/Users/hronov/Documents/Keyboard Trainer/languages";
    QDir dir(languagesPath);
    QStringList filters {"*.json"};
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks);

    if (fileList.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Папка languages пуста или не найдены JSON файлы");
        overlay->deleteLater();
        this->setGraphicsEffect(nullptr);
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Выберите набор слов");
    dialog.setModal(true);
    dialog.setFixedSize(350, 450);

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #2e3440;
            color: #d8dee9;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana;
        }
        QLineEdit {
            background-color: #3b4252;
            border: 1px solid #4c566a;
            border-radius: 5px;
            padding: 6px 8px;
            color: #eceff4;
            font-size: 14px;
            margin-bottom: 10px;
        }
        QLineEdit:focus {
            border: 1px solid #88c0d0;
            background-color: #434c5e;
        }
        QListWidget {
            background-color: #3b4252;
            border: 1px solid #4c566a;
            border-radius: 5px;
            color: #eceff4;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 8px 12px;
            border-radius: 3px;
        }
        QListWidget::item:selected {
            background-color: #81a1c1;
            color: #2e3440;
        }
        QListWidget::item:hover {
            background-color: #5e81ac;
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

    QVBoxLayout layout(&dialog);

    QLineEdit *searchEdit = new QLineEdit(&dialog);
    searchEdit->setPlaceholderText("Поиск...");
    layout.addWidget(searchEdit);

    QListWidget *listWidget = new QListWidget(&dialog);
    for (const QFileInfo &fileInfo : fileList) {
        QString name = fileInfo.completeBaseName();
        name.replace('_', ' ');
        listWidget->addItem(name);
    }
    layout.addWidget(listWidget);

    connect(searchEdit, &QLineEdit::textChanged, this, [listWidget](const QString &text){
        QString filter = text.trimmed();
        for (int i = 0; i < listWidget->count(); ++i) {
            QListWidgetItem *item = listWidget->item(i);
            bool match = item->text().contains(filter, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    });

    connect(listWidget, &QListWidget::itemClicked, &dialog, [&](QListWidgetItem *item) {
        QString fileName = item->text();
        fileName.replace(' ', '_') += ".json";
        dialog.accept();

        QString fullPath = languagesPath + '/' + fileName;
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл " + fileName);
            return;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QMessageBox::warning(this, "Ошибка", "Ошибка парсинга JSON: " + parseError.errorString());
            return;
        }

        QStringList wordsList;

        if (doc.isArray()) {
            for (const QJsonValue &val : doc.array()) {
                if (val.isString())
                    wordsList.append(val.toString());
            }
        } else if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("words") && obj.value("words").isArray()) {
                for (const QJsonValue &val : obj.value("words").toArray()) {
                    if (val.isString())
                        wordsList.append(val.toString());
                }
            } else {
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (it.value().isString())
                        wordsList.append(it.value().toString());
                }
            }
        }

        if (wordsList.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "В файле нет слов для генерации");
            return;
        }

        currentWordList_ = wordsList;
        wordsModeActive_ = true;
        GenerateNewTextFromWordList();
    });

    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);

    dialog.exec();

    setGraphicsEffect(nullptr);
    overlay->deleteLater();
}

void Window::GenerateNewTextFromWordList() {
    typing_allowed_ = true;

    if (timeModeActive_) {
        timeRemaining_ = selectedTimeSeconds_;
    }

    QStringList newSelection;
    int count = currentWordAmount_;

    if (currentWordList_.isEmpty() && !numbersModeActive_) {
        generated_text_->clear();
        return;
    }

    QSet<int> usedIndices;
    const double numberChance = numbersModeActive_ ? 0.15 : 0.0;
    const double punctuationChance = punctuationModeActive_ ? 0.20 : 0.0;

    QVector<QChar> punctuationChars = {'!', ',', ';', ':', '?', '.', '-'};

    bool lastWasPunctuation = false;
    bool capitalizeNextWord = true;

    int i = 0;
    while (i < count) {
        QString nextWord;
        bool insertNumber = (QRandomGenerator::global()->generateDouble() < numberChance);

        if (insertNumber) {
            nextWord = QString::number(QRandomGenerator::global()->bounded(1, 1000));
        } else if (currentWordList_.isEmpty()) {
            nextWord = QString::number(QRandomGenerator::global()->bounded(1, 1000));
        } else {
            int index;
            do {
                index = QRandomGenerator::global()->bounded(currentWordList_.size());
            } while (usedIndices.contains(index) && usedIndices.size() < currentWordList_.size());
            usedIndices.insert(index);
            nextWord = currentWordList_.at(index);
        }

        // Управление заглавной буквой первого слова и последующих после знаков
        if (capitalizeNextWord && !nextWord.isEmpty() && !nextWord[0].isDigit()) {
            if (punctuationModeActive_ || i > 0) {
                nextWord[0] = nextWord[0].toUpper();
                capitalizeNextWord = false;
            } else {
                nextWord[0] = nextWord[0].toLower();
                capitalizeNextWord = false;
            }
        } else if (!nextWord.isEmpty()) {
            nextWord[0] = nextWord[0].toLower();
        }

        newSelection.append(nextWord);
        lastWasPunctuation = false;
        ++i;

        // Если текущее слово — число, сбросить флаг capitalizeNextWord,
        // чтобы следующее слово не начиналось с заглавной буквы
        if (!nextWord.isEmpty() && nextWord[0].isDigit()) {
            capitalizeNextWord = false;
        }

        if (punctuationChance == 0.0)
            continue;

        if (i < count && !lastWasPunctuation && (QRandomGenerator::global()->generateDouble() < punctuationChance)) {
            QChar punc;
            do {
                punc = punctuationChars.at(QRandomGenerator::global()->bounded(punctuationChars.size()));
            } while (lastWasPunctuation);

            newSelection.append(QString(punc));
            lastWasPunctuation = true;

            if (punc == '.' || punc == '!' || punc == '?') {
                capitalizeNextWord = true;
            }
        }
    }

    QString result;
    for (int idx = 0; idx < newSelection.size(); ++idx) {
        const QString &token = newSelection.at(idx);
        if (token.length() == 1 && punctuationChars.contains(token[0])) {
            if (token[0] == '-') {
                result += " - ";
            } else {
                result += token;
            }
        } else {
            if (!result.isEmpty() && !result.endsWith(' ')) {
                result += ' ';
            }
            result += token;
        }
    }

    result = result.trimmed().replace("  -  ", " - ");

    generated_text_->setText(result);
    ResetText();
}
void Window::ShowStats() {
    if (currentUsername_.isEmpty()) {
        QMessageBox::information(this, "Инфо", "Сначала войдите в систему");
        return;
    }

    QVector<QPair<QDateTime, QPair<double,double>>> sessions = database_.getTypingSessions(currentUsername_);
    if (sessions.isEmpty()) {
        QMessageBox::information(this, "Статистика", "Нет данных о тестах для пользователя");
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Статистика скорости печати и точности");
    dialog->setModal(true);

    QRect screen_geometry = this->screen()->geometry();
    int width = static_cast<int>(screen_geometry.width() * 0.9);
    int height = static_cast<int>(screen_geometry.height() * 0.9);
    dialog->setFixedSize(width, height);

    dialog->setStyleSheet(R"(
        QDialog {
            background-color: #2e3440;
            color: #d8dee9;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana;
        }
        QCheckBox {
            color: #d8dee9;
            font-size: 14px;
        }
    )");

    QWidget *contentWidget = new QWidget(dialog);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);

    QLineSeries *speedSeries = new QLineSeries();
    QLineSeries *speedMovingAvgSeries = new QLineSeries();
    for (int i = 0; i < sessions.size(); ++i) {
        speedSeries->append(i + 1, sessions[i].second.first);
    }
    int windowSize = 10;
    for (int i = 0; i < sessions.size(); ++i) {
        int startIdx = qMax(0, i - windowSize + 1);
        int count = i - startIdx + 1;
        double sum = 0;
        for (int j = startIdx; j <= i; ++j) {
            sum += sessions[j].second.first;
        }
        double avg = sum / count;
        speedMovingAvgSeries->append(i + 1, avg);
    }

    QChart *speedChart = new QChart();
    speedChart->addSeries(speedSeries);
    speedChart->addSeries(speedMovingAvgSeries);
    speedChart->setTitle("Скорость печати (WPM) по тестам");
    speedChart->setTitleBrush(QBrush(Qt::white));
    speedChart->legend()->hide();
    speedChart->setBackgroundBrush(QBrush(QColor("#3b4252")));

    QValueAxis *axisXSpeed = new QValueAxis();
    axisXSpeed->setRange(1, sessions.size());
    axisXSpeed->setLabelFormat("%d");
    axisXSpeed->setTitleText("Номер теста");
    axisXSpeed->setLabelsBrush(QBrush(Qt::white));
    axisXSpeed->setTitleBrush(QBrush(Qt::white));
    axisXSpeed->setTickCount(qMin(sessions.size(), 10));
    axisXSpeed->setGridLineVisible(true);
    axisXSpeed->setGridLinePen(QPen(QColor("#434c5e"), 1, Qt::DashLine));

    double maxSpeed = 0;
    for (const auto &p : sessions)
        if (p.second.first > maxSpeed)
            maxSpeed = p.second.first;

    QValueAxis *axisYSpeed = new QValueAxis();
    axisYSpeed->setRange(0, maxSpeed + 10);
    axisYSpeed->setTitleText("Скорость (WPM)");
    axisYSpeed->setLabelsBrush(QBrush(Qt::white));
    axisYSpeed->setTitleBrush(QBrush(Qt::white));
    axisYSpeed->setGridLineVisible(true);
    axisYSpeed->setGridLinePen(QPen(QColor("#434c5e"), 1, Qt::DashLine));

    speedChart->addAxis(axisXSpeed, Qt::AlignBottom);
    speedChart->addAxis(axisYSpeed, Qt::AlignLeft);

    speedSeries->attachAxis(axisXSpeed);
    speedSeries->attachAxis(axisYSpeed);
    speedMovingAvgSeries->attachAxis(axisXSpeed);
    speedMovingAvgSeries->attachAxis(axisYSpeed);

    QColor whiteColor(255, 255, 255, 150);
    QPen penWhite(whiteColor);
    penWhite.setWidth(2);
    speedSeries->setPen(penWhite);
    speedSeries->setPointsVisible(false);

    QPen penYellow(QColor("#ffdd00"));
    penYellow.setWidth(4);
    speedMovingAvgSeries->setPen(penYellow);
    speedMovingAvgSeries->setPointsVisible(false);

    QChartView *speedChartView = new QChartView(speedChart);
    speedChartView->setRenderHint(QPainter::Antialiasing);
    speedChartView->setStyleSheet("background-color: transparent;");
    speedChartView->setMinimumHeight(350);

    QLineSeries *accSeries = new QLineSeries();
    QLineSeries *accMovingAvgSeries = new QLineSeries();
    for (int i = 0; i < sessions.size(); ++i) {
        accSeries->append(i + 1, sessions[i].second.second);
    }
    for (int i = 0; i < sessions.size(); ++i) {
        int startIdx = qMax(0, i - windowSize + 1);
        int count = i - startIdx + 1;
        double sum = 0;
        for (int j = startIdx; j <= i; ++j) {
            sum += sessions[j].second.second;
        }
        double avg = sum / count;
        accMovingAvgSeries->append(i + 1, avg);
    }

    QChart *accChart = new QChart();
    accChart->addSeries(accSeries);
    accChart->addSeries(accMovingAvgSeries);
    accChart->setTitle("Точность (%) по тестам");
    accChart->setTitleBrush(QBrush(Qt::white));
    accChart->legend()->hide();
    accChart->setBackgroundBrush(QBrush(QColor("#3b4252")));

    QValueAxis *axisXAcc = new QValueAxis();
    axisXAcc->setRange(1, sessions.size());
    axisXAcc->setLabelFormat("%d");
    axisXAcc->setTitleText("Номер теста");
    axisXAcc->setLabelsBrush(QBrush(Qt::white));
    axisXAcc->setTitleBrush(QBrush(Qt::white));
    axisXAcc->setTickCount(qMin(sessions.size(), 10));
    axisXAcc->setGridLineVisible(true);
    axisXAcc->setGridLinePen(QPen(QColor("#434c5e"), 1, Qt::DashLine));

    QValueAxis *axisYAcc = new QValueAxis();
    axisYAcc->setRange(0, 100);
    axisYAcc->setTitleText("Точность (%)");
    axisYAcc->setLabelsBrush(QBrush(Qt::white));
    axisYAcc->setTitleBrush(QBrush(Qt::white));
    axisYAcc->setGridLineVisible(true);
    axisYAcc->setGridLinePen(QPen(QColor("#434c5e"), 1, Qt::DashLine));

    accChart->addAxis(axisXAcc, Qt::AlignBottom);
    accChart->addAxis(axisYAcc, Qt::AlignLeft);

    accSeries->attachAxis(axisXAcc);
    accSeries->attachAxis(axisYAcc);
    accMovingAvgSeries->attachAxis(axisXAcc);
    accMovingAvgSeries->attachAxis(axisYAcc);

    accSeries->setPen(penWhite);
    accSeries->setPointsVisible(false);
    accMovingAvgSeries->setPen(penYellow);
    accMovingAvgSeries->setPointsVisible(false);

    QChartView *accChartView = new QChartView(accChart);
    accChartView->setRenderHint(QPainter::Antialiasing);
    accChartView->setStyleSheet("background-color: transparent;");
    accChartView->setMinimumHeight(350);

    QCheckBox *cbSpeedRaw = new QCheckBox("Показать скорость (сырые данные)", dialog);
    cbSpeedRaw->setChecked(true);
    QCheckBox *cbSpeedAvg = new QCheckBox("Показать скорость (скользящее среднее)", dialog);
    cbSpeedAvg->setChecked(true);

    QCheckBox *cbAccRaw = new QCheckBox("Показать точность (сырые данные)", dialog);
    cbAccRaw->setChecked(true);
    QCheckBox *cbAccAvg = new QCheckBox("Показать точность (скользящее среднее)", dialog);
    cbAccAvg->setChecked(true);

    contentLayout->addWidget(cbSpeedRaw);
    contentLayout->addWidget(cbSpeedAvg);
    contentLayout->addWidget(speedChartView);

    contentLayout->addSpacing(20);

    contentLayout->addWidget(cbAccRaw);
    contentLayout->addWidget(cbAccAvg);
    contentLayout->addWidget(accChartView);

    QScrollArea *scrollArea = new QScrollArea(dialog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
    dialogLayout->addWidget(scrollArea);

    auto updateVisibility = [speedSeries, speedMovingAvgSeries,
                             accSeries, accMovingAvgSeries,
                             cbSpeedRaw, cbSpeedAvg, cbAccRaw, cbAccAvg]() {
        speedSeries->setVisible(cbSpeedRaw->isChecked());
        speedMovingAvgSeries->setVisible(cbSpeedAvg->isChecked());
        accSeries->setVisible(cbAccRaw->isChecked());
        accMovingAvgSeries->setVisible(cbAccAvg->isChecked());
    };

    QObject::connect(cbSpeedRaw, &QCheckBox::toggled, dialog, updateVisibility);
    QObject::connect(cbSpeedAvg, &QCheckBox::toggled, dialog, updateVisibility);
    QObject::connect(cbAccRaw, &QCheckBox::toggled, dialog, updateVisibility);
    QObject::connect(cbAccAvg, &QCheckBox::toggled, dialog, updateVisibility);

    updateVisibility();

    dialog->move(
        (screen_geometry.width() - dialog->width()) / 2,
        (screen_geometry.height() - dialog->height()) / 2);

    dialog->setWindowOpacity(0);
    dialog->show();

    QPropertyAnimation *anim = new QPropertyAnimation(dialog, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void Window::random() {
    for (int i = 0; i < 100; i++) {
        double randNumber = 120+QRandomGenerator::global()->bounded(15.5);
        double randAccuracy = QRandomGenerator::global()->bounded(100.0);
        database_.saveTypingSession(currentUsername_,randNumber,randAccuracy);
    }
}

void Window::ShowKeyboardLayoutDialog() {
    QString layoutsPath = "/Users/hronov/Documents/Keyboard Trainer/keyboard_layouts";
    QDir dir(layoutsPath);
    QStringList filters {"*.json"};
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks);

    if (fileList.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Папка с раскладками пуста или не найдены JSON файлы");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Выберите раскладку клавиатуры");
    dialog.setModal(true);
    dialog.setFixedSize(350, 450);

    QVBoxLayout layout(&dialog);

    QLineEdit *searchEdit = new QLineEdit(&dialog);
    searchEdit->setPlaceholderText("Поиск...");
    layout.addWidget(searchEdit);

    QListWidget *listWidget = new QListWidget(&dialog);

    for (const QFileInfo &fileInfo : fileList) {
        QString name = fileInfo.completeBaseName();
        name.replace('_', ' ');
        listWidget->addItem(name);
    }
    layout.addWidget(listWidget);

    connect(searchEdit, &QLineEdit::textChanged, this, [listWidget](const QString &text){
        QString filter = text.trimmed();
        for (int i = 0; i < listWidget->count(); ++i) {
            QListWidgetItem *item = listWidget->item(i);
            bool match = item->text().contains(filter, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    });

    connect(listWidget, &QListWidget::itemClicked, &dialog, [&](QListWidgetItem* item){
        QString fileName = item->text();
        fileName.replace(' ', '_');
        QString fileNameToSave = fileName;
        fileName += ".json";
        dialog.accept();

        QString fullPath = layoutsPath + '/' + fileName;

        if (!keyboardWidget_->loadLayoutFromJson(fullPath)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить раскладку из файла");
        } else {
            if (!currentUsername_.isEmpty()) {
                database_.updateUserSetting(currentUsername_,"keyboard_layout",fileNameToSave);
            }
        }
    });

    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);

    dialog.exec();
}

void Window::ShowAmountSelectorDialog() {
    QWidget *overlay = new QWidget(this);
    overlay->setObjectName("overlayWidget");
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 120);");
    overlay->show();

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(overlay);
    blur->setBlurRadius(40);
    this->setGraphicsEffect(blur);

    QDialog dialog(this);
    dialog.setWindowTitle("Выберите количество слов");
    dialog.setModal(true);
    dialog.resize(470, 400);

    QWidget *formContainer = new QWidget(&dialog);
    formContainer->setObjectName("formContainer");
    formContainer->setFixedSize(430, 360);

    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(30, 25, 30, 25);
    formLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Введите количество слов", formContainer);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #d8dee9;");

    QLineEdit *inputLine = new QLineEdit(formContainer);
    inputLine->setValidator(new QIntValidator(1, 9999, &dialog));
    inputLine->setText(QString::number(currentWordAmount_));
    inputLine->setPlaceholderText("Число от 1 до 9999");
    inputLine->setObjectName("inputField");
    inputLine->setFixedHeight(50);

    QHBoxLayout *quickButtonsLayout = new QHBoxLayout();
    quickButtonsLayout->setSpacing(12);

    auto createQuickBtn = [&](const QString &text) {
        QPushButton *btn = new QPushButton(text, formContainer);
        btn->setObjectName("secondaryButton");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedSize(70, 34);
        connect(btn, &QPushButton::clicked, this, [inputLine, text]() {
            inputLine->setText(text);
        });
        return btn;
    };

    quickButtonsLayout->addStretch();
    quickButtonsLayout->addWidget(createQuickBtn("10"));
    quickButtonsLayout->addWidget(createQuickBtn("25"));
    quickButtonsLayout->addWidget(createQuickBtn("50"));
    quickButtonsLayout->addWidget(createQuickBtn("100"));
    quickButtonsLayout->addStretch();

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(20);

    QPushButton *okButton = new QPushButton("OK", formContainer);
    okButton->setObjectName("primaryButton");
    okButton->setDefault(true);
    okButton->setCursor(Qt::PointingHandCursor);
    okButton->setFixedSize(100, 40);

    QPushButton *cancelButton = new QPushButton("Отмена", formContainer);
    cancelButton->setObjectName("secondaryButton");
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setFixedSize(100, 40);

    actionsLayout->addStretch();
    actionsLayout->addWidget(okButton);
    actionsLayout->addWidget(cancelButton);
    actionsLayout->addStretch();

    formLayout->addWidget(titleLabel);
    formLayout->addWidget(inputLine);
    formLayout->addLayout(quickButtonsLayout);
    formLayout->addLayout(actionsLayout);

    QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->addStretch();
    dialogLayout->addWidget(formContainer, 0, Qt::AlignHCenter);
    dialogLayout->addStretch();
    dialogLayout->setContentsMargins(0, 0, 0, 0);

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #2e3440;
            color: #d8dee9;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana;
            font-weight: normal;
        }
        #formContainer {
            background-color: #3b4252;
            border-radius: 12px;
        }
        #inputField {
            background-color: #3b4252;
            border: 1.5px solid #4c566a;
            border-radius: 8px;
            padding: 8px 12px;
            color: #eceff4;
            font-size: 16px;
            font-weight: normal;
            margin-bottom: 10px;
        }
        #inputField:focus {
            border: 1.5px solid #88c0d0;
            background-color: #434c5e;
            outline: none;
        }
        QPushButton#primaryButton {
            background-color: #667eea;
            color: white;
            border-radius: 14px;
            font-weight: normal;
            font-size: 16px;
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
            border: 2px solid #667eea;
            color: #667eea;
            border-radius: 14px;
            font-weight: normal;
            font-size: 14px;
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

    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);

    connect(okButton, &QPushButton::clicked, &dialog, [&dialog, inputLine]() {
        bool ok = false;
        int val = inputLine->text().toInt(&ok);
        if (!ok || val <= 0) {
            inputLine->setFocus();
            return;
        }
        dialog.accept();
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        bool ok = false;
        int val = inputLine->text().toInt(&ok);
        if (ok && val > 0) {
            currentWordAmount_ = val;
            if (!currentUsername_.isEmpty()) {
                database_.updateUserSetting(currentUsername_, "words_amount", QVariant(val));
            }
            GenerateNewTextFromWordList();
        }
    }

    setGraphicsEffect(nullptr);
    overlay->deleteLater();
}

void Window::ShowTimeSelectorDialog() {
    QWidget *overlay = new QWidget(this);
    overlay->setObjectName("overlayWidget");
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 120);");
    overlay->show();

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(overlay);
    blur->setBlurRadius(40);
    this->setGraphicsEffect(blur);

    QDialog dialog(this);
    dialog.setWindowTitle("Выберите длительность теста в секундах");
    dialog.setModal(true);
    dialog.resize(400, 250);

    QWidget *formContainer = new QWidget(&dialog);
    formContainer->setObjectName("formContainer");
    formContainer->setFixedSize(360, 200);

    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(30, 25, 30, 25);
    formLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Введите время (в секундах)", formContainer);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #d8dee9;");

    QLineEdit *inputLine = new QLineEdit(formContainer);
    inputLine->setValidator(new QIntValidator(1, 3600, &dialog)); // от 1 до 3600 сек (1 час)
    inputLine->setText(QString::number(selectedTimeSeconds_));
    inputLine->setPlaceholderText("Число от 1 до 3600");
    inputLine->setObjectName("inputField");
    inputLine->setFixedHeight(50);

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(20);

    QPushButton *okButton = new QPushButton("OK", formContainer);
    okButton->setObjectName("primaryButton");
    okButton->setDefault(true);
    okButton->setCursor(Qt::PointingHandCursor);
    okButton->setFixedSize(100, 40);

    QPushButton *cancelButton = new QPushButton("Отмена", formContainer);
    cancelButton->setObjectName("secondaryButton");
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setFixedSize(100, 40);

    actionsLayout->addStretch();
    actionsLayout->addWidget(okButton);
    actionsLayout->addWidget(cancelButton);
    actionsLayout->addStretch();

    formLayout->addWidget(titleLabel);
    formLayout->addWidget(inputLine);
    formLayout->addLayout(actionsLayout);

    QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->addStretch();
    dialogLayout->addWidget(formContainer, 0, Qt::AlignHCenter);
    dialogLayout->addStretch();
    dialogLayout->setContentsMargins(0, 0, 0, 0);

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #2e3440;
            color: #d8dee9;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana;
            font-weight: normal;
        }
        #formContainer {
            background-color: #3b4252;
            border-radius: 12px;
        }
        #inputField {
            background-color: #3b4252;
            border: 1.5px solid #4c566a;
            border-radius: 8px;
            padding: 8px 12px;
            color: #eceff4;
            font-size: 16px;
            font-weight: normal;
            margin-bottom: 10px;
        }
        #inputField:focus {
            border: 1.5px solid #88c0d0;
            background-color: #434c5e;
            outline: none;
        }
        QPushButton#primaryButton {
            background-color: #667eea;
            color: white;
            border-radius: 14px;
            font-weight: normal;
            font-size: 16px;
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
            border: 2px solid #667eea;
            color: #667eea;
            border-radius: 14px;
            font-weight: normal;
            font-size: 14px;
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

    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);

    connect(okButton, &QPushButton::clicked, &dialog, [&dialog, inputLine]() {
        bool ok = false;
        int val = inputLine->text().toInt(&ok);
        if (!ok || val <= 0) {
            inputLine->setFocus();
            return;
        }
        dialog.accept();
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        bool ok = false;
        int val = inputLine->text().toInt(&ok);
        if (ok && val > 0) {
            selectedTimeSeconds_ = val;
            timeLabel_->setText(QString("Осталось времени: %1 с").arg(selectedTimeSeconds_));
            GenerateNewTextFromWordList();
        }
    }

    setGraphicsEffect(nullptr);
    overlay->deleteLater();
}

void Window::StartCountdownTimer() {
    if (!countdownTimer_->isActive()) {
        timeRemaining_ = selectedTimeSeconds_;
        countdownTimer_->start();
    }
}