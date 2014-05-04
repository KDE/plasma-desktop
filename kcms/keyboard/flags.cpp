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

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <klocalizedstring.h>

#include <plasma/svg.h>
#include <plasma/theme.h>

#include <QStringList>
#include <QPixmap>
#include <QPainter>
#include <QIcon>

#include <math.h>

#include "x11_helper.h"

//for text handling
#include "keyboard_config.h"
#include "xkb_rules.h"


static const int FLAG_MAX_WIDTH = 21;
static const int FLAG_MAX_HEIGHT = 14;
static const char flagTemplate[] = "l10n/%1/flag.png";

Flags::Flags():
	svg(NULL)
{
	transparentPixmap = new QPixmap(FLAG_MAX_WIDTH, FLAG_MAX_HEIGHT);
	transparentPixmap->fill(Qt::transparent);
}

Flags::~Flags()
{
	if( svg != NULL ) {
		disconnect(svg, SIGNAL(repaintNeeded()), this, SLOT(themeChanged()));
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
		if( layout == "epo" ) {
			QString file = KStandardDirs::locate("data", "kcmkeyboard/pics/epo.png");
			icon.addFile(file);
		}
		else {
			QString countryCode = getCountryFromLayoutName( layout );
			if( ! countryCode.isEmpty() ) {
				QString file = KStandardDirs::locate("locale", QString(flagTemplate).arg(countryCode));
				//			kDebug() << "Creating icon for" << layout << "with" << file;
				icon.addFile(file);
			}
		}
	}
	return icon;
}


//static
//const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString& layout)  const
{
	QString countryCode = layout;

	if( countryCode == "nec_vndr/jp" )
		return "jp";

//	if( NON_COUNTRY_LAYOUTS.contain(layout) )
	if( countryCode.length() > 2 )
		return "";

	return countryCode;
}

//TODO: move this to some other class?

