#include "window.h"

Window::Window(Database &db,QWidget *parent) : QWidget(parent), database_(db) {
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
        ApplyTextStyles();
    });

    QString button_style = R"(
        QPushButton {
            background-color: #ffffff;
            border: 1px solid #000000;
            color: #000000;
            padding: 5px 10px;
            font-size: 16px;
            margin: 2px;
            border-radius: 3px;
        }
        QPushButton:hover {
            background-color: #f0f0f0;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
            color: #333333;
        }
    )";

    typing_timer_ = new QTimer(this);
    typing_timer_->setInterval(kIntervalMs);
    connect(typing_timer_, &QTimer::timeout, this, &Window::UpdateWPM);
    elapsed_seconds_ = 0;

    generated_text_ = new QLabel(this);
    generated_text_->setStyleSheet(
        "font-size: 16px; color: #FFFFFF; letter-spacing: 2px; word-spacing: 2px; font-weight: 500;");
    // Выравнивание по центру горизонтально и по верху вертикально
    generated_text_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    generated_text_->setWordWrap(true);
    generated_text_->setFixedWidth(kTextFieldWidth);
    generated_text_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    generated_text_->setMinimumHeight(kTextFieldMinimumHeigth);

    auto open_button = new QPushButton("Сгенерировать текст", this);
    open_button->setFixedSize(kButtonWidth, kButtonHeight);
    open_button->setStyleSheet(button_style);
    connect(open_button, &QPushButton::clicked, this, &Window::Prompt);

    auto language_button = new QPushButton("Выбрать язык", this);
    language_button->setFixedSize(kButtonWidth, kButtonHeight);
    language_button->setStyleSheet(button_style);
    connect(language_button, &QPushButton::clicked, this, &Window::ShowLanguageDialog);

    auto enable_typing_button = new QPushButton("Начать печатать", this);
    enable_typing_button->setFixedSize(kButtonWidth, kButtonHeight);
    enable_typing_button->setStyleSheet(button_style);
    connect(enable_typing_button, &QPushButton::clicked, this, &Window::EnableTyping);

    auto disable_typing_button = new QPushButton("Закончить печатать", this);
    disable_typing_button->setFixedSize(kButtonWidth, kButtonHeight);
    disable_typing_button->setStyleSheet(button_style);
    connect(disable_typing_button, &QPushButton::clicked, this, &Window::DisableTyping);

    statusLabel_ = new QLabel("RAW WPM: 0 | Точность: 100% | WPM: 0", this);
    statusLabel_->setAlignment(Qt::AlignBottom | Qt::AlignCenter);
    statusLabel_->setStyleSheet(
        "font-size: 20px; color: #FFFFFF; letter-spacing: 2px; word-spacing: 2px");

    // auto load_text_button = new QPushButton("Загрузить текст", this);
    // load_text_button->setFixedSize(kButtonWidth, kButtonHeight);
    // load_text_button->setStyleSheet(button_style);
    // connect(load_text_button, &QPushButton::clicked, this, &Window::LoadTextFromFile);

    auto wordset_button = new QPushButton("Выбрать набор слов", this);
    wordset_button->setFixedSize(kButtonWidth, kButtonHeight);
    wordset_button->setStyleSheet(button_style);
    connect(wordset_button, &QPushButton::clicked, this, &Window::ShowWordSetDialog);


    auto login_button = new QPushButton("Войти", this);
    login_button->setFixedSize(kButtonWidth, kButtonHeight);
    login_button->setStyleSheet(button_style);
    connect(login_button, &QPushButton::clicked, this, &Window::showLoginDialog);

    auto settings_button = new QPushButton(this);
    settings_button->setIcon(QIcon("/Users/hronov/Documents/Keyboard Trainer/icons/settings.svg"));
    settings_button->setIconSize(QSize(45, 45));
    settings_button->setFlat(true);
    settings_button->setCursor(Qt::PointingHandCursor);
    connect(settings_button, &QPushButton::clicked, this, &Window::ShowSettings);

    accountIconLabel = new QSvgWidget("/Users/hronov/Documents/Keyboard Trainer/icons/account.svg", this);
    accountIconLabel->setFixedSize(44, 45);

    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(language_button);
    button_layout->addWidget(open_button);
    button_layout->addWidget(enable_typing_button);
    button_layout->addWidget(disable_typing_button);
    // button_layout->addWidget(load_text_button);
    button_layout->addWidget(wordset_button);
    button_layout->addWidget(login_button);
    button_layout->setAlignment(Qt::AlignCenter);
    button_layout->setSpacing(kLayoutSpacing);

    auto bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(settings_button);
    bottomLayout->addSpacing(100);
    bottomLayout->addWidget(accountIconLabel);
    usernameLabel = new QLabel("", this);
    usernameLabel->setStyleSheet("font-size: 18px; color: #FFFFFF; letter-spacing: 2px; word-spacing: 2px");
    bottomLayout->addWidget(usernameLabel);
    bottomLayout->setAlignment(Qt::AlignCenter);

    auto main_layout = new QVBoxLayout(this);
    main_layout->addLayout(bottomLayout);
    main_layout->addLayout(button_layout);
    main_layout->addSpacing(20); // Отступ между кнопками и текстом
    main_layout->addWidget(generated_text_, 0, Qt::AlignHCenter);
    main_layout->addWidget(statusLabel_);
    main_layout->addStretch();

    main_layout->setAlignment(Qt::AlignTop);

    setLayout(main_layout);
}

