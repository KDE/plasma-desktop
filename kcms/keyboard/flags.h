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

#ifndef FLAGS_H_
#define FLAGS_H_

#include <QMap>
#include <QObject>
#include <QString>

class QPixmap;
class QIcon;
class LayoutUnit;
class KeyboardConfig;
struct Rules;
class QPainter;
namespace Plasma
{
class Svg;
}

class Flags : public QObject
{
    Q_OBJECT

public:
    Flags();
    ~Flags() override;

    enum ColorType { Plasma = Qt::UserRole, ColorScheme };

    const QIcon getIcon(const QString &layout);
    const QIcon getIconWithText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig, ColorType colorType = ColorType::Plasma);
    const QPixmap &getTransparentPixmap() const
    {
        return *transparentPixmap;
    }

    static QString getLongText(const LayoutUnit &layoutUnit, const Rules *rules);
    static QString getShortText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig);
    static QString getFullText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig, const Rules *rules);

public Q_SLOTS:
    void themeChanged();
    void clearCache();

Q_SIGNALS:
    void pixmapChanged();

private:
    QIcon createIcon(const QString &layout);
    QString getCountryFromLayoutName(const QString &fullLayoutName) const;
    void drawLabel(QPainter &painter, const QString &layoutText, bool flagShown, ColorType colorType);
    Plasma::Svg *getSvg();

    QMap<QString, QIcon> iconMap;
    QMap<QString, QIcon> iconOrTextMap;
    QPixmap *transparentPixmap;
    Plasma::Svg *svg;
};

#endif /* FLAGS_H_ */
