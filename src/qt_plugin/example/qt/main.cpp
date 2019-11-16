#include "listener.h"

#ifdef Clarius_BUILD
#define Clarius_IMPORT_QT_WIDGETS_LIB
#define Clarius_IMPORT_QT_NETWORK_LIB
#include <cus/qtplugins.h>
#endif

#ifdef CLARIUS_LISTEN_STATIC
Q_IMPORT_PLUGIN(Listen)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Listener listener;
    listener.show();
    return a.exec();
}
