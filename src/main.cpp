#include <QtCore/QtCore>
#include <QtGui/QtGui>

#include "primarydisplay.h"

int main(int argc, char *argv[])
{
    QApplication app (argc, argv);
    PrimaryDisplay display;
    
    display.showFullScreen();
    return app.exec();
}

