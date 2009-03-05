#include "panelagent.h"
#include "impaneladaptor.h"
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

    new ImpanelAdaptor(this);

    QDBusConnection::connectToBus(QDBusConnection::SessionBus,"kimpanel_bus").registerObject("/org/kde/impanel", this);
    QDBusConnection::connectToBus(QDBusConnection::SessionBus,"kimpanel_bus").registerService("org.kde.impanel");

    // directly connect to corresponding signal
    QDBusConnection("kimpanel_bus").connect("","","",
        "Enable",this,SIGNAL(enable(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowPreedit",this,SIGNAL(showPreedit(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowAux",this,SIGNAL(showAux(bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ShowLookupTable",this,SIGNAL(showLookupTable(bool)));

    // do some serialization 
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateLookupTable",this,SLOT(UpdateLookupTable(const QStringList &,
        const QStringList &,const QStringList &,int,int,int,bool)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdatePreeditCaret",this,SLOT(UpdatePreeditCaret(int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdatePreeditText",this,SLOT(UpdatePreeditText(const QString &,const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateAux",this,SLOT(UpdateAux(const QString &,const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "UpdateSpotLocation",this,SIGNAL(updateSpotLocation(int,int)));
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
        "RegisterProperties",this,SLOT(RegisterProperties(const QStringList &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "RegisterHelper",this,SLOT(RegisterHelper(int,
            const QString &,const QString &,
            const QString &,const QString &,unsigned int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "RegisterHelperProperties",this,SLOT(RegisterHelperProperties(int,const QStringList &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "RemoveHelper",this,SLOT(RemoveHelper(int)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ExecDialog",this,SLOT(ExecDialog(const QString &)));
    QDBusConnection("kimpanel_bus").connect("","","",
        "ExecMenu",this,SLOT(ExecMenu(const QStringList &)));
    //emit showFactoryMenu();
}

PanelAgent::~PanelAgent()
{
    // destructor
    QDBusConnection::disconnectFromBus("kimpanel_bus");
}

//---------------------internal function start-----------------------
static QList<TextAttribute> String2AttrList(const QString &str)
{
    QList<TextAttribute> result;
    foreach (const QString &s, str.split(';')) {
        TextAttribute attr;
        QStringList list = str.split(':');
        if (list.size() < 4)
            continue;
        switch (list.at(0).toInt()) {
        case 0:
            attr.type = TextAttribute::None;
            break;
        case 1:
            attr.type = TextAttribute::Decorate;
            break;
        case 2:
            attr.type = TextAttribute::Foreground;
            break;
        case 3:
            attr.type = TextAttribute::Background;
            break;
        default:
            attr.type = TextAttribute::None;
        }
        attr.start = list.at(1).toInt();
        attr.length = list.at(2).toInt();
        attr.value = list.at(3).toInt();
        result << attr;
    }
    return result;
}

static Property String2Property(const QString &str)
{
    Property result;

    QStringList list = str.split(':');

    if (list.size() < 4)
        return result;

    result.key = list.at(0);
    result.label = list.at(1);
    result.icon = list.at(2);
    result.tip = list.at(3);
//X     {
//X         result.state = Property::None;
//X         int n = list.at(4).toInt();
//X         if (n & Property::Active)
//X             result.state &= Property::Active;
//X         if (n & Property::Visible)
//X             result.state &= Property::Visible;
//X     }

    return result;
}

static LookupTable Args2LookupTable(const QStringList &labels, const QStringList &candis, const QStringList &attrs, int, int, int, bool to_show)
{
    Q_ASSERT(labels.size() == candis.size());
    Q_ASSERT(labels.size() == attrs.size());

    LookupTable result;

    for(int i = 0; i < labels.size(); i++) {
        LookupTable::Entry entry;

        entry.label = labels.at(i);
        entry.text = candis.at(i);
        entry.attr = String2AttrList(attrs.at(i));
        
        result.entries << entry;
    }
    
    result.to_show = to_show;
    return result;
}

//---------------------internal function end----------------------

//---------------------handle kimpanel call start-----------------

void PanelAgent::created()
{
    emit PanelCreated();
}

void PanelAgent::exit()
{
    emit Exit();
}

void PanelAgent::reloadConfig()
{
    emit ReloadConfig();
}

//---------------------handle kimpanel call end-----------------------------------------

//---------------------handle dbus signal start-----------------------------------------

void PanelAgent::UpdateLookupTable(const QStringList &labels,
    const QStringList &candis,
    const QStringList &attrlists,
    int i1,int i2,int i3,bool to_show)
{
    emit updateLookupTable(Args2LookupTable(labels,candis,attrlists,i1,i2,i3,to_show));
}

void PanelAgent::UpdatePreeditCaret(int position)
{
    //kDebug(0)<<position;
}

void PanelAgent::UpdatePreeditText(const QString &text,const QString &attr)
{
    //kDebug(0)<<text<<attr;
}

void PanelAgent::UpdateAux(const QString &text,const QString &attr)
{
    emit updateAux(text,String2AttrList(attr));
}

void PanelAgent::UpdateScreen(int screen_id)
{
    kDebug(0)<<screen_id;
}

void PanelAgent::UpdateProperty(const QString &prop)
{
    emit updateProperty(String2Property(prop));
}

void PanelAgent::RegisterProperties(const QStringList &props)
{
    if (cached_props != props) {
        cached_props = props;
        QList<Property> list;
        foreach (const QString &prop, props) {
            list << String2Property(prop);
        }

        emit registerProperties(list);
    } else {
//X         kDebug()<<"cache hit :)";
    }
}

void PanelAgent::ExecDialog(const QString &prop)
{
    emit execDialog(String2Property(prop));
}

void PanelAgent::ExecMenu(const QStringList &entries)
{
    kDebug()<<entries;
    QList<Property> list;
    foreach (const QString &entry, entries) {
        list << String2Property(entry);
    }

    emit execMenu(list);
}

//---------------------handle dbus signal end--------------------------------------
#include "panelagent.moc"
