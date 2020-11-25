#include "caster.h"

#ifdef Clarius_BUILD
#define Clarius_IMPORT_QT_WIDGETS_LIB
#define Clarius_IMPORT_QT_NETWORK_LIB
#include <cus/qtplugins.h>
#endif

#ifdef CLARIUS_LISTEN_STATIC
Q_IMPORT_PLUGIN(Cast)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Caster caster;
    caster.show();
    return a.exec();
}
