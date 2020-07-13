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

#include "flags.h"

#include <KLocalizedString>

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <KFontUtils>

#include <QStandardPaths>
#include <QStringList>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QFontDatabase>

#include <math.h>

#include "x11_helper.h"

//for text handling
#include "keyboard_config.h"
#include "xkb_rules.h"


static const int FLAG_MAX_SIZE = 22;
static const char flagTemplate[] = "kf5/locale/countries/%1/flag.png";

int iconSize(int s)
{
    if (s < 16) {
        return 16;
    } else if (s < 22) {
        return 22;
    } else if (s < 32) {
        return 32;
    } else if (s < 48) {
        return 48;
    } else if (s < 64) {
        return 64;
    } else {
        return 128;
    }
}

Flags::Flags():
	svg(nullptr)
{
	transparentPixmap = new QPixmap(FLAG_MAX_SIZE, FLAG_MAX_SIZE);
	transparentPixmap->fill(Qt::transparent);
}

Flags::~Flags()
{
	if( svg != nullptr ) {
		disconnect(svg, &Plasma::Svg::repaintNeeded, this, &Flags::themeChanged);
		delete svg;
	}
	delete transparentPixmap;
}

const QIcon Flags::getIcon(const QString& layout)
{
	if( ! iconMap.contains(layout) ) {
		iconMap[ layout ] = createIcon(layout);
	}
	return iconMap[ layout ];
}

QIcon Flags::createIcon(const QString& layout)
{
	QIcon icon;
	if( ! layout.isEmpty() ) {
        QString file;
		if( layout == QLatin1String("epo") ) {
			file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kcmkeyboard/pics/epo.png"));
		}
		else {
			QString countryCode = getCountryFromLayoutName( layout );
			if( ! countryCode.isEmpty() ) {
				file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString(flagTemplate).arg(countryCode));
				//			qCDebug(KCM_KEYBOARD, ) << "Creating icon for" << layout << "with" << file;
			}
		}

		if (!file.isEmpty()) {
            QImage flagImg;
            flagImg.load(file);
            const int size = iconSize(qMax(flagImg.width(), flagImg.height()));
            QPixmap iconPix(size, size);
            iconPix.fill(Qt::transparent);
            QRect dest(flagImg.rect());
            dest.moveCenter(iconPix.rect().center());

            QPainter painter(&iconPix);
            painter.drawImage(dest, flagImg);
            painter.end();

            icon.addPixmap(iconPix);
        }
	}
	return icon;
}


//static
//const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString& layout)  const
{
	QString countryCode = layout;

	if( countryCode == QLatin1String("nec_vndr/jp") )
		return QStringLiteral("jp");

//	if( NON_COUNTRY_LAYOUTS.contain(layout) )
	if( countryCode.length() > 2 )
		return QLatin1String("");

	return countryCode;
}

//TODO: move this to some other class?

QString Flags::getShortText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig)
{
	if( layoutUnit.isEmpty() )
		return QStringLiteral("--");

    QString layoutText = layoutUnit.layout();

	foreach(const LayoutUnit& lu, keyboardConfig.layouts) {
        if( layoutUnit.layout() == lu.layout() && layoutUnit.variant() == lu.variant() ) {
			layoutText = lu.getDisplayName();
			break;
		}
	}

//TODO: good autolabel
//	if( layoutText == layoutUnit.layout && layoutUnit.getDisplayName() != layoutUnit.layout ) {
//		layoutText = layoutUnit.getDisplayName();
//	}

	return layoutText;
}

QString Flags::getFullText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig, const Rules* rules)
{
	QString shortText = Flags::getShortText(layoutUnit, keyboardConfig);
	QString longText = Flags::getLongText(layoutUnit, rules);
	return i18nc("short layout label - full layout name", "%1 - %2", shortText, longText);
}

static QString getDisplayText(const QString& layout, const QString& variant, const Rules* rules)
{
	if( variant.isEmpty() )
		return layout;
	if( rules == nullptr || rules->version == QLatin1String("1.0") )
		return i18nc("layout - variant", "%1 - %2", layout, variant);
	return variant;
}

