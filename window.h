#ifndef WINDOW_H
#define WINDOW_H

// Qt Includes
#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QFontComboBox>
#include <QtSvgWidgets/qsvgwidget.h>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QSpinBox>
#include <QString>
#include <QTextEdit>
#include <QToolButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QTimer>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonObject>
#include <QRandomGenerator>

// Project Includes
#include "AI json-request/api.h"
#include "src/languages.h"
#include "database.h"
#include "logindialog.h"
#include "settingswidget.h"

// Constants
constexpr int kWindowSize = 1600;
constexpr int kButtonWidth = 220;
constexpr int kButtonHeight = 40;

constexpr int kLanguageChoiceWidth = 450;
constexpr int kLanguageChoiceHeight = 600;
constexpr int kWordsNumber = 100;

constexpr int kDefaultLineHeight = 20;
constexpr int kIntervalMs = 200;

constexpr int kTextFieldWidth = 1200;
constexpr int kTextFieldMinimumHeigth = 500;
constexpr int kTextFieldMinimumHeight = 100;

constexpr int kSpinBoxWidth = 72;

constexpr int kDefaultLetterSpacing = 2;
constexpr int kDefaultWordSpacing = 2;

constexpr int kMinFontWeight = 100;
constexpr int kMaxFontWeight = 900;
constexpr int kFontWeightStep = 100;

constexpr int kMinFontSize = 8;
constexpr int kMaxFontSize = 72;

constexpr int kMinLineHeight = 10;
constexpr int kMaxLineHeight = 50;
constexpr int kLineHeightStep = 2;

constexpr int kMaxLetterSpacing = 12;
constexpr int kMaxWordSpacing = 24;

constexpr int kAnimationDurationMs = 400;
constexpr int kTypingIntervalMs = 200;

constexpr double kUpdateIntervalSec = 0.2;
constexpr double kSecondsInMinute = 60.0;

constexpr int kFontButtonWidth = 180;

constexpr int kHundred = 100;

constexpr int kLayoutSpacing = 10;
constexpr int kMainLayoutSpacing = 15;

constexpr int kColorButtonWidth = 100;
constexpr int kColorButtonHeight = 30;

constexpr double kWpmCoefficient = 4.5;

const std::string kPromptTemplatePart1 =
    "Please find a random article about programming(c++, python, variables, "
    "etc.). Summarize the key tips and highlights presented in the article. "
    "The generated text must contain exactly ";

const std::string kPromptTemplatePart2 =
    " words.Avoid very long sentences. Avoid using symbols and signs that do "
    "not relate to the chosen language. The response should contain only the "
    "text, without mentioning the article or its source.This will be followed "
    "by the language in which you will write this text (this is very "
    "important). Just send a text, don't mention how you did it, don't mention "
    "my request.Avoid outputting language name that you are using!";

class Window : public QWidget {
    Q_OBJECT

public:
    explicit Window(Database &db, QWidget *parent = nullptr);

    void LoadUserSettings();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // User interaction
    void SetLanguage(const QString& language);
    void Prompt();
    void ShowLanguageDialog();
    void DisableTyping();
    void LoadTextFromFile();
    void showLoginDialog();
    void ShowSettings();
    void ShowWordSetDialog();

private:
    // Typing related methods
    void ApplyTextStyles();
    void ResetText();
    void StartTypingTimer();
    void StopTypingTimer();
    void UpdateWPM();
    void GenerateNewTextFromWordList();

    // UI elements
    QLabel* generated_text_;
    QLabel* statusLabel_;
    QLabel* usernameLabel_;
    SettingsWidget *settingsWidget_;
    QSvgWidget* accountIconLabel;
    QSvgWidget* settingsIconLabel;

    QGraphicsOpacityEffect* effect_;
    QPropertyAnimation* animation_;

    // Typing state variables
    QString prompt_language_;
    QString targetText_;
    QVector<QChar> typedChars_;
    QVector<bool> errorFlags_;

    int currentIndex_ = 0;
    int errorCount_ = 0;
    int typedCharCount_ = 0;

    QTimer* typing_timer_;
    double elapsed_seconds_ = 0;
    bool typing_allowed_ = false;

    // User session info
    QString currentUsername_;
    Database &database_;

    // Visual & Formatting state
    bool wordsModeActive_ = false;
    QStringList currentWordList_;

    int letterSpacing_ = kDefaultLetterSpacing;
    int wordSpacing_ = kDefaultWordSpacing;
    int fontWeight_;
    int fontSize_;
    int lineHeight_ = kDefaultLineHeight;
    QString caretSmooth_;
    QString caretStyle_;

    QMap<QString, QString> caretMap = {
        { "off", "" },      // пустая строка — каретка выключена
        { "▮", "▮" },       // сплошной блок
        { "▯", "▯" },       // пустой блок
        { "_", "_" }        // нижнее подчеркивание
    };

    QFont currentFont_;
    QColor textColor_ = Qt::white;
};

#endif // WINDOW_H