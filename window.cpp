#include "window.h"

Window::Window(Database &db, QWidget *parent)
    : QWidget(parent), database_(db) {

    // --- Настройка виджета настроек ---
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
        caretSmooth_ = s.caret_smooth;

        ApplyTextStyles();
    });

    // --- Таймер для подсчета WPM ---
    typing_timer_ = new QTimer(this);
    typing_timer_->setInterval(kIntervalMs);
    connect(typing_timer_, &QTimer::timeout, this, &Window::UpdateWPM);
    elapsed_seconds_ = 0;

    // --- Настройка меток ---
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

    // --- Кнопки настроек и входа ---
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

    // --- Нижняя панель с кнопками и именем пользователя ---
    auto bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(settings_button);
    bottomLayout->addSpacing(100);
    bottomLayout->addWidget(login_button);

    usernameLabel_ = new QLabel("", this);
    usernameLabel_->setStyleSheet("font-size: 18px; color: #d8dee9; letter-spacing: 2px; word-spacing: 2px;");
    bottomLayout->addWidget(usernameLabel_);
    bottomLayout->setAlignment(Qt::AlignCenter);

    // --- Категории / Меню ---
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
            button->setStyleSheet(R"(
                color: #eee;
                font-size: 14px;
                padding: 5px 12px;
                background-color: rgba(245, 245, 245, 0.15);
                border-radius: 8px;
                font-weight: 600;
            )");
            categoryLayout->addWidget(button);

            QMap<QString, std::function<void()>> actions = {
                { "words", [this]() { ShowWordSetDialog(); } },
                { "ai", [this]() { Prompt(); } },
                { "language", [this]() { ShowLanguageDialog(); } },
                { "stop", [this]() { DisableTyping(); } },
                { "stats", [this]() { ShowStats(); } },
                { "quote", [this]() { random(); } },
            };

            connect(button, &QPushButton::clicked, this, [this, text, actions]() {
                auto it = actions.find(text);
                if (it != actions.end()) {
                    it.value()();
                }
            });
        }
    };

    // Добавляем кнопки и метки категорий
    addCategoryWidget("ai", true);
    addCategoryWidget("language", true);
    addCategoryWidget("punctuation");
    addCategoryWidget("numbers");
    addCategoryWidget("time");
    addCategoryWidget("words", true);
    addCategoryWidget("quote",true);
    addCategoryWidget("custom", true);
    addCategoryWidget("stop", true);
    addCategoryWidget("stats", true);

    categoryWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QHBoxLayout *categoryWrapperLayout = new QHBoxLayout();
    categoryWrapperLayout->addStretch();
    categoryWrapperLayout->addWidget(categoryWidget);
    categoryWrapperLayout->addStretch();

    // --- Основной лейаут ---
    auto main_layout = new QVBoxLayout(this);
    main_layout->addLayout(bottomLayout);
    main_layout->addLayout(categoryWrapperLayout);
    main_layout->addSpacing(20);
    main_layout->addWidget(generated_text_, 0, Qt::AlignHCenter);
    main_layout->addWidget(statusLabel_);
    main_layout->addStretch();
    main_layout->setAlignment(Qt::AlignTop);

    setLayout(main_layout);

    // --- Глобальный стиль ---
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

