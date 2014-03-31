/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "keyboard_applet.h"

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <plasma/theme.h>
#include <plasma/tooltipmanager.h>
#include <plasma/paintutils.h>

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QAction>
#include <QtDBus/QtDBus>

#include "x11_helper.h"
#include "xkb_rules.h"
#include "keyboard_config.h"
#include "keyboard_dbus.h"
#include "layouts_menu.h"
//#include "utils.h"


K_EXPORT_PLASMA_APPLET(keyboard, KeyboardApplet)

KeyboardApplet::KeyboardApplet(QObject *parent, const QVariantList &args):
	Plasma::Applet(parent, args),
	xEventNotifier(),
	rules(Rules::readRules(Rules::READ_EXTRAS)),
	keyboardConfig(new KeyboardConfig()),
	layoutsMenu(new LayoutsMenu(*keyboardConfig, *rules, flags))
{
	if( ! X11Helper::xkbSupported(NULL) ) {
		setFailedToLaunch(true, i18n("XKB extension failed to initialize"));
		return;
	}

    m_svg = new Plasma::Svg(this);
	m_svg->setImagePath("widgets/labeltexture");
	m_svg->setContainsMultipleImages(true);
	resize(48,48);

	setHasConfigurationInterface(false);

	setAspectRatioMode(Plasma::KeepAspectRatio);
	//setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
	setBackgroundHints(DefaultBackground);
	connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configChanged()));
}

KeyboardApplet::~KeyboardApplet()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.disconnect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configChanged()));

    delete layoutsMenu;
	delete rules;
}

void KeyboardApplet::keyboardConfigChanged()
{
	readConfig();
	update();
}

void KeyboardApplet::readConfig()
{
//	KConfigGroup config = Plasma::Applet::config("KeyboardLayout");
	keyboardConfig->load();
//	drawFlag = keyboardConfig->readEntry("ShowFlag", true);
}

void KeyboardApplet::configChanged()
{
	Applet::configChanged();
	readConfig();
}

void KeyboardApplet::init()
{
	Applet::init();

	readConfig();
	connect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	connect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutChanged()));
	xEventNotifier.start();

	layoutChanged();
}

void KeyboardApplet::destroy()
{
	xEventNotifier.stop();
	disconnect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutChanged()));
	disconnect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	Applet::destroy();
}

void KeyboardApplet::layoutChanged()
{
	generatePixmap();
	updateTooltip();
	update();
}

void KeyboardApplet::updateTooltip()
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

	const QIcon icon(getFlag(layoutUnit.layout));
	Plasma::ToolTipContent data(name(), flags.getLongText(layoutUnit, rules), icon);
	Plasma::ToolTipManager::self()->setContent(this, data);
}

const QIcon KeyboardApplet::getFlag(const QString& layout)
{
	return keyboardConfig->isFlagShown() ? flags.getIcon(layout) : QIcon();
}

void KeyboardApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem */*option*/, const QRect &contentsRect)
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

	const QIcon icon(getFlag(layoutUnit.layout));
	if( ! icon.isNull() ) {
		p->save();
		p->setRenderHint(QPainter::SmoothPixmapTransform);
		p->setRenderHint(QPainter::Antialiasing);
		QPixmap pixmap = icon.pixmap(contentsRect.size());
		p->drawPixmap(contentsRect, pixmap);
		p->restore();
	}
	if( icon.isNull() || keyboardConfig->isLabelShown() ) {
		QRect finalRect(m_pixmap.rect());
		finalRect.moveCenter(contentsRect.center());
		p->drawPixmap(finalRect, m_pixmap);
	}
}

void KeyboardApplet::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
	if( event->button() == Qt::LeftButton ) {
		X11Helper::switchToNextLayout();
	}
	event->ignore();
}

void KeyboardApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
	int iconSize;
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            iconSize = IconSize(KIconLoader::Desktop);
        } else {
            iconSize = IconSize(KIconLoader::Small);
        }
	setMinimumSize(iconSize, iconSize);
    }
    if (constraints & Plasma::SizeConstraint) {
		generatePixmap();
	}
}

void KeyboardApplet::generatePixmap()
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	QRect contentsRect = KeyboardApplet::contentsRect().toRect();
	QString shortText = Flags::getShortText(layoutUnit, *keyboardConfig);

	QPixmap pixmap(contentsRect.size());
	pixmap.fill(Qt::transparent);

	QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DesktopFont);
	int height = qMin(contentsRect.height(), contentsRect.width());
	int fontSize = shortText.length() == 2
			? height * 13 / 15
			: height * 5 / 15;

	int smallestReadableSize = KGlobalSettings::smallestReadableFont().pixelSize();
	if( fontSize < smallestReadableSize ) {
		fontSize = smallestReadableSize;
	}
	font.setPixelSize(fontSize);
	if( keyboardConfig->isFlagShown() ) {
		m_pixmap = Plasma::PaintUtils::shadowText(shortText, font, Qt::black, Qt::white, QPoint(), 3);
	}
	else {
		m_pixmap = Plasma::PaintUtils::texturedText(shortText, font, m_svg);
	}
}

QList<QAction*> KeyboardApplet::contextualActions()
{
	return layoutsMenu->contextualActions();
}