void Window::SetLanguage(const QString& language) {
    prompt_language_ = language;  // setting language for HTTP POST request
}

void Window::Prompt() {
    if (!prompt_language_.isEmpty()) {
        try {
            typing_allowed_ = false;
            QString result_final = QString::fromStdString(getResponse(
                kPromptTemplatePart1 + std::to_string(kWordsNumber) +
                kPromptTemplatePart2 +
                "IMPORTANT. set-language:" + prompt_language_.toStdString()));
            generated_text_->setText(
                result_final);	// getting response from AI model
            ResetText();

            //just animation for text appearing below
            effect_ = new QGraphicsOpacityEffect(this);
            generated_text_->setGraphicsEffect(effect_);
            animation_ = new QPropertyAnimation(effect_, "opacity");
            animation_->setDuration(kAnimationDurationMs);
            animation_->setStartValue(0.0);
            animation_->setEndValue(1.0);
            animation_->start(QAbstractAnimation::DeleteWhenStopped);

        } catch (const nlohmann::json::type_error& e) {
            QMessageBox::warning(
                this, "Ошибка",
                "Ошибка обработки JSON-ответа. Попробуйте включить "
                "VPN.");  // due to blocking Belarusian IP
        }
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Сначала выберите язык для генерации.");
    }  // if not choosen
}


void Window::ShowLanguageDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Выберите язык");
    dialog.setModal(
        true);	// can't interract with main window while using modal one
    dialog.setFixedSize(kLanguageChoiceWidth, kLanguageChoiceHeight);
    QVBoxLayout layout(&dialog);
    QListWidget list_widget(&dialog);


    for (const auto& lang : kLanguages) {
        list_widget.addItem(QString::fromStdString(lang));
    }  // adding widgets-languages


    connect(
        &list_widget, &QListWidget::itemClicked, this,
        [&dialog, this](QListWidgetItem* item) {
            SetLanguage(item->text());
            dialog.accept();  // when choosen
        });	 // capturing both windows and using parameter item just to set the text of this item(lambda-function)
    layout.addWidget(&list_widget);	 // adding to list
    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);	// to be centered

    dialog.exec();	// executing and making it modal so waiting till closed
}

void Window::EnableTyping() {
    if (generated_text_->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала сгенерируйте текст.");
        return;
    } else {
        typing_allowed_ = true;	 // allowing to type, preventing UB from user
    }
}

void Window::DisableTyping() {


    if (typing_allowed_) {
        typing_allowed_ = false;
        generated_text_->setText("");
        ResetText();  // resetting everything when user doesn't wanna type anymore (e.x. in the middle of typing)
    }
}

void Window::ResetText() {
    targetText_ = generated_text_->text();	// setting text we will type
    typedChars_.assign(
        targetText_.length(),
        '|');  // creating a vector or our symbols(| just some symbol that is not usually typed(in my case - never), not space cause we use it)
    errorFlags_.assign(
        targetText_.length(),
        false);	 // linking whether our symbol is typed with error(true) or not(false), by default false
    currentIndex_ = 0;
    errorCount_ = 0;
    typedCharCount_ = 0;
    statusLabel_->setText(
        "RAW WPM: 0 | Точность: 100% | WPM: 0");  // resetting the info of typing session
    StopTypingTimer();
}
void Window::StartTypingTimer() {
    if (!typing_timer_->isActive()) {
        elapsed_seconds_ = 0;
        typing_timer_->start();	 // when the first symbol is pressed
    }
}

