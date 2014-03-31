#ifndef __FONT_FILTER_H__
#define __FONT_FILTER_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KLineEdit>
#include <QPixmap>
#include <QFontDatabase>

class QLabel;
class QMenu;
class QActionGroup;
class KAction;

namespace KFI
{

class CFontFilter : public KLineEdit
{
    Q_OBJECT

    public:

    enum ECriteria
    {
        CRIT_FAMILY,
        CRIT_STYLE,
        CRIT_FOUNDRY,
        CRIT_FONTCONFIG,
        CRIT_FILETYPE,
        CRIT_FILENAME,
        CRIT_LOCATION,
        CRIT_WS,

        NUM_CRIT
    };

    CFontFilter(QWidget *parent);
    virtual ~CFontFilter() { }

    void setFoundries(const QSet<QString> &currentFoundries);

    QSize sizeHint() const;

    Q_SIGNALS:

    void criteriaChanged(int crit, qulonglong ws, const QStringList &ft);

    private Q_SLOTS:

    void filterChanged();
    void ftChanged(const QString &ft);
    void wsChanged(const QString &writingSystemName);
    void foundryChanged(const QString &foundry);

    private:

    void addAction(ECriteria crit, const QString &text, bool on);
    void resizeEvent(QResizeEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void setCriteria(ECriteria crit);

    private:

    QLabel                       *itsMenuButton;
    QMenu                        *itsMenu;
    ECriteria                    itsCurrentCriteria;
    QFontDatabase::WritingSystem itsCurrentWs;
    QStringList                  itsCurrentFileTypes;
    QPixmap                      itsPixmaps[NUM_CRIT];
    QAction                      *itsActions[NUM_CRIT];
    QActionGroup                 *itsActionGroup;
};

}

#endif
