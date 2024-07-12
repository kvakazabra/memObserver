#include "cmainwindow.h"
#include "utilities.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    Utilities::programDataDirectory(); // init static variable

    QApplication a(argc, argv);
    CMainWindow w;
    w.show();
    return a.exec();
}