void Window::StopTypingTimer() {
    typing_timer_->stop();	// GUESS WHAT IT DOES
}

void Window::UpdateWPM() {
    elapsed_seconds_ += kUpdateIntervalSec;
    double minutes =
        double(elapsed_seconds_) /
        kSecondsInMinute;  // how many minutes user has been typing since start


    if (minutes > 0) {
        double raw_wpm =
            (double(typedCharCount_) / kWpmCoefficient) /
            minutes;  // raw - without accuracy(just amount of typed symbols without caring bout errors)
        double accuracy =
            kHundred -
            (((double)errorCount_ / targetText_.length()) * kHundred);
        if (accuracy < 0) {
            accuracy = 0;
        }
        statusLabel_->setText(
            QString("RAW WPM: %1 | Точность: %2% | WPM: %3")
                .arg(QString::number(raw_wpm, 'f', 2))
                .arg(QString::number(accuracy, 'f', 2))
                .arg(QString::number(raw_wpm * accuracy / kHundred, 'f', 2)));
    }
}

void Window::keyPressEvent(QKeyEvent* event) {


    if (!typing_allowed_) {
        return;
    }


    typedCharCount_ = 0;
    if (event->key() == Qt::Key_Backspace) {


        if (currentIndex_ > 0) {
            typedCharCount_--;
            --currentIndex_;
            typedChars_[currentIndex_] = '|';
            errorFlags_[currentIndex_] = false;
        }  // just logic for backspace pressing while typing
    } else {
        QString new_text = event->text();  // getting our symbol
        if (!new_text.isEmpty() &&
            currentIndex_ <
                targetText_
                    .length()) {  // if not service keys (Shift, etc.) + if typed less than the text length


            if (currentIndex_ == 0) {
                StartTypingTimer();
            }  // if pressed at the beginning, we start our timer

            QChar expected_char =
                targetText_.at(currentIndex_);	// what we expect to symbol be
            QChar typed_char = new_text.at(
                0);	 // what it really is (0 cause event->text() returns symbol)


            if (typed_char == expected_char) {
                errorFlags_[currentIndex_] =
                    false;	//could be optimised by removing(assigning our vector by default by false), just for clarity
            } else {
                errorFlags_[currentIndex_] = true;
                errorCount_++;
            }  // logic for wrong symbol
            typedChars_[currentIndex_] = typed_char;
            currentIndex_++;
        }
    }

    QString colored_text = "";
    for (int i = 0; i < targetText_.length(); ++i) {


        if (typedChars_[i] != '|') {  // If this position has been typed
            QString color = errorFlags_[i] ? "red" : "green";
            QChar target_char = targetText_.at(i);

            // If it's a space with an error, underline it without using &nbsp;
            if (target_char == ' ' && errorFlags_[i]) {
                colored_text +=
                    "<span style='text-decoration: underline; color: red;'> "
                    "</span>";
            } else {
                colored_text += "<span style='color:" + color + ";'>" +
                                QString(targetText_.at(i)) + "</span>";
            }
            typedCharCount_++;
        } else {  // If the symbol hasn't been typed yet
            if (i == currentIndex_) {
                colored_text +=
                    "<span style='text-decoration: underline; color: white;'>" +
                    QString(targetText_.at(i)) +
                    "</span>";	// Underline for the current symbol
            } else {
                colored_text += "<span style='color:" + textColor_.name() +
                                ";'>" + QString(targetText_.at(i)) + "</span>";
            }
        }
    }

    generated_text_->setText(colored_text);	 // setting colored text
    if (currentIndex_ ==
        targetText_.length()) {	 // if we hit the end of the text
        typing_allowed_ = false;
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
        "line-height: %6px;"
        )
        .arg(fontSize_)
        .arg(textColor_.name())
        .arg(fontWeight_)
        .arg(letterSpacing_)
        .arg(wordSpacing_)
        .arg(lineHeight_)
        .arg(currentFont_.family())
    );
}  // gathering and applying all parameters (1 can be reset if 2-nd is changed separately)

