#include "touchpadbackend.h"

#include <QtGlobal>

#include "xlibbackend.h"

Q_GLOBAL_STATIC(XlibBackend, backend)

TouchpadBackend::TouchpadBackend(QObject *parent)
    : QObject(parent), m_triedInit(false), m_initResult(false)
{
}

TouchpadBackend *TouchpadBackend::self()
{
    return backend();
}

void TouchpadBackend::applyConfig(const TouchpadParameters *p)
{
    if (!tryInit()) {
        return;
    }
    internalApplyConfig(p);
}

void TouchpadBackend::getConfig(TouchpadParameters *p)
{
    if (!tryInit()) {
        return;
    }
    internalGetConfig(p);
}

QStringList TouchpadBackend::supportedParameters()
{
    if (!tryInit()) {
        return QStringList();
    }
    return internalSupportedParameters();
}

bool TouchpadBackend::tryInit()
{
    if (!m_triedInit) {
        m_triedInit = true;
        m_initResult = init();
    }
    return m_initResult;
}
