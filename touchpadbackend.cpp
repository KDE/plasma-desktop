#include "touchpadbackend.h"

#include <QtGlobal>
#include <QSharedPointer>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KService>
#include <KServiceTypeTrader>

class BackendSelector
{
public:
    BackendSelector()
    {
        KService::List services =
                KServiceTypeTrader::self()->query("TouchpadKCMBackend");

        Q_FOREACH(KService::Ptr service, services) {
            if (!service) {
                continue;
            }

            KPluginFactory *factory =
                    KPluginLoader(service->library()).factory();

            if (!factory) {
                continue;
            }

            QSharedPointer<TouchpadBackend> backend(
                        factory->create<TouchpadBackend>());

            if (backend && backend->test()) {
                m_selectedBackend = backend;
                break;
            }
        }
    }

    TouchpadBackend *backend() const
    {
        return m_selectedBackend.data();
    }

private:
    QSharedPointer<TouchpadBackend> m_selectedBackend;
};

Q_GLOBAL_STATIC(BackendSelector, backendSelector)

TouchpadBackend::TouchpadBackend(QObject *parent) : QObject(parent)
{
}

TouchpadBackend *TouchpadBackend::self()
{
    return backendSelector()->backend();
}