void Window::LoadTextFromFile() {  // just setting a text from a file
    QString file_name = QFileDialog::getOpenFileName(
        this, "Выберите текстовый файл", "",
        "Текстовые файлы (*.txt);;Все файлы (*.*)");
    if (!file_name.isEmpty()) {
        QFile file(file_name);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString text = in.readAll();
            file.close();
            generated_text_->setText(text);
            ResetText();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        }
    }
}


void Window::showLoginDialog()
{
    if (database_.initDatabase()) {
        LoginDialog dialog(database_, this);
        if (dialog.exec() == QDialog::Accepted && dialog.isLoggedIn()) {
            currentUsername_ = dialog.getUsername();
            QMessageBox::information(this, "Успех",
                "Вы вошли как: " + dialog.getUsername());
            currentUsername_ = dialog.getUsername();
            settingsWidget_->setUsername(currentUsername_);
            LoadUserSettings();
        }
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
    }
}

void Window::LoadUserSettings()
{
    if (currentUsername_.isEmpty()) {
        return;
    }

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

        ApplyTextStyles();

        usernameLabel->setText(currentUsername_);
}


void Window::ShowSettings() {
    if (!currentUsername_.isEmpty()) {
        settingsWidget_->loadSettings();

        // Получить размер экрана, на котором находится окно или родитель
        QRect screenGeometry;
        if (settingsWidget_->parentWidget()) {
            screenGeometry = settingsWidget_->parentWidget()->screen()->geometry();
        } else {
            screenGeometry = QGuiApplication::primaryScreen()->geometry();
        }

        // Установить размер окна равным размеру экрана и позицию (0,0) экрана
        settingsWidget_->setGeometry(screenGeometry);

        // Показать окно нормальным способом, не fullscreen
        settingsWidget_->show();

        // Анимация появления (опционально)
        QPropertyAnimation *anim = new QPropertyAnimation(settingsWidget_, "windowOpacity");
        settingsWidget_->setWindowOpacity(0);
        anim->setDuration(300);
        anim->setStartValue(0);
        anim->setEndValue(1);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        QMessageBox::information(this, "Инфо", "Сначала войдите в систему");
    }
}


void Window::ShowWordSetDialog() {
    // Создаем overlay — затемняющий полупрозрачный виджет поверх родителя
    QWidget *overlay = new QWidget(this);
    overlay->setObjectName("overlayWidget");
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
    overlay->show();

    // Применяем блюр к родителю (к this)
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(overlay);
    blur->setBlurRadius(40);
    this->setGraphicsEffect(blur);

    QString languagesPath = "/Users/hronov/Documents/Keyboard Trainer/languages";
    QDir dir(languagesPath);
    QStringList filters;
    filters << "*.json";
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
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
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
            QJsonArray jsonArray = doc.array();
            for (const QJsonValue &val : jsonArray) {
                if (val.isString()) {
                    wordsList.append(val.toString());
                }
            }
        } else if (doc.isObject()) {
            QJsonObject jsonObj = doc.object();
            if (jsonObj.contains("words") && jsonObj.value("words").isArray()) {
                QJsonArray jsonArray = jsonObj.value("words").toArray();
                for (const QJsonValue &val : jsonArray) {
                    if (val.isString()) {
                        wordsList.append(val.toString());
                    }
                }
            } else {
                for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
                    if (it.value().isString())
                        wordsList.append(it.value().toString());
                }
            }
        }

        if (wordsList.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "В файле нет слов для генерации");
            return;
        }

        int count = std::min(15, int(wordsList.size()));
        QStringList selectedWords;
        QSet<int> usedIndices;
        while (selectedWords.size() < count) {
            int index = QRandomGenerator::global()->bounded(wordsList.size());
            if (!usedIndices.contains(index)) {
                usedIndices.insert(index);
                selectedWords.append(wordsList.at(index));
            }
        }

        generated_text_->setText(selectedWords.join(" "));
        ResetText();
    });

    QRect screen_geometry = this->screen()->geometry();
    dialog.move(
        (screen_geometry.width() - dialog.width()) / 2,
        (screen_geometry.height() - dialog.height()) / 2);

    dialog.exec();

    // По закрытию диалога убираем блюр и overlay
    this->setGraphicsEffect(nullptr);
    overlay->deleteLater();
}