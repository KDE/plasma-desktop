#include "kimstatusbargraphics.h"

#include <KConfigDialog>
#include <KMessageBox>
#include <KDesktopFile>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QDrag>
#include <QMenu>
#include <QMouseEvent>
#include <QMimeData>
#include <QToolButton>

#include <KAction>
#include <KDialog>
#include <KMimeType>
#include <KStandardDirs>
#include <KWindowSystem>
#include <KMessageBox>
#include <KMenu>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include <math.h>
#include "paintutils.h"
//#include "kimpanelagent.h"

KIMStatusBarGraphics::KIMStatusBarGraphics(PanelAgent *agent, QGraphicsItem *parent)
  : QGraphicsWidget(parent),
    m_layout(0),
    m_collapsed(false),
    m_enableCollapse(false),
    m_icon_mapper(new QSignalMapper(this)),
    m_empty(true),
    m_logoVisible(false),
    m_panel_agent(agent)
{

    setContentsMargins(0,0,0,0);

    // Initalize background
    m_background = new Plasma::FrameSvg();
    m_background->setImagePath("widgets/panel-background");

    // Initialize layout
//X     m_layout = new QGraphicsGridLayout(this);
    m_layout = new KIMPanelLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    connect(m_icon_mapper,SIGNAL(mapped(const QString &)),
            this,SIGNAL(triggerProperty(const QString &)));

    m_logoIcon = new Plasma::IconWidget(this);
    m_logoIcon->setIcon(KIcon("draw-freehand"));
    m_logoIcon->hide();

    m_collapseAction = new QAction(KIcon("arrow-up-double"),i18n("Expand out"),this);
    connect(m_collapseAction, SIGNAL(triggered()), SLOT(changeCollapseStatus()));
    m_collapseIcon = new Plasma::IconWidget(this);
    m_collapseIcon->setIcon(m_collapseAction->icon());
    connect(m_collapseIcon,SIGNAL(clicked()),m_collapseAction,SIGNAL(triggered()));
    m_collapseIcon->hide();

    m_reloadConfigAction = new QAction(KIcon("view-refresh"),i18n("Reload Config"),this);
//X     connect(m_reloadConfigAction,SIGNAL(triggered()),agent,SIGNAL(ReloadConfig()));

    // change from true -> false
    //m_collapsed = true;
    //changeCollapseStatus();

    //setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    if (m_panel_agent) {
        connect(m_panel_agent,
            SIGNAL(registerProperties(const QList<Property> &)),
            this,
            SLOT(registerProperties(const QList<Property> &)));
        connect(m_panel_agent,
            SIGNAL(updateProperty(const Property &)),
            this,
            SLOT(updateProperty(const Property &)));
        connect(this,
            SIGNAL(triggerProperty(const QString &)),
            m_panel_agent,
            SIGNAL(TriggerProperty(const QString &)));

        connect(m_panel_agent,
            SIGNAL(execDialog(const Property &)),
            this,
            SLOT(execDialog(const Property &)));
        connect(m_panel_agent,
            SIGNAL(execMenu(const QList<Property> &)),
            this,
            SLOT(execMenu(const QList<Property> &)));

        m_panel_agent->created();
    }
}

KIMStatusBarGraphics::~KIMStatusBarGraphics()
{
}

void KIMStatusBarGraphics::setCollapsible(bool b)
{
    if (m_enableCollapse != b) {
        m_enableCollapse = b;
        registerProperties(m_props);
    }
}

void KIMStatusBarGraphics::showLogo(bool b)
{
    if (m_logoVisible != b) {
        m_logoVisible = b;
        registerProperties(m_props);
    }
}

QList<QAction *> KIMStatusBarGraphics::actions() const
{
    QList<QAction *> result;

    if (m_enableCollapse) {
        result << m_collapseAction;
    }

    return result;
}

void KIMStatusBarGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void KIMStatusBarGraphics::updateProperty(const Property &prop)
{
    if (!m_prop_map.contains(prop.key)) {
        return;
    }

    Plasma::IconWidget *prop_icon = m_prop_map.value(prop.key);

    KIcon icon;

    if (!prop.icon.isEmpty()) {
//        icon_pixmap = KIcon(prop.icon).pixmap(KIconLoader::SizeSmall,KIconLoader::SizeSmall,Qt::KeepAspectRatio);
        icon = KIcon(prop.icon);
    } else {
        icon = KIcon(KIM::renderText(prop.label,KIM::Statusbar).scaled(256,256,Qt::KeepAspectRatio));
//        icon = KIcon(renderText(prop.label));
    }

    prop_icon->setIcon(icon);
    //prop_button->setToolTip(prop.tip);
}

void KIMStatusBarGraphics::registerProperties(const QList<Property> &props)
{
    while (m_layout->count()) {
        QGraphicsLayoutItem *item = m_layout->itemAt(0);
        m_layout->removeAt(0);
        item->graphicsItem()->hide();
    }
    
    m_icons.clear();
    if (m_logoVisible) {
        m_icons << m_logoIcon;
    }
    m_prop_map.clear();
    m_props = props;
    Q_FOREACH (const Property &prop, props) {

        KIcon kicon;

        if (!prop.icon.isEmpty()) {
            kicon = KIcon(prop.icon);
        } else {
            kicon = KIcon(KIM::renderText(prop.label,KIM::Statusbar).scaled(256,256,Qt::KeepAspectRatio));
           // kicon = KIcon(renderText(prop.label));
        }

        Plasma::IconWidget *icon = new Plasma::IconWidget(kicon,"",this);
        m_icons << icon;
        m_prop_map.insert(prop.key, icon);
//X         m_layout->addItem(icon);

        connect(icon,SIGNAL(clicked()),m_icon_mapper,SLOT(map()));
        m_icon_mapper->setMapping(icon,prop.key);
    }
    
    if (m_enableCollapse) {
        m_icons << m_collapseIcon;
    }

    m_layout->setItems(m_icons);
    Q_FOREACH (Plasma::IconWidget *icon, m_icons) {
        icon->show();
    }
    emit iconCountChanged();
//X     kDebug() << m_layout->effectiveSizeHint(Qt::PreferredSize);
    
}

void KIMStatusBarGraphics::changeCollapseStatus()
{
    m_collapsed = !m_collapsed;
    if (m_collapsed) {
        m_collapseAction->setIcon(KIcon("arrow-down-double"));
        m_collapseAction->setText(i18n("Collapse to panel"));
        m_collapseIcon->setIcon(m_collapseAction->icon());
    } else {
        m_collapseAction->setIcon(KIcon("arrow-up-double"));
        m_collapseAction->setText(i18n("Expand out"));
        m_collapseIcon->setIcon(m_collapseAction->icon());

    }
    emit collapsed(m_collapsed);
}

void KIMStatusBarGraphics::execDialog(const Property &prop) 
{
    KMessageBox::information(NULL,prop.tip,prop.label);
}

void KIMStatusBarGraphics::execMenu(const QList<Property> &prop_list)
{
    QMenu *menu = new QMenu();
    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper,SIGNAL(mapped(const QString&)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));
    foreach (const Property &prop, prop_list) {
        QAction *action = new QAction(QIcon(prop.icon),prop.label,this);
        mapper->setMapping(action,prop.key);
        connect(action,SIGNAL(triggered()),mapper,SLOT(map()));
        menu->addAction(action);
    }
    menu->exec(QCursor::pos());
    delete menu;
}

// vim: sw=4 sts=4 et tw=100
