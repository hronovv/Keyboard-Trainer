#ifndef WINDOW_H
#define WINDOW_H
#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QFontComboBox>
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
#include "QCheckBox"
#include "QColorDialog"
#include "QTimer"
#include "AI json-request/api.h"
#include "src/languages.h"
constexpr int kWindowSize = 1600;
constexpr int kButtonWidth = 220;
constexpr int kButtonHeight = 45;
constexpr int kLanguageChoiceWidth = 400;
constexpr int kLanguageChoiceHeight = 600;
constexpr int kWordsNumber = 100;
constexpr int kDeafultFontWeight = 400;
constexpr int kDefaultFontSize = 16;
constexpr int kDefaultLineHeight = 20;
constexpr int kIntervalMs = 200;
constexpr int kTextFieldWidth = 800;
constexpr int kTextFieldMinimumHeigth = 600;
constexpr int kSpinBoxWidth = 72;
constexpr int kDefaultLetterSpacing = 2;
constexpr int kDefaultWordSpacing = 2;
constexpr int kTextFieldMinimumHeight = 100;
constexpr int kDefaultFontWeight = 400;
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
constexpr int kColotButtonHeight = 30;
constexpr double kWpmCoefficient = 4.25;
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
    Window();

   protected:
    void keyPressEvent(QKeyEvent* event) override;

   private slots:
    void SetLanguage(const QString& language);
    void Prompt();
    void ShowLanguageDialog();
    void EnableTyping();
    void DisableTyping();
    void LoadTextFromFile();

    // visual methods
    void UpdateLetterSpacing(int spacing);
    void UpdateWordSpacing(int spacing);
    void UpdateFont(const QFont& font);
    void UpdateFontWeight(int weight);
    void ChooseTextColor();
    void UpdateFontSize(int size);
    void UpdateLineHeight(int height);

   private:
    //for proper typing
    void ApplyTextStyles();
    void ResetText();
    void StartTypingTimer();
    void StopTypingTimer();
    void UpdateWPM();
    QGraphicsOpacityEffect* effect_;
    QPropertyAnimation* animation_;
    QLabel* generated_text_;
    QLabel* statusLabel_;
    QString prompt_language_;
    QString targetText_;
    QVector<QChar> typedChars_;
    QVector<bool> errorFlags_;
    int currentIndex_ = 0;
    int errorCount_ = 0;
    QTimer* typing_timer_;
    double elapsed_seconds_ = 0;
    int typedCharCount_ = 0;

    // visual methods
    int letterSpacing_ = 2;
    int wordSpacing_ = 2;
    int fontWeight_ = kDeafultFontWeight;
    int fontSize_ = kDefaultFontSize;
    int lineHeight_ = kDefaultLineHeight;
    QFont currentFont_;
    QColor textColor_ = Qt::white;
    bool typing_allowed_ = false;
};

#endif	// WINDOW_H
