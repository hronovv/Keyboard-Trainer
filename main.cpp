#include <QApplication>
#include <QPushButton>
#include "window.h"


int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    QApplication a(argc, argv);
    auto* window = new Window();
    window->setWindowTitle("Keyboard Trainer");
    window->resize(kWindowSize, kWindowSize);
    window->show();
    curl_global_cleanup();
    return a.exec();
}
