#include "window.h"

Window::Window() {
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
        "font-size: 16px; color: #FFFFFF; letter-spacing: 2px; word-spacing: "
        "2px");
    generated_text_->setAlignment(Qt::AlignCenter);
    generated_text_->setWordWrap(true);
    generated_text_->setFixedWidth(kTextFieldWidth);
    generated_text_->setSizePolicy(QSizePolicy::Preferred,
                                   QSizePolicy::Expanding);
    generated_text_->setMinimumHeight(kTextFieldMinimumHeigth);

    auto open_button = new QPushButton("Сгенерировать текст", this);
    open_button->setFixedSize(kButtonWidth, kButtonHeight);
    open_button->setStyleSheet(button_style);
    connect(open_button, &QPushButton::clicked, this, &Window::Prompt);

    auto language_button = new QPushButton("Выбрать язык", this);
    language_button->setFixedSize(kButtonWidth, kButtonHeight);
    language_button->setStyleSheet(button_style);
    connect(language_button, &QPushButton::clicked, this,
            &Window::ShowLanguageDialog);

    auto enable_typing_button = new QPushButton("Начать печатать", this);
    enable_typing_button->setFixedSize(kButtonWidth, kButtonHeight);
    enable_typing_button->setStyleSheet(button_style);
    connect(enable_typing_button, &QPushButton::clicked, this,
            &Window::EnableTyping);

    auto disable_typing_button = new QPushButton("Закончить печатать", this);
    disable_typing_button->setFixedSize(kButtonWidth, kButtonHeight);
    disable_typing_button->setStyleSheet(button_style);
    connect(disable_typing_button, &QPushButton::clicked, this,
            &Window::DisableTyping);

    auto spacing_spin_box = new QSpinBox(this);
    spacing_spin_box->setRange(0, kMaxLetterSpacing);
    spacing_spin_box->setSingleStep(1);
    spacing_spin_box->setValue(2);
    spacing_spin_box->setFixedWidth(kSpinBoxWidth);
    spacing_spin_box->setFocusPolicy(Qt::NoFocus);
    connect(spacing_spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &Window::UpdateLetterSpacing);

    auto word_spacing_spin_box = new QSpinBox(this);
    word_spacing_spin_box->setRange(0, kMaxWordSpacing);
    word_spacing_spin_box->setSingleStep(1);
    word_spacing_spin_box->setValue(2);
    word_spacing_spin_box->setFixedWidth(kSpinBoxWidth);
    word_spacing_spin_box->setFocusPolicy(Qt::NoFocus);
    connect(word_spacing_spin_box, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Window::UpdateWordSpacing);

    auto font_combo_box = new QFontComboBox(this);
    font_combo_box->setFixedWidth(kFontButtonWidth);
    font_combo_box->setFocusPolicy(Qt::NoFocus);
    connect(font_combo_box, &QFontComboBox::currentFontChanged, this,
            &Window::UpdateFont);

    auto weight_spin_box = new QSpinBox(this);
    weight_spin_box->setRange(kMinFontWeight, kMaxFontWeight);
    weight_spin_box->setSingleStep(kFontWeightStep);
    weight_spin_box->setValue(kDefaultFontWeight);
    weight_spin_box->setFixedWidth(kSpinBoxWidth);
    weight_spin_box->setFocusPolicy(Qt::NoFocus);
    connect(weight_spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &Window::UpdateFontWeight);

    statusLabel_ = new QLabel("RAW WPM: 0 | Точность: 100% | WPM: 0", this);
    statusLabel_->setAlignment(Qt::AlignBottom | Qt::AlignCenter);
    statusLabel_->setStyleSheet(
        "font-size: 20px; color: #FFFFFF; letter-spacing: 2px; word-spacing: "
        "2px");

    auto load_text_button = new QPushButton("Загрузить текст", this);
    load_text_button->setFixedSize(kButtonWidth, kButtonHeight);
    load_text_button->setStyleSheet(button_style);
    connect(load_text_button, &QPushButton::clicked, this,
            &Window::LoadTextFromFile);

    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(language_button);
    button_layout->addWidget(open_button);
    button_layout->addWidget(enable_typing_button);
    button_layout->addWidget(disable_typing_button);
    button_layout->addWidget(load_text_button);
    button_layout->setAlignment(Qt::AlignCenter);
    button_layout->setSpacing(kLayoutSpacing);

    auto color_button = new QPushButton("Цвет", this);
    color_button->setFixedSize(kColorButtonWidth, kColotButtonHeight);
    connect(color_button, &QPushButton::clicked, this,
            &Window::ChooseTextColor);

    auto font_size_spin_box = new QSpinBox(this);
    font_size_spin_box->setRange(kMinFontSize, kMaxFontSize);
    font_size_spin_box->setValue(kDefaultFontSize);
    font_size_spin_box->setFixedWidth(kSpinBoxWidth);
    font_size_spin_box->setFocusPolicy(Qt::NoFocus);
    connect(font_size_spin_box, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Window::UpdateFontSize);

    auto line_height_spin_box = new QSpinBox(this);
    line_height_spin_box->setRange(kMinLineHeight, kMaxLineHeight);
    line_height_spin_box->setSingleStep(kLineHeightStep);
    line_height_spin_box->setValue(lineHeight_);
    line_height_spin_box->setFixedWidth(kSpinBoxWidth);
    line_height_spin_box->setFocusPolicy(Qt::NoFocus);
    connect(
        line_height_spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this,
        &Window::
            UpdateLineHeight);	// explicit overloading for int with a proper slot when signal is triggered

    auto settings_layout = new QHBoxLayout();
    settings_layout->addWidget(new QLabel("Отступ(буквы):", this));
    settings_layout->addWidget(spacing_spin_box);
    settings_layout->addWidget(new QLabel("Отступ(слова):", this));
    settings_layout->addWidget(word_spacing_spin_box);
    settings_layout->addWidget(new QLabel("Шрифт:", this));
    settings_layout->addWidget(font_combo_box);
    settings_layout->addWidget(new QLabel("Размер шрифта:", this));
    settings_layout->addWidget(font_size_spin_box);
    settings_layout->addWidget(new QLabel("Жирность:", this));
    settings_layout->addWidget(weight_spin_box);
    settings_layout->addWidget(new QLabel("Межстрочный интервал:", this));
    settings_layout->addWidget(line_height_spin_box);
    settings_layout->addWidget(color_button);
    settings_layout->setAlignment(Qt::AlignCenter);
    settings_layout->setSpacing(kLayoutSpacing);

    auto main_layout = new QVBoxLayout(this);
    main_layout->addLayout(button_layout);
    main_layout->addLayout(settings_layout);
    main_layout->addWidget(generated_text_, 0, Qt::AlignCenter);
    main_layout->addWidget(statusLabel_);
    main_layout->setAlignment(Qt::AlignTop);
    main_layout->setSpacing(kMainLayoutSpacing);

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
    generated_text_->setStyleSheet(QString("font-size: %1px; "
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
                                       .arg(lineHeight_));
}  // gathering and applying all parameters (1 can be reset if 2-nd is changed separately)


void Window::UpdateLetterSpacing(int spacing) {
    letterSpacing_ = spacing;
    ApplyTextStyles();
}  // distance between symbols

void Window::UpdateWordSpacing(int spacing) {
    wordSpacing_ = spacing;
    ApplyTextStyles();
}  // distance between words

void Window::UpdateFont(const QFont& font) {
    currentFont_ = font;
    generated_text_->setFont(currentFont_);
}  // setting font


void Window::UpdateFontWeight(int weight) {
    fontWeight_ = weight;
    ApplyTextStyles();
}  // updating font weigth(boldness)

void Window::ChooseTextColor() {
    QColor chosen_color =
        QColorDialog::getColor(textColor_, this, "Выберите цвет текста");
    if (chosen_color.isValid()) {
        textColor_ = chosen_color;
        ApplyTextStyles();
    }
}  // allowing to use any color for text while typing

void Window::UpdateFontSize(int size) {
    fontSize_ = size;
    ApplyTextStyles();
}  // setting font size

void Window::UpdateLineHeight(int height) {
    lineHeight_ = height;
    ApplyTextStyles();
}  // setting line height

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
