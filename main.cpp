#include <QApplication>
#include <QPushButton>
#include "window.h"


int main(int argc, char* argv[]) {
    Database db;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    QApplication a(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    auto* window = new Window(db);
    window->setWindowTitle("Keyboard Trainer");
    window->resize(kWindowSize, kWindowSize);
    window->show();
    curl_global_cleanup();
    return a.exec();
}

