#include "fontselector.h"
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>
#include <QFontDatabase>
#include <QDialog>
#include <QVBoxLayout>

FontSelector::FontSelector(QWidget* parent) : QWidget(parent) {
    button_ = new QPushButton("Select font", this);
    button_->setMinimumWidth(200);
    button_->setStyleSheet(R"(
        QPushButton {
            background-color: #3b4252;
            border: 1px solid #4c566a;
            border-radius: 5px;
            color: #eceff4;
            padding: 6px 8px;
            font-size: 14px;
            min-height: 36px;
            text-align: left;
        }
        QPushButton:hover {
            border: 1px solid #81a1c1;
        }
    )");
    connect(button_, &QPushButton::clicked, this, &FontSelector::showFontDialog);
}

QString FontSelector::currentFont() const {
    return selectedFont_;
}

void FontSelector::setCurrentFont(const QString& fontFamily) {
    selectedFont_ = fontFamily;
    button_->setText(fontFamily);
}

void FontSelector::showFontDialog() {
    QDialog dialog(this, Qt::Popup | Qt::FramelessWindowHint);
    dialog.setStyleSheet(R"(
        QListView {
            background-color: #3b4252;
            border: 1px solid #4c566a;
            color: #eceff4;
        }
        QListView::item:selected {
            background-color: #81a1c1;
            color: #2e3440;
        }
    )");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0,0,0,0);

    QListView* listView = new QListView(&dialog);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setUniformItemSizes(true);
    listView->setMinimumWidth(220);
    listView->setMinimumHeight(300);

    QStandardItemModel* model = new QStandardItemModel(listView);
    QFontDatabase fontDb;
    for (const QString& family : fontDb.families()) {
        model->appendRow(new QStandardItem(family));
    }
    listView->setModel(model);

    QModelIndex currentIndex;
    for (int i=0; i < model->rowCount(); ++i) {
        if (model->item(i)->text() == selectedFont_) {
            currentIndex = model->index(i, 0);
            break;
        }
    }
    if (currentIndex.isValid()) {
        listView->setCurrentIndex(currentIndex);
        listView->scrollTo(currentIndex, QAbstractItemView::PositionAtCenter);
    }

    layout->addWidget(listView);

    connect(listView, &QListView::clicked, [&](const QModelIndex &index){
        if (!index.isValid())
            return;
        selectedFont_ = model->item(index.row())->text();
        button_->setText(selectedFont_);
        emit fontChanged(selectedFont_);
        dialog.accept();
    });

    QPoint pos = button_->mapToGlobal(QPoint(0, button_->height()));
    dialog.move(pos);
    dialog.exec();
}