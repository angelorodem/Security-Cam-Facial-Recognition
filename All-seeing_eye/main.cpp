#include <QApplication>
#include <QDir>
#include <QDebug>
#include "gui.h"
//#include "alg.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    GUI gui;
    gui.show();
    /*Alg a;
    a.exec();*/
    return app.exec();
}