QString Flags::getShortText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig)
{
	if( layoutUnit.isEmpty() )
		return QString("--");

	QString layoutText = layoutUnit.layout;

	foreach(const LayoutUnit& lu, keyboardConfig.layouts) {
		if( layoutUnit.layout == lu.layout && layoutUnit.variant == lu.variant ) {
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
	if( rules == NULL || rules->version == "1.0" )
		return i18nc("layout - variant", "%1 - %2", layout, variant);
	return variant;
}

QString Flags::getLongText(const LayoutUnit& layoutUnit, const Rules* rules)
{
	if( rules == NULL ) {
		return getDisplayText(layoutUnit.layout, layoutUnit.variant, rules);
	}

	QString layoutText = layoutUnit.layout;
	const LayoutInfo* layoutInfo = rules->getLayoutInfo(layoutUnit.layout);
	if( layoutInfo != NULL ) {
		layoutText = layoutInfo->description;

		if( ! layoutUnit.variant.isEmpty() ) {
			const VariantInfo* variantInfo = layoutInfo->getVariantInfo(layoutUnit.variant);
			QString variantText = variantInfo != NULL ? variantInfo->description : layoutUnit.variant;

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
		return "_fl";
	case KeyboardConfig::SHOW_LABEL_ON_FLAG:
		return "_bt";
	case KeyboardConfig::SHOW_LABEL:
		return "_lb";
	}
	return "_";	// should not happen
}

QPixmap shadowText (QFont font, QColor textColor, QString text) {
    //don't try to paint stuff on a future null pixmap because the text is empty
    if (text.isEmpty()) {
        return QPixmap();
    }

    // Draw text
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    QPixmap textPixmap(textRect.width(), fm.height());
    textPixmap.fill(Qt::transparent);
    QPainter p(&textPixmap);
    p.setPen(textColor);
    p.setFont(font);
    // FIXME: the center alignment here is odd: the rect should be the size needed by
    //        the text, but for some fonts and configurations this is off by a pixel or so
    //        and "centering" the text painting 'fixes' that. Need to research why
    //        this is the case and determine if we should be painting it differently here,
    //        doing soething different with the boundingRect call or if it's a problem
    //        in Qt itself
    p.drawText(textPixmap.rect(), Qt::AlignCenter, text);
    p.end();

    return textPixmap;
}


void Flags::drawLabel(QPainter& painter, const QString& layoutText, bool flagShown)
{
	QFont font = painter.font();

    QRect rect = painter.window();
//	int fontSize = layoutText.length() == 2
//			? height * 7 / 10
//			: height * 5 / 10;

	int fontSize = rect.height();// * 7 /10;

	font.setPixelSize(fontSize);
	font.setWeight(QFont::DemiBold);

	QFontMetrics fm = painter.fontMetrics();
	int width = fm.width(layoutText);

	if( width > rect.width() * 2 / 3 ) {
		fontSize = round( (double)fontSize * ((double)rect.width()*2/3) / width );
	}
	
	int smallestReadableSize = KGlobalSettings::smallestReadableFont().pixelSize();
	if( fontSize < smallestReadableSize ) {
		fontSize = smallestReadableSize;
	}
	font.setPixelSize(fontSize);

#ifdef DONT_USE_PLASMA
	painter.setFont(font);
	painter.setPen(Qt::white);
	painter.drawText(QRect(rect).adust(1,1,0,0), Qt::AlignCenter | Qt::AlignHCenter, layoutText);
	painter.setPen(Qt::black);
	painter.drawText(rect, Qt::AlignCenter | Qt::AlignHCenter, layoutText);
#else
	// we init svg so that we get notification about theme change
	getSvg();

    Plasma::Theme theme;
    QColor textColor = flagShown ? Qt::black : theme.color(Plasma::Theme::TextColor);
    //QColor shadowColor = flagShown ? Qt::white : theme.color(Plasma::Theme::BackgroundColor);
    //    QPixmap pixmap = Plasma::PaintUtils::texturedText(layoutText, font, svg);
    QPixmap labelPixmap = shadowText(font, textColor, layoutText);

    int y = round((rect.height() - labelPixmap.height()) / 2.0);
    int x = round((rect.width() - labelPixmap.width()) / 2.0);
    painter.drawPixmap(QPoint(x, y), labelPixmap);
#endif
}


const QIcon Flags::getIconWithText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig)
{
	QString keySuffix(getPixmapKey(keyboardConfig));
	QString key(layoutUnit.toString() + keySuffix);
	if( iconOrTextMap.contains(key) ) {
		return iconOrTextMap[ key ];
	}

	if( keyboardConfig.indicatorType == KeyboardConfig::SHOW_FLAG ) {
		QIcon icon = getIcon(layoutUnit.layout);
		if( ! icon.isNull() ) {
			iconOrTextMap[ key ] = icon;
			return icon;
		}
	}

	QString layoutText = Flags::getShortText(layoutUnit, keyboardConfig);

	const QSize TRAY_ICON_SIZE(21, 14);
	QPixmap pixmap = QPixmap(TRAY_ICON_SIZE);
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);
//	p.setRenderHint(QPainter::SmoothPixmapTransform);
//	p.setRenderHint(QPainter::Antialiasing);

	if( keyboardConfig.indicatorType == KeyboardConfig::SHOW_LABEL_ON_FLAG ) {
    	QIcon iconf = createIcon(layoutUnit.layout);
    	iconf.paint(&painter, painter.window(), Qt::AlignCenter);
	}

	drawLabel(painter, layoutText, keyboardConfig.isFlagShown());

    painter.end();

    QIcon icon(pixmap);
	iconOrTextMap[ key ] = icon;

	return icon;
}

Plasma::Svg* Flags::getSvg()
{
	if( svg == NULL ) {
		svg = new Plasma::Svg;
	    svg->setImagePath("widgets/labeltexture");
	    svg->setContainsMultipleImages(true);
	    connect(svg, SIGNAL(repaintNeeded()), this, SLOT(themeChanged()));
	}
	return svg;
}

void Flags::themeChanged()
{
//	kDebug() << "Theme changed, new text color" << Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
	clearCache();
	emit pixmapChanged();
}

void Flags::clearCache()
{
//	kDebug() << "Clearing flag pixmap cache";
	iconOrTextMap.clear();
}