// === Методы ---

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

    // Центрируем диалог по экрану
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

    if (event->key() == Qt::Key_Tab && wordsModeActive_) {
        event->accept();
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
                if (caretStyle_.isEmpty()) {
                    colored_text += "<span style='color:" + textColor_.name() + ";'>" + QString(targetText_.at(i)) +
                        "</span>";
                } else if (caretStyle_ == "_") {
                         colored_text += "<span style='text-decoration: underline; color: white;'>" +
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

    if (currentIndex_ == targetText_.length()) {
        typing_allowed_ = false;
        double minutes = elapsed_seconds_ / kSecondsInMinute;
        double raw_wpm = (typedCharCount_ / kWpmCoefficient) / minutes;
        double accuracy = kHundred - ((double)errorCount_ / targetText_.length() * kHundred);
        accuracy = qMax(accuracy, 0.0);


        if (!currentUsername_.isEmpty()) {
            database_.saveTypingSession(currentUsername_, raw_wpm * accuracy / kHundred, accuracy);
        }

        StopTypingTimer();
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

void Window::LoadTextFromFile() {
    QString file_name = QFileDialog::getOpenFileName(this, "Выберите текстовый файл", "",
                                                    "Текстовые файлы (*.txt);;Все файлы (*.*)");
    if (file_name.isEmpty())
        return;

    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    QTextStream in(&file);
    QString text = in.readAll();
    file.close();

    generated_text_->setText(text);
    ResetText();
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
    caretSmooth_ = caretMap.value(settings.caret_smooth);
    caretStyle_ = settings.caret_style;


    ApplyTextStyles();

    usernameLabel_->setText(currentUsername_);
}

void Window::ShowSettings() {
    if (currentUsername_.isEmpty()) {
        QMessageBox::information(this, "Инфо", "Сначала войдите в систему");
        return;
    }

    settingsWidget_->loadSettings();

    // Получение геометрии экрана для позиционирования
    QRect screenGeometry;
    if (settingsWidget_->parentWidget()) {
        screenGeometry = settingsWidget_->parentWidget()->screen()->geometry();
    } else {
        screenGeometry = QGuiApplication::primaryScreen()->geometry();
    }

    settingsWidget_->setGeometry(screenGeometry);
    settingsWidget_->show();

    // Анимация появления
    QPropertyAnimation *anim = new QPropertyAnimation(settingsWidget_, "windowOpacity");
    settingsWidget_->setWindowOpacity(0);
    anim->setDuration(300);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void Window::ShowWordSetDialog() {
    // Создаем overlay — затемняющий полупрозрачный виджет поверх главного
    QWidget *overlay = new QWidget(this);
    overlay->setObjectName("overlayWidget");
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
    overlay->show();

    // Добавляем блюр к главному виджету
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(overlay);
    blur->setBlurRadius(40);
    this->setGraphicsEffect(blur);

    // Получаем список JSON файлов в папке языков
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

    // Создаем диалог выбора набора слов
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

    // Поисковое поле
    QLineEdit *searchEdit = new QLineEdit(&dialog);
    searchEdit->setPlaceholderText("Поиск...");
    layout.addWidget(searchEdit);

    // Список наборов слов
    QListWidget *listWidget = new QListWidget(&dialog);
    for (const QFileInfo &fileInfo : fileList) {
        QString name = fileInfo.completeBaseName();
        name.replace('_', ' ');
        listWidget->addItem(name);
    }
    layout.addWidget(listWidget);

    // Фильтрация списка по поиску
    connect(searchEdit, &QLineEdit::textChanged, this, [listWidget](const QString &text){
        QString filter = text.trimmed();
        for (int i = 0; i < listWidget->count(); ++i) {
            QListWidgetItem *item = listWidget->item(i);
            bool match = item->text().contains(filter, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    });

    // Обработка выбора набора слов
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

        // Парсим слова из JSON
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
        typing_allowed_ = true;
    });

    // Центрируем диалог
    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);

    dialog.exec();

    // Убираем блюр и оверлей по закрытию диалога
    setGraphicsEffect(nullptr);
    overlay->deleteLater();
}

void Window::GenerateNewTextFromWordList() {
    if (currentWordList_.isEmpty()) {
        return;
    }

    QStringList newSelection;
    QSet<int> usedIndices;
    int count = std::min(15, int(currentWordList_.size()));

    while (newSelection.size() < count) {
        int index = QRandomGenerator::global()->bounded(currentWordList_.size());
        if (!usedIndices.contains(index)) {
            usedIndices.insert(index);
            newSelection.append(currentWordList_.at(index));
        }
    }

    generated_text_->setText(newSelection.join(' '));
    ResetText();
}

void Window::ShowStats() {
    if (currentUsername_.isEmpty()) {
        QMessageBox::information(this, "Инфо", "Сначала войдите в систему");
        return;
    }

    QVector<QPair<QDateTime, double>> sessions = database_.getTypingSessionsForUser(currentUsername_);
    if (sessions.isEmpty()) {
        QMessageBox::information(this, "Статистика", "Нет данных о тестах для пользователя");
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Статистика скорости печати");
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

    // Исходные данные - белая линия
    QLineSeries *series = new QLineSeries();
    for (int i = 0; i < sessions.size(); ++i) {
        series->append(i + 1, sessions[i].second);
    }

    // Рассчёт скользящего среднего (moving average)
    int windowSize = 10;
    QLineSeries *movingAvgSeries = new QLineSeries();
    for (int i = 0; i < sessions.size(); ++i) {
        int startIdx = qMax(0, i - windowSize + 1);
        int count = i - startIdx + 1;
        double sum = 0;
        for (int j = startIdx; j <= i; ++j) {
            sum += sessions[j].second;
        }
        double avg = sum / count;
        movingAvgSeries->append(i + 1, avg);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->addSeries(movingAvgSeries);

    chart->setTitle("Скорость печати (WPM) по номерам тестов");
    chart->setTitleBrush(QBrush(Qt::white));
    chart->legend()->hide();
    chart->setBackgroundBrush(QBrush(QColor("#3b4252")));

    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(1, sessions.size());
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Номер теста");
    axisX->setTitleBrush(QBrush(Qt::white));
    axisX->setLabelsBrush(QBrush(Qt::white));
    axisX->setTickCount(qMin(sessions.size(), 10));
    axisX->setGridLineVisible(true);
    axisX->setGridLinePen(QPen(QColor("#434c5e"), 1, Qt::DashLine));

    double maxWpm = 0;
    for (const auto &p : sessions) {
        if (p.second > maxWpm)
            maxWpm = p.second;
    }

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxWpm + 10);
    axisY->setTitleText("Скорость (WPM)");
    axisY->setTitleBrush(QBrush(Qt::white));
    axisY->setLabelsBrush(QBrush(Qt::white));
    axisY->setGridLineVisible(true);
    axisY->setGridLinePen(QPen(QColor("#434c5e"), 1, Qt::DashLine));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);
    movingAvgSeries->attachAxis(axisX);
    movingAvgSeries->attachAxis(axisY);

    QColor whiteColor(255, 255, 255, 150); // частичная прозрачность
    QPen penWhite(whiteColor);
    penWhite.setWidth(2);
    series->setPen(penWhite);
    series->setPointsVisible(false);

    QPen penYellow(QColor("#ffdd00"));
    penYellow.setWidth(4);
    movingAvgSeries->setPen(penYellow);
    movingAvgSeries->setPointsVisible(false);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background-color: transparent;");

    // Чекбоксы
    QCheckBox *cbRaw = new QCheckBox("Show raw speed", dialog);
    cbRaw->setChecked(true);
    QCheckBox *cbAvg = new QCheckBox("Show average speed", dialog);
    cbAvg->setChecked(true);

    // Контейнер для расположения элементов: чекбоксов сверху и графика ниже
    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    // Горизонтальный layout для чекбоксов
    QHBoxLayout *checkBoxLayout = new QHBoxLayout();
    checkBoxLayout->addWidget(cbRaw);
    checkBoxLayout->addWidget(cbAvg);
    checkBoxLayout->addStretch();

    mainLayout->addLayout(checkBoxLayout);
    mainLayout->addWidget(chartView);

    // Слот для обновления видимости линий по чекбоксам
    auto updateVisibility = [series, movingAvgSeries, cbRaw, cbAvg]() {
        series->setVisible(cbRaw->isChecked());
        movingAvgSeries->setVisible(cbAvg->isChecked());
    };

    // Подключаем сигналы к слоту
    QObject::connect(cbRaw, &QCheckBox::toggled, dialog, updateVisibility);
    QObject::connect(cbAvg, &QCheckBox::toggled, dialog, updateVisibility);

    // Вызвать однократно для установки начального состояния
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