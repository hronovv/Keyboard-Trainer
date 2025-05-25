#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QGridLayout>
#include <QWidget>
#include <QPushButton>
#include <QMap>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>

class KeyboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyboardWidget(QWidget *parent = nullptr);

    bool loadLayoutFromJson(const QString &jsonFilePath); // Загрузка из JSON
    void highlightKey(QChar key);                           // Подсветить клавишу
    void clearHighlight();                                  // Очистить подсветку
    void clearKeyboard();
    signals:
    void spaceKeyClicked();
private:
    QGridLayout *layout_;
    QMap<QChar, QPushButton*> keyButtons_;

    QPushButton *currentlyHighlighted_ = nullptr;
    QString keyboardLayoutName_;

    void createKeyButton(const QString &keyLabel, int row, int col, int colspan = 1);
};

#endif // KEYBOARDWIDGET_H