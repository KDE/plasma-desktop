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

#include "layout_widget.h"

#include <kpluginfactory.h>

#include <QPushButton>

#include "xkb_rules.h"
#include "x11_helper.h"
#include "xkb_helper.h"
#include "keyboard_config.h"
#include "flags.h"


K_PLUGIN_FACTORY(LayoutWidgetFactory, registerPlugin<LayoutWidget>();)

LayoutWidget::LayoutWidget(QWidget* parent, const QList<QVariant>& /*args*/):
	QWidget(parent),
	xEventNotifier(nullptr),
	keyboardConfig(new KeyboardConfig()),
	flags(new Flags())
{
	if( ! X11Helper::xkbSupported(NULL) ) {
//		setFailedToLaunch(true, "XKB extension failed to initialize");
		hide();
		return;
	}

	keyboardConfig->load();
	bool show = // keyboardConfig->showIndicator &&
			( keyboardConfig->showSingle || X11Helper::getLayoutsList().size() > 1 );
	if( ! show ) {
		hide();
		return;
	}

	widget = new QPushButton(this);
	widget->setFlat(true);

	layoutChanged();
	init();
}

LayoutWidget::~LayoutWidget()
{
	destroy();
}

void LayoutWidget::init()
{
	connect(widget, &QPushButton::clicked, this, &LayoutWidget::toggleLayout);
	connect(xEventNotifier, &XEventNotifier::layoutChanged, this, &LayoutWidget::layoutChanged);
	connect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &LayoutWidget::layoutChanged);
	xEventNotifier->start();
}

void LayoutWidget::destroy()
{
	xEventNotifier->stop();
	disconnect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &LayoutWidget::layoutChanged);
	disconnect(xEventNotifier, &XEventNotifier::layoutChanged, this, &LayoutWidget::layoutChanged);
}

void LayoutWidget::toggleLayout()
{
	X11Helper::switchToNextLayout();
}

void LayoutWidget::layoutChanged()
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

	QIcon icon;
	if( keyboardConfig->isFlagShown() ) {
		icon = flags->getIcon(layoutUnit.layout);
	}

	QString longText = Flags::getLongText(layoutUnit, NULL);
	if( ! icon.isNull() ) {
		widget->setIcon(icon);
		widget->setText("");
		widget->setToolTip(longText);
	}
	else {
		QString shortText = Flags::getShortText(layoutUnit, *keyboardConfig);
		widget->setIcon(icon);
		widget->setText(shortText);
		widget->setToolTip(longText);
//		widget->setShortcut(QKeySequence());
	}
}

#include "layout_widget.moc"
