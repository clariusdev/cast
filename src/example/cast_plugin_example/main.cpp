#include "caster.h"

#ifdef CLARIUS_CAST_STATIC
Q_IMPORT_PLUGIN(Cast)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Caster caster;
    caster.show();
    return a.exec();
}
