#include "xform.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XForm w;
    w.show();
    return a.exec();
}
