#ifndef FONTSELECTOR_H
#define FONTSELECTOR_H

#include <QWidget>

class QPushButton;

class FontSelector : public QWidget {
    Q_OBJECT
public:
    explicit FontSelector(QWidget* parent = nullptr);

    QString currentFont() const;
    void setCurrentFont(const QString& fontFamily);

    signals:
        void fontChanged(const QString& font);

    private slots:
        void showFontDialog();

private:
    QPushButton* button_;
    QString selectedFont_;
};

#endif // FONTSELECTOR_H