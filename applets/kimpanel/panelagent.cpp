#include "panelagent.h"
#include <KDebug>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

PanelAgent::PanelAgent(QObject *parent)
    : QObject(parent)
{
    // constructor
    QDBusConnection::connectToBus(QDBusConnection::SessionBus,"kimpanel_bus").registerObject("/org/kde/impanel", this);
    QDBusConnection::connectToBus(QDBusConnection::SessionBus,"kimpanel_bus").registerService("org.kde.impanel");
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowPreedit",this,SLOT(ShowPreedit(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowAux",this,SLOT(ShowAux(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowLookupTable",this,SLOT(ShowLookupTable(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateAux",this,SLOT(UpdateAux(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdatePreeditText",this,SLOT(UpdatePreeditText(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateLookupTable",this,SLOT(UpdateLookupTable(const QStringList &,
        const QStringList &,const QStringList &,int,int,int,bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateSpotLocation",this,SLOT(UpdateSpotLocation(int,int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateScreen",this,SLOT(UpdateScreen(int)));
}

PanelAgent::~PanelAgent()
{
    // destructor
    QDBusConnection::disconnectFromBus("kimpanel_bus");
}

void PanelAgent::Enable(bool to_enable)
{
    kDebug(0)<<to_enable;
}

void PanelAgent::ShowHelp()
{
    kDebug(0)<<".";
}

void PanelAgent::ShowPreedit(bool to_show)
{
    kDebug(0)<<to_show;
}

void PanelAgent::ShowAux(bool to_show)
{
    kDebug(0)<<to_show;
}

void PanelAgent::ShowLookupTable(bool to_show)
{
    kDebug(0)<<to_show;
}

void PanelAgent::UpdateLookupTable(const QStringList &labels,
    const QStringList &candis,
    const QStringList &attrlists,
    int,int,int,bool)
{
    kDebug(0)<<labels<<candis;
}

void PanelAgent::UpdatePreeditText(const QString &text)
{
    kDebug(0)<<text;
}

void PanelAgent::UpdateAux(const QString &text)
{
    kDebug(0)<<text;
}

void PanelAgent::UpdateSpotLocation(int x,int y)
{
    kDebug(0)<<x<<y;
}

void PanelAgent::UpdateScreen(int screen_id)
{
    kDebug(0)<<screen_id;
}

#include "panelagent.moc"
