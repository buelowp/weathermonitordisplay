#include <QtCore/QtCore>
#include <QtGui/QtGui>

#include "primarydisplay.h"

int main(int argc, char *argv[])
{
    QApplication app (argc, argv);
    PrimaryDisplay display;
    
    QCursor cursor(Qt::BlankCursor);
    app.setOverrideCursor(cursor);
    app.changeOverrideCursor(cursor);
    
    display.setGeometry(100, 100, 800, 480);
//    display.showFullScreen();
    display.show();
    return app.exec();
}

