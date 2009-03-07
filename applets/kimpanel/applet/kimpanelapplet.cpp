/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#include "kimpanelapplet.h"

#include <KConfigDialog>
#include <KDesktopFile>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QDrag>
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

#include "math.h"

static const int s_magic_margin = 4;

KIMPanelApplet::KIMPanelApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent,args),
    m_layout(0),
    m_widget(0)
{
    setHasConfigurationInterface(true);
//X     setAcceptDrops(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    // set our default size here
//X     resize((m_visibleIcons / m_rowCount) * s_defaultIconSize +
//X             (s_defaultSpacing * (m_visibleIcons + 1)),
//X            m_rowCount * 22 + s_defaultSpacing * 3);
}

KIMPanelApplet::~KIMPanelApplet()
{
}

void KIMPanelApplet::saveState(KConfigGroup &config) const
{
}

void KIMPanelApplet::init()
{
    setBackgroundHints(Plasma::Applet::DefaultBackground);

    KConfigGroup cg = config();

    m_largestIconWidth = qMax((int)KIconLoader::SizeSmall,
        cg.readEntry("LargestIconWidth", (int)KIconLoader::SizeSmallMedium));

    // Initalize background
    m_background = new Plasma::FrameSvg();
    m_background->setImagePath("widgets/systemtray");

    // Initialize layout
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0,0,0,0);

    // Initailize Plasma::Dialog
//X     m_dialog = new Plasma::Dialog(0,Qt::X11BypassWindowManagerHint);
//X     m_dialog->setContextMenuPolicy(Qt::ActionsContextMenu);

    // Initialize widget which holds all im properties.
    m_widget = new KIMPanelWidget(this);
    
    m_widget->setContentsMargins(s_magic_margin,s_magic_margin,
        s_magic_margin,s_magic_margin);


    m_collapsedIcon = new Plasma::IconWidget(KIcon("arrow-up-double"),"",this);
    m_collapsedIcon->hide();
    
    // By default not collapsed
    m_layout->addItem(m_widget);

//X     m_layout->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed,QSizePolicy::DefaultType);
   connect(m_widget,SIGNAL(iconCountChanged(int)),SLOT(adjustSelf(int)));
    
}

/*
QSizeF KIMPanelApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{

    QSizeF sizeHint = size();
    if (!m_layout) {
        return sizeHint;
    }
    sizeHint.setWidth(left + right + m_layout->effectiveSizeHint(which).width());
    sizeHint.setHeight(top + bottom + m_layout->effectiveSizeHint(which).height());
    kDebug() << sizeHint;
    return sizeHint;
}
*/

void KIMPanelApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints & Plasma::FormFactorConstraint) ||
        (constraints & Plasma::SizeConstraint)) {
        //TODO: don't call so often
        adjustSelf(m_widget->iconCount());
        //resize(preferredSize());
    }
}

void KIMPanelApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    m_uiConfig.setupUi(widget);
    connect(parent, SIGNAL(accepted()), SLOT(configAccepted()));
    m_uiConfig.icons->setValue(m_largestIconWidth);
    parent->addPage(widget, i18n("General"), icon());
}

void KIMPanelApplet::configAccepted()
{
    bool changed = false;
    int temp = m_uiConfig.icons->value();

    KConfigGroup cg = config();

    if (temp != m_largestIconWidth) {
        m_largestIconWidth = temp;
        cg.writeEntry("LargestIconWidth", m_largestIconWidth);
        changed = true;
    }

    if (changed) {
        emit configNeedsSaving();
        adjustSelf(m_widget->iconCount());
    }
}

QList<QAction*> KIMPanelApplet::contextActions(Plasma::IconWidget *icon)
{
#if 0
    QList<QAction*> tempActions;
    if (!m_addAction) {
        m_addAction = new QAction(KIcon("list-add"), i18n("Add Icon..."), this);
        connect(m_addAction, SIGNAL(triggered(bool)), this, SLOT(showAddInterface()));
    }

    tempActions << m_addAction;

    if (icon) {
        m_rightClickedIcon = icon;
        if (!m_removeAction) {
            m_removeAction = new QAction(KIcon("list-remove"), i18n("Remove Icon"), this);
            connect(m_removeAction, SIGNAL(triggered(bool)), this, SLOT(removeCurrentIcon()));
        }
        tempActions << m_removeAction;
    }
    return tempActions;
#endif
    return QList<QAction *>();
}

void KIMPanelApplet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

//    QRect r = rect().toRect();
//    m_background->setElementPrefix("lastelements");

    kDebug() << contentsRect;
    painter->save();

    //m_background->setElementPrefix(QString());
    m_background->resizeFrame(contentsRect.size());
    m_background->paintFrame(painter, contentsRect, contentsRect);

    painter->restore();
}

void KIMPanelApplet::adjustSelf(int iconCount)
{
    int iconWidth; 
    int i,j;
    QSizeF sizeHint = geometry().size();
    switch (formFactor()) {
    case Plasma::Horizontal:
        i = 1;
        while (m_widget->contentsRect().height()/i > m_largestIconWidth)
            i++;
        j = (iconCount + (i - 1)) / i;
        sizeHint = QSizeF(j*m_widget->contentsRect().height()/i, m_widget->contentsRect().height());
        break;
    case Plasma::Vertical:
        i = 1;
        while (m_widget->contentsRect().width()/i > m_largestIconWidth)
            i++;
        j = (iconCount + (i - 1)) / i;
        sizeHint = QSizeF(m_widget->contentsRect().width(),j*m_widget->contentsRect().width()/i);
        break;
    case Plasma::Planar:
    case Plasma::MediaCenter:
        sizeHint = QSizeF(iconCount*(qreal)KIconLoader::SizeMedium,(qreal)KIconLoader::SizeMedium);
        break;
    }
    
    qreal left, top, right, bottom;
    getContentsMargins(&left,&top,&right,&bottom);
    sizeHint = QSizeF(sizeHint.width() + left + right, sizeHint.height() + top + bottom);
    m_widget->getContentsMargins(&left,&top,&right,&bottom);
    sizeHint = QSizeF(sizeHint.width() + left + right, sizeHint.height() + top + bottom);
    kDebug() << sizeHint;
    setPreferredSize(sizeHint);
}

#if 0
void KIMPanelApplet::collapsed(bool b)
{
    kDebug() << b;
    if (b) {
        m_layout->removeAt(0);
        delete  m_widget;
        
        m_widget = new KIMPanelWidget(this);
        
        connect(m_widget,SIGNAL(collapsed(bool)),SLOT(collapsed(bool)));

        m_widget->setContentsMargins(s_magic_margin,s_magic_margin,
            s_magic_margin,s_magic_margin);

        m_dialog->setGraphicsWidget(m_widget);
        m_dialog->show();

        //m_layout->addItem(m_collapsedIcon);
        //m_collapsedIcon->show();
    } else {
/*
        m_collapsedIcon->hide();
        m_layout->removeAt(0);
        m_layout->addItem(m_widget);
        m_collapsedIcon->show();
        m_widget->show();
*/
    }
}
#endif 

K_EXPORT_PLASMA_APPLET(kimpanel, KIMPanelApplet)

#include "kimpanelapplet.moc"