QString Flags::getLongText(const LayoutUnit& layoutUnit, const Rules* rules)
{
	if( rules == nullptr ) {
        return getDisplayText(layoutUnit.layout(), layoutUnit.variant(), rules);
	}

    QString layoutText = layoutUnit.layout();
    const LayoutInfo* layoutInfo = rules->getLayoutInfo(layoutUnit.layout());
	if( layoutInfo != nullptr ) {
		layoutText = layoutInfo->description;

        if( ! layoutUnit.variant().isEmpty() ) {
            const VariantInfo* variantInfo = layoutInfo->getVariantInfo(layoutUnit.variant());
            QString variantText = variantInfo != nullptr ? variantInfo->description : layoutUnit.variant();

			layoutText = getDisplayText(layoutText, variantText, rules);
		}
	}

	return layoutText;
}

static
QString getPixmapKey(const KeyboardConfig& keyboardConfig)
{
	switch(keyboardConfig.indicatorType) {
	case KeyboardConfig::SHOW_FLAG:
		return QStringLiteral("_fl");
	case KeyboardConfig::SHOW_LABEL_ON_FLAG:
		return QStringLiteral("_bt");
	case KeyboardConfig::SHOW_LABEL:
		return QStringLiteral("_lb");
	}
	return QStringLiteral("_");	// should not happen
}

void Flags::drawLabel(QPainter& painter, const QString& layoutText, bool flagShown)
{
	QFont font = painter.font();
    QRect rect = painter.window();

	font.setPointSize(KFontUtils::adaptFontSize(painter, layoutText, rect.size(), rect.height()));

	// we init svg so that we get notification about theme change
	getSvg();

    const QColor textColor = flagShown ? Qt::black : Plasma::Theme().color(Plasma::Theme::TextColor);

    painter.setPen(textColor);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, layoutText);
}

const QIcon Flags::getIconWithText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig)
{
	const QString keySuffix(getPixmapKey(keyboardConfig));
	const QString key(layoutUnit.toString() + keySuffix);
	if( iconOrTextMap.contains(key) ) {
		return iconOrTextMap[ key ];
	}

	if( keyboardConfig.indicatorType == KeyboardConfig::SHOW_FLAG ) {
        QIcon icon = getIcon(layoutUnit.layout());
		if( ! icon.isNull() ) {
			iconOrTextMap[ key ] = icon;
			return icon;
		}
	}

	QString layoutText = Flags::getShortText(layoutUnit, keyboardConfig);

	const QSize TRAY_ICON_SIZE(128, 128);
	QPixmap pixmap = QPixmap(TRAY_ICON_SIZE);
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
//	p.setRenderHint(QPainter::Antialiasing);

	if( keyboardConfig.indicatorType == KeyboardConfig::SHOW_LABEL_ON_FLAG ) {
        QIcon iconf = createIcon(layoutUnit.layout());
        painter.drawPixmap(pixmap.rect(), iconf.pixmap(TRAY_ICON_SIZE));
	}

	drawLabel(painter, layoutText, keyboardConfig.isFlagShown());

    painter.end();

    QIcon icon(pixmap);
	iconOrTextMap[ key ] = icon;

	return icon;
}

Plasma::Svg* Flags::getSvg()
{
	if( svg == nullptr ) {
		svg = new Plasma::Svg;
	    svg->setImagePath(QStringLiteral("widgets/labeltexture"));
	    svg->setContainsMultipleImages(true);
	    connect(svg, &Plasma::Svg::repaintNeeded, this, &Flags::themeChanged);
	}
	return svg;
}

void Flags::themeChanged()
{
//	qCDebug(KCM_KEYBOARD, ) << "Theme changed, new text color" << Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
	clearCache();
	emit pixmapChanged();
}

void Flags::clearCache()
{
//	qCDebug(KCM_KEYBOARD, ) << "Clearing flag pixmap cache";
	iconOrTextMap.clear();
}
