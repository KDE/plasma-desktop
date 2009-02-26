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
        "Enable",this,SLOT(Enable(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowHelp",this,SLOT(ShowHelp(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowPreedit",this,SLOT(ShowPreedit(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowAux",this,SLOT(ShowAux(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowLookupTable",this,SLOT(ShowLookupTable(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateLookupTable",this,SLOT(UpdateLookupTable(const QStringList &,
        const QStringList &,const QStringList &,int,int,int,bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdatePreeditCaret",this,SLOT(UpdatePreeditCaret(int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdatePreeditText",this,SLOT(UpdatePreeditText(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateAux",this,SLOT(UpdateAux(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateSpotLocation",this,SLOT(UpdateSpotLocation(int,int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateScreen",this,SLOT(UpdateScreen(int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateProperty",this,SLOT(UpdateProperty(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateHelperProperty",this,SLOT(UpdateHelperProperty(int,const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateFactoryInfo",this,SLOT(UpdateFactoryInfo(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowFactoryMenu",this,SLOT(ShowFactoryMenu(const QStringList &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "RegisterHelper",this,SLOT(RegisterHelper(int,const QString &,const QString &,
            const QString &,const QString &,unsigned int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "RegisterHelperProperties",this,SLOT(RegisterHelperProperties(int,const QStringList &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "RemoveHelper",this,SLOT(RemoveHelper(int)));
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

void PanelAgent::ShowHelp(const QString &help)
{
    kDebug(0)<<help;
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
    int,int,int,bool to_show)
{
    kDebug(0)<<labels<<candis;
}

void PanelAgent::UpdatePreeditCaret(int position)
{
    kDebug(0)<<position;
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

void PanelAgent::UpdateProperty(const QString &prop)
{
    kDebug(0)<<prop;
}

void PanelAgent::UpdateHelperProperty(int id,const QString &prop)
{
    kDebug(0)<<id<<prop;
}

void PanelAgent::UpdateFactoryInfo(const QString &info)
{
    kDebug(0)<<info;
}
 
void PanelAgent::ShowFactoryMenu(const QStringList &infos)
{
    kDebug(0)<<infos;
}

void PanelAgent:: RegisterHelper(int id,const QString &uuid,const QString &name,
        const QString &icon,const QString &description, unsigned int option)
{
    kDebug(0)<<id<<uuid<<name<<icon<<description<<option;
}

void PanelAgent:: RegisterHelperProperties(int id,const QStringList &props)
{
    kDebug(0)<<id<<props;
}

void PanelAgent:: RemoveHelper(int id)
{
    kDebug(0)<<id;
}

#include "panelagent.moc"
