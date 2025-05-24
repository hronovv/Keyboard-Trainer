#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSpinBox>
#include <QFontComboBox>
#include <QPushButton>
#include <QList>
#include "database.h"

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(Database &db, QString username, QWidget *parent = nullptr);
    void loadSettings();
    void setUsername(const QString& username);

    signals:
        void settingsChanged(const UserSettings& settings);

    private slots:
        void saveSettings();
    void cancel();

private:
    Database &database_;
    QString username_;

    QSpinBox *letterSpacingSpinBox_;
    QSpinBox *wordSpacingSpinBox_;
    QFontComboBox *fontComboBox_;
    QSpinBox *fontSizeSpinBox_;
    QSpinBox *fontWeightSpinBox_;
    QSpinBox *lineHeightSpinBox_;
    QPushButton *colorButton_;
    QColor currentColor_;

    QPushButton *saveButton_;
    QPushButton *cancelButton_;


    QList<QPushButton*> caretSmoothButtons_;
    QList<QPushButton*> caretStyleButtons_;

    void applySettingsToUI(const UserSettings &settings);
    UserSettings getSettingsFromUI() const;
    QString getCheckedButtonValue(const QList<QPushButton*> &buttons) const;
};

#endif // SETTINGSWIDGET_H