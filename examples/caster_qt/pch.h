#ifndef PCH_H
#define PCH_H

#ifdef __clang__
    #pragma clang diagnostic push
#elif _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4512)
    #pragma warning(disable: 4510)
    #pragma warning(disable: 4610)
    #pragma warning(disable: 4458)
    #pragma warning(disable: 4800)
#endif

#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <Qt3DCore/Qt3DCore>
#include <Qt3DRender/Qt3DRender>
#include <Qt3DExtras/Qt3DExtras>

#ifdef __clang__
    #pragma clang diagnostic pop
#elif _MSC_VER
    #pragma warning(pop)
#endif

#endif
