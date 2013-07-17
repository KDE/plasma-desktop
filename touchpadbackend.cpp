#include "touchpadbackend.h"

#include <QtGlobal>

#include "xlibbackend.h"

Q_GLOBAL_STATIC(XlibBackend, backend)

TouchpadBackend::TouchpadBackend(QObject *parent) : QObject(parent)
{
}

TouchpadBackend *TouchpadBackend::self()
{
    return backend()->test() ? backend() : 0;
}
