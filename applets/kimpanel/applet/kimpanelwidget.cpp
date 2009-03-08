#include "kimpanelwidget.h"

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

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include <math.h>
#include "paintutils.h"

KIMPanelWidget::KIMPanelWidget(QGraphicsItem *parent)
  : QGraphicsWidget(parent),
    m_layout(0),
    m_collapsed(false),
    m_enableCollapse(true),
    m_panel_agent(0),
    m_empty(true)
{

//X     setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed,QSizePolicy::DefaultType);
    // Initalize background
    m_background = new Plasma::FrameSvg();
    m_background->setImagePath("widgets/panel-background");

    // Initialize layout
//X     m_layout = new QGraphicsGridLayout(this);
    m_layout = new KIMPanelLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // Initial collapse action
    m_collapseAction = new QAction(KIcon("arrow-up-double"),i18n("Collapse/Expand"),this);
    connect(m_collapseAction, SIGNAL(triggered()), SLOT(changeCollapseStatus()));
    m_collapseIcon = new Plasma::IconWidget(this);
    m_collapseIcon->setIcon(KIcon("draw-freehand"));
    connect(m_collapseIcon,SIGNAL(clicked()),m_collapseAction,SIGNAL(triggered()));
    m_collapseIcon->show();

    m_panel_agent = new PanelAgent(this);

    connect(m_panel_agent,
        SIGNAL(execDialog(const Property &)),
        SLOT(execDialog(const Property &)));
    connect(m_panel_agent,
        SIGNAL(execMenu(const QList<Property> &)),
        SLOT(execMenu(const QList<Property> &)));
//X     connect(this,SIGNAL(triggerProperty(const QString &)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));

    m_statusbar = new StatusBarWidget(0,QList<QAction *>() << m_collapseAction);

    KAction *action = new KAction(KIcon("view-refresh"),i18n("Reload Config"),this);
    connect(action,SIGNAL(triggered()),m_panel_agent,SIGNAL(ReloadConfig()));
    m_statusbarActions << action;
    m_statusbarActions << m_collapseAction;
    //m_statusbarActions << KStandardAction::aboutApp(this,SLOT(about()),this);
    //m_statusbarActions << KStandardAction::quit(this,SLOT(exit()),this);

    m_statusbar->addActions(m_statusbarActions);
    m_statusbar->setContextMenuPolicy(Qt::ActionsContextMenu);

    m_lookup_table = new LookupTableWidget();
    connect(m_panel_agent,
        SIGNAL(updateSpotLocation(int,int)),
        m_lookup_table,
        SLOT(updateSpotLocation(int,int)));
    connect(m_panel_agent,
        SIGNAL(updateLookupTable(const LookupTable &)),
        m_lookup_table,
        SLOT(updateLookupTable(const LookupTable &)));
    connect(m_panel_agent,
        SIGNAL(updateAux(const QString &,const QList<TextAttribute> &)),
        m_lookup_table,
        SLOT(updateAux(const QString &,const QList<TextAttribute> &)));
    connect(m_panel_agent,
        SIGNAL(showLookupTable(bool)),
        m_lookup_table,
        SLOT(setVisible(bool)));

    // change from true -> false
    m_collapsed = true;
    changeCollapseStatus();

    m_panel_agent->created();

    //setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
}

KIMPanelWidget::~KIMPanelWidget()
{
}

void KIMPanelWidget::setCollapsible(bool b)
{
    if (m_enableCollapse != b) {
        m_enableCollapse = b;
        if (m_icons.size() > 0) {
            if (b) {
                m_icons.at(0)->addIconAction(m_collapseAction);
            } else {
                m_icons.at(0)->removeIconAction(m_collapseAction);
            }
        }
    }
}

QList<QAction *> KIMPanelWidget::contextualActions() const
{
    if (!m_empty) {
        return QList<QAction *>() << m_collapseAction;
    } else {
        return QList<QAction *>();
    }
}

void KIMPanelWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

//    QRect r = rect().toRect();
//    m_background->setElementPrefix("lastelements");

    painter->save();

    //m_background->setElementPrefix(QString());
    //m_background->resizeFrame(contentsRect.size());
    //m_background->paintFrame(painter, contentsRect, contentsRect);

    painter->restore();
}

void KIMPanelWidget::updateProperty(const Property &prop)
{
    if (!m_prop_map.contains(prop.key)) {
        kWarning() << prop.key <<"ignored.";
        return;
    }

    Plasma::IconWidget *prop_icon = m_prop_map.value(prop.key);

    KIcon icon;

    if (!prop.icon.isEmpty()) {
//        icon_pixmap = KIcon(prop.icon).pixmap(KIconLoader::SizeSmall,KIconLoader::SizeSmall,Qt::KeepAspectRatio);
        icon = KIcon(prop.icon);
    } else {
        icon = KIcon(renderText(prop.label).scaled(256,256,Qt::KeepAspectRatio));
//        icon = KIcon(renderText(prop.label));
    }

    prop_icon->setIcon(icon);
    //prop_button->setToolTip(prop.tip);
}

void KIMPanelWidget::registerProperties(const QList<Property> &props)
{
    m_empty = false;
    while (m_layout->count()) {
        QGraphicsLayoutItem *item = m_layout->itemAt(0);
        m_layout->removeAt(0);
        item->graphicsItem()->hide();
    }
    
    m_icons.clear();
    m_prop_map.clear();
    m_props = props;
    Q_FOREACH (const Property &prop, props) {

        KIcon kicon;

        if (!prop.icon.isEmpty()) {
            kicon = KIcon(prop.icon);
        } else {
            kicon = KIcon(renderText(prop.label).scaled(256,256,Qt::KeepAspectRatio));
           // kicon = KIcon(renderText(prop.label));
        }

        Plasma::IconWidget *icon = new Plasma::IconWidget(kicon,"",this);
        m_icons << icon;
        m_prop_map.insert(prop.key, icon);
//X         m_layout->addItem(icon);

        m_icon_mapper.setMapping(icon,prop.key);
        connect(icon,SIGNAL(clicked()),&m_icon_mapper,SLOT(map()));
    }
    
    if (m_enableCollapse) {
        if (m_icons.size() > 0) {
            m_icons.at(0)->addIconAction(m_collapseAction);
        }
        m_icons << m_collapseIcon;
//X         m_layout->addItem(m_collapseIcon);
    }

    m_layout->setItems(m_icons);
    Q_FOREACH (Plasma::IconWidget *icon, m_icons) {
        icon->show();
    }
    emit iconCountChanged(m_icons.size());
//X     kDebug() << m_layout->effectiveSizeHint(Qt::PreferredSize);
    
}

void KIMPanelWidget::execDialog(const Property &prop)
{
    KMessageBox::information(NULL,prop.tip,prop.label);
}

void KIMPanelWidget::execMenu(const QList<Property> &prop_list)
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

void KIMPanelWidget::changeCollapseStatus()
{
    m_collapsed = !m_collapsed;
    if (m_collapsed) {
        m_collapseAction->setIcon(KIcon("arrow-down-double"));
        m_collapseIcon->setIcon(KIcon("draw-freehand"));

        connect(m_panel_agent,
            SIGNAL(registerProperties(const QList<Property> &)),
            m_statusbar,
            SLOT(registerProperties(const QList<Property> &)));
        connect(m_panel_agent,
            SIGNAL(updateProperty(const Property &)),
            m_statusbar,
            SLOT(updateProperty(const Property &)));
        connect(m_statusbar,SIGNAL(triggerProperty(const QString &)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));

        while (m_layout->count()) {
            QGraphicsLayoutItem *item = m_layout->itemAt(0);
            m_layout->removeAt(0);
            item->graphicsItem()->hide();
        }
        disconnect(&m_icon_mapper, SIGNAL(mapped(const QString &)), 
            m_panel_agent,
            SIGNAL(TriggerProperty(const QString &)));
        disconnect(m_panel_agent,
            SIGNAL(registerProperties(const QList<Property> &)),
            this,
            SLOT(registerProperties(const QList<Property> &)));
        disconnect(m_panel_agent,
            SIGNAL(updateProperty(const Property &)),
            this,
            SLOT(updateProperty(const Property &)));

        m_statusbar->registerProperties(m_props);
        m_statusbar->show();

        m_icons.clear();
        m_icons << m_collapseIcon;
        m_layout->setItems(m_icons);
        m_collapseIcon->show();
//X         m_collapseIcon->hide();
        emit iconCountChanged(m_icons.size());


    } else {
        m_collapseAction->setIcon(KIcon("arrow-up-double"));
        m_collapseIcon->setIcon(KIcon("arrow-up-double"));

        m_statusbar->hide();

        connect(m_panel_agent,
            SIGNAL(registerProperties(const QList<Property> &)),
            this,
            SLOT(registerProperties(const QList<Property> &)));
        connect(m_panel_agent,
            SIGNAL(updateProperty(const Property &)),
            this,
            SLOT(updateProperty(const Property &)));
        connect(&m_icon_mapper, SIGNAL(mapped(const QString &)), 
            m_panel_agent,
            SIGNAL(TriggerProperty(const QString &)));

        registerProperties(m_statusbar->registeredProperties());

        disconnect(m_panel_agent,
            SIGNAL(registerProperties(const QList<Property> &)),
            m_statusbar,
            SLOT(registerProperties(const QList<Property> &)));
        disconnect(m_panel_agent,
            SIGNAL(updateProperty(const Property &)),
            m_statusbar,
            SLOT(updateProperty(const Property &)));
        disconnect(m_statusbar,SIGNAL(triggerProperty(const QString &)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));

    }
    emit collapsed(m_collapsed);
}

#include "kimpanelwidget.moc"

// vim: sw=4 sts=4 et tw=100
