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

#include <KDialog>
#include <KMimeType>
#include <KStandardDirs>
#include <KWindowSystem>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include <math.h>
#include "paintutils.h"

static const int s_defaultIconSize = 16;
static const int s_defaultSpacing = 0;

KIMPanelWidget::KIMPanelWidget(QGraphicsItem *parent)
  : QGraphicsWidget(parent),
    m_layout(0),
    m_rowCount(2),
    m_panel_agent(0)
{

//X     setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed,QSizePolicy::DefaultType);
    // Initalize background
    m_background = new Plasma::FrameSvg();
    m_background->setImagePath("widgets/panel-background");

    // Initialize layout
//X     m_layout = new QGraphicsGridLayout(this);
    m_layout = new KIMPanelLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // Initial "more icons" arrow
//X     m_arrow = new Plasma::IconWidget(this);
//X     m_arrow->setIcon(KIcon("arrow-right"));
    //connect(m_arrow, SIGNAL(clicked()), SLOT(showDialog()));
    

    m_panel_agent = new PanelAgent(this);

    connect(m_panel_agent,
        SIGNAL(registerProperties(const QList<Property> &)),
        SLOT(registerProperties(const QList<Property> &)));
    connect(m_panel_agent,
        SIGNAL(updateProperty(const Property &)),
        SLOT(updateProperty(const Property &)));
    connect(m_panel_agent,
        SIGNAL(execDialog(const Property &)),
        SLOT(execDialog(const Property &)));
    connect(m_panel_agent,
        SIGNAL(execMenu(const QList<Property> &)),
        SLOT(execMenu(const QList<Property> &)));
//X     connect(this,SIGNAL(triggerProperty(const QString &)),m_panel_agent,SIGNAL(TriggerProperty(const QString &)));

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

    connect(&m_icon_mapper, SIGNAL(mapped(const QString &)), m_panel_agent,
        SIGNAL(TriggerProperty(const QString &)));

    m_panel_agent->created();

    //setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
}

KIMPanelWidget::~KIMPanelWidget()
{
}

//X QSizeF KIMPanelWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
//X {
//X     return m_layout->sizeHint(which, constraint);
//X }

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
        icon = KIcon(renderText(prop.label).scaled(KIconLoader::SizeEnormous,KIconLoader::SizeEnormous));
    }

    prop_icon->setIcon(icon);
    //prop_button->setToolTip(prop.tip);
}

void KIMPanelWidget::registerProperties(const QList<Property> &props)
{
    while (m_layout->count()) {
        QGraphicsLayoutItem *item = m_layout->itemAt(0);
        m_layout->removeAt(0);
        item->graphicsItem()->hide();
    }
    
    m_icons.clear();
    m_prop_map.clear();
    Q_FOREACH (const Property &prop, props) {
        kDebug() << prop.key << prop.label;

        KIcon kicon;

        if (!prop.icon.isEmpty()) {
            kicon = KIcon(prop.icon);
        } else {
            kicon = KIcon(renderText(prop.label).scaled(KIconLoader::SizeEnormous,KIconLoader::SizeEnormous));
        }

        Plasma::IconWidget *icon = new Plasma::IconWidget(kicon,"",this);
        m_icons << icon;
        m_prop_map.insert(prop.key, icon);
        m_layout->addItem(icon);

        m_icon_mapper.setMapping(icon,prop.key);
        connect(icon,SIGNAL(clicked()),&m_icon_mapper,SLOT(map()));
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

#include "kimpanelwidget.moc"

// vim: sw=4 sts=4 et tw=100
