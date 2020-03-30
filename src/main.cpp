#include <QtCore/QtCore>
#include <QtGui/QtGui>

#include "primarydisplay.h"

int main(int argc, char *argv[])
{
    QApplication app (argc, argv);
    PrimaryDisplay display;
    
    display.setGeometry(100, 100, 640, 480);
    display.show();
    return app.exec();
}

