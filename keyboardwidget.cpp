#include "keyboardwidget.h"

#include <iostream>
#include <QGridLayout>
#include <QFile>
#include <QJsonObject>

KeyboardWidget::KeyboardWidget(QWidget *parent) : QWidget(parent) {
    layout_ = new QGridLayout(this);
    layout_->setSpacing(4);
    layout_->setContentsMargins(0, 0, 0, 0);
}

bool KeyboardWidget::loadLayoutFromJson(const QString &jsonFilePath) {
    clearKeyboard();
    QFile file(jsonFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << jsonFilePath;
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    // Получаем имя раскладки без пути и расширения
    QFileInfo fileInfo(jsonFilePath);
    keyboardLayoutName_ = fileInfo.completeBaseName();  // например "russian"

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Expected JSON root to be an object.";
        return false;
    }

    QJsonObject rootObj = doc.object();

    if (!rootObj.contains("keys") || !rootObj["keys"].isObject()) {
        qWarning() << "No 'keys' object in JSON.";
        return false;
    }

    QJsonObject keysObj = rootObj["keys"].toObject();

    QStringList rowOrder = {"row1", "row2", "row3", "row4", "row5"};

    const int rowIndents[] = {0, 1, 1, 2, 3};

    int rowNum = 0;
    for (const QString &rowName : rowOrder) {
        if (!keysObj.contains(rowName)) {
            rowNum++;
            continue;
        }

        QJsonValue rowVal = keysObj[rowName];
        if (!rowVal.isArray()) {
            rowNum++;
            continue;
        }

        if (rowName == "row5") {
            int colNum = rowIndents[rowNum];

            // Используем название раскладки вместо пробела
            createKeyButton(keyboardLayoutName_, rowNum, colNum, 7);

            rowNum++;
            continue;
        }

        QJsonArray keysArray = rowVal.toArray();

        int colNum = rowIndents[rowNum];

        for (const QJsonValue &keyVal : keysArray) {
            if (keyVal.isString()) {
                QString label = keyVal.toString();
                createKeyButton(label, rowNum, colNum);
                colNum++;
            } else if (keyVal.isObject()) {
                QJsonObject keyObj = keyVal.toObject();
                QString label = keyObj["label"].toString();
                int colspan = keyObj.contains("width") ? keyObj["width"].toInt() : 1;
                createKeyButton(label, rowNum, colNum, colspan);
                colNum += colspan;
            }
        }
        rowNum++;
    }

    return true;
}

void KeyboardWidget::createKeyButton(const QString &keyLabel, int row, int col, int colspan) {
    QString displayLabel;

    if (keyLabel.toLower() == "space") {
        // Если встречается ключ "space", ставим пустой пробел (для совместимости)
        displayLabel = " ";
    } else if (keyLabel == keyboardLayoutName_) {
        // Если метка совпадает с названием раскладки (на пробеле), показываем это название
        displayLabel = keyLabel;
    } else if (!keyLabel.isEmpty()) {
        QChar lowerChar = keyLabel.at(0).toLower();
        displayLabel = QString(lowerChar);
    } else {
        displayLabel = keyLabel;
    }

    QPushButton *btn = new QPushButton(displayLabel, this);

    const int btnHeight = 43;
    const int btnWidthPerCol = 43;
    const int spacing = layout_->spacing();

    btn->setFixedHeight(btnHeight);

    int btnWidth = btnWidthPerCol * colspan + spacing * (colspan - 1);
    btn->setFixedWidth(btnWidth);

    btn->setStyleSheet("background-color: #3b4252; color: #d8dee9; border-radius: 5px; font-weight: bold; font-size: 12px;");

    QFont font = btn->font();
    font.setPointSize(12);
    font.setBold(true);
    btn->setFont(font);

    layout_->addWidget(btn, row, col, 1, colspan);

    if (!keyLabel.isEmpty()) {
        if (keyLabel.toLower() == "space") {
            keyButtons_[' '] = btn;
            connect(btn, &QPushButton::clicked, this, &KeyboardWidget::spaceKeyClicked);
        } else if (keyLabel == keyboardLayoutName_) {
            keyButtons_[' '] = btn;
            connect(btn, &QPushButton::clicked, this, &KeyboardWidget::spaceKeyClicked);
        } else {
            QChar keyChar = keyLabel.at(0).toLower();
            keyButtons_[keyChar] = btn;
        }
    }
}

void KeyboardWidget::highlightKey(QChar key) {
    clearHighlight();

    key = key.toLower();

    if (keyButtons_.contains(key)) {
        QPushButton *btn = keyButtons_[key];
        btn->setStyleSheet("background-color: #88c0d0; color: #2e3440; border-radius: 5px; font-weight: bold;");
        currentlyHighlighted_ = btn;
    }
}

void KeyboardWidget::clearHighlight() {
    if (currentlyHighlighted_) {
        currentlyHighlighted_->setStyleSheet("background-color: #3b4252; color: #d8dee9; border-radius: 5px; "
                                             "font-weight: bold;");
        currentlyHighlighted_ = nullptr;
    }
}

void KeyboardWidget::clearKeyboard() {
    QLayoutItem *child;
    while ((child = layout_->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    keyButtons_.clear();

    currentlyHighlighted_ = nullptr;
}