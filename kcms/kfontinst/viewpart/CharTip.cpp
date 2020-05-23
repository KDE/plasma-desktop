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

/*
 * Inspired by konq_filetip.cc
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2000, 2001, 2002 David Faure <faure@kde.org>
 * Copyright (C) 2004 Martin Koller <m.koller@surfeu.at>
*/

#include "CharTip.h"
#include "FontPreview.h"
#include "UnicodeCategories.h"
#include <QApplication>
#include <QLabel>
#include <QToolTip>
#include <QTimer>
#include <QPixmap>
#include <QBoxLayout>
#include <QEvent>
#include <QResizeEvent>
#include <QScreen>

namespace KFI
{

EUnicodeCategory getCategory(quint32 ucs2)
{
    for(int i=0; UNICODE_INVALID!=constUnicodeCategoryList[i].category; ++i)
        if(constUnicodeCategoryList[i].start<=ucs2 &&
           constUnicodeCategoryList[i].end>=ucs2)
            return constUnicodeCategoryList[i].category;

    return UNICODE_UNASSIGNED;
}

static QString toStr(EUnicodeCategory cat)
{
    switch (cat)
    {
        case UNICODE_CONTROL:
            return i18n("Other, Control");
        case UNICODE_FORMAT:
            return i18n("Other, Format");
        case UNICODE_UNASSIGNED:
            return i18n("Other, Not Assigned");
        case UNICODE_PRIVATE_USE:
            return i18n("Other, Private Use");
        case UNICODE_SURROGATE:
            return i18n("Other, Surrogate");
        case UNICODE_LOWERCASE_LETTER:
            return i18n("Letter, Lowercase");
        case UNICODE_MODIFIER_LETTER:
            return i18n("Letter, Modifier");
        case UNICODE_OTHER_LETTER:
            return i18n("Letter, Other");
        case UNICODE_TITLECASE_LETTER:
            return i18n("Letter, Titlecase");
        case UNICODE_UPPERCASE_LETTER:
            return i18n("Letter, Uppercase");
        case UNICODE_COMBINING_MARK:
            return i18n("Mark, Spacing Combining");
        case UNICODE_ENCLOSING_MARK:
            return i18n("Mark, Enclosing");
        case UNICODE_NON_SPACING_MARK:
            return i18n("Mark, Non-Spacing");
        case UNICODE_DECIMAL_NUMBER:
            return i18n("Number, Decimal Digit");
        case UNICODE_LETTER_NUMBER:
            return i18n("Number, Letter");
        case UNICODE_OTHER_NUMBER:
            return i18n("Number, Other");
        case UNICODE_CONNECT_PUNCTUATION:
            return i18n("Punctuation, Connector");
        case UNICODE_DASH_PUNCTUATION:
            return i18n("Punctuation, Dash");
        case UNICODE_CLOSE_PUNCTUATION:
            return i18n("Punctuation, Close");
        case UNICODE_FINAL_PUNCTUATION:
            return i18n("Punctuation, Final Quote");
        case UNICODE_INITIAL_PUNCTUATION:
            return i18n("Punctuation, Initial Quote");
        case UNICODE_OTHER_PUNCTUATION:
            return i18n("Punctuation, Other");
        case UNICODE_OPEN_PUNCTUATION:
            return i18n("Punctuation, Open");
        case UNICODE_CURRENCY_SYMBOL:
            return i18n("Symbol, Currency");
        case UNICODE_MODIFIER_SYMBOL:
            return i18n("Symbol, Modifier");
        case UNICODE_MATH_SYMBOL:
            return i18n("Symbol, Math");
        case UNICODE_OTHER_SYMBOL:
            return i18n("Symbol, Other");
        case UNICODE_LINE_SEPARATOR:
            return i18n("Separator, Line");
        case UNICODE_PARAGRAPH_SEPARATOR:
            return i18n("Separator, Paragraph");
        case UNICODE_SPACE_SEPARATOR:
            return i18n("Separator, Space");
        default:
            return "";
    }
}

CCharTip::CCharTip(CFontPreview *parent)
        : QFrame(nullptr, Qt::ToolTip | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint),
          itsParent(parent)
{
    itsPixmapLabel = new QLabel(this);
    itsLabel = new QLabel(this);
    itsTimer = new QTimer(this);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(0);
    layout->addWidget(itsPixmapLabel);
    layout->addWidget(itsLabel);

    setPalette(QToolTip::palette());
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);
    hide();
}

CCharTip::~CCharTip()
{
}

void CCharTip::setItem(const CFcEngine::TChar &ch)
{
    hideTip();

    itsItem=ch;
    itsTimer->disconnect(this);
    connect(itsTimer, &QTimer::timeout, this, &CCharTip::showTip);
    itsTimer->setSingleShot(true);
    itsTimer->start(300);
}

void CCharTip::showTip()
{
    if(!itsParent->underMouse())
        return;

    static const int constPixSize=96;

    EUnicodeCategory cat(getCategory(itsItem.ucs4));
    QString          details("<table>");

    details+="<tr><td align=\"right\"><b>"+i18n("Category")+"&nbsp;</b></td><td>"+
             toStr(cat)+"</td></tr>";
    details+="<tr><td align=\"right\"><b>"+i18n("UCS-4")+"&nbsp;</b></td><td>"+
             QString().sprintf("U+%4.4X", itsItem.ucs4)+"&nbsp;</td></tr>";

    QString str(QString::fromUcs4(&(itsItem.ucs4), 1));
    details+="<tr><td align=\"right\"><b>"+i18n("UTF-16")+"&nbsp;</b></td><td>";

    const ushort *utf16(str.utf16());

    for(int i=0; utf16[i]; ++i)
    {
        if(i)
            details+=' ';
        details+=QString().sprintf("0x%4.4X",  utf16[i]);
    }
    details+="</td></tr>";
    details+="<tr><td align=\"right\"><b>"+i18n("UTF-8")+"&nbsp;</b></td><td>";

    QByteArray utf8(str.toUtf8());

    for(int i=0; i<utf8.size(); ++i)
    {
        if(i)
            details+=' ';
        details+=QString().sprintf("0x%2.2X", (unsigned char)(utf8.constData()[i]));
    }
    details+="</td></tr>";

    // Note: the "<b></b> below is just to stop Qt converting the xml entry into
    // a character!
    if ((0x0001 <= itsItem.ucs4 && itsItem.ucs4 <= 0xD7FF) ||
        (0xE000 <= itsItem.ucs4 && itsItem.ucs4 <= 0xFFFD) ||
        (0x10000 <= itsItem.ucs4 && itsItem.ucs4 <= 0x10FFFF))
        details+="<tr><td align=\"right\"><b>"+i18n("XML Decimal Entity")+"&nbsp;</b></td><td>"+
                 QString().sprintf("&#<b></b>%d;", itsItem.ucs4)+"</td></tr>";

    details+="</table>";
    itsLabel->setText(details);

    QList<CFcEngine::TRange> range;
    range.append(CFcEngine::TRange(itsItem.ucs4, 0));

    QColor bgnd(Qt::white);
    bgnd.setAlpha(0);

    QImage img=itsParent->engine()->draw(itsParent->itsFontName, itsParent->itsStyleInfo,
                                         itsParent->itsCurrentFace-1, palette().text().color(), bgnd,
                                         constPixSize, constPixSize, false, range, nullptr);

    if(!img.isNull())
        itsPixmapLabel->setPixmap(QPixmap::fromImage(img));
    else
        itsPixmapLabel->setPixmap(QPixmap());

    itsTimer->disconnect(this);
    connect(itsTimer, &QTimer::timeout, this, &CCharTip::hideTip);
    itsTimer->setSingleShot(true);
    itsTimer->start(15000);

    qApp->installEventFilter(this);
    reposition();
    show();
}

void CCharTip::hideTip()
{
    itsTimer->stop();
    qApp->removeEventFilter(this);
    hide();
}

void CCharTip::reposition()
{
    QRect rect(itsItem);

    rect.moveTopRight(itsParent->mapToGlobal(rect.topRight()));

    QPoint pos(rect.center());
    QRect  desk(QApplication::screenAt(rect.center())->geometry());

    if ((rect.center().x() + width()) > desk.right())
    {
        if (pos.x() - width() < 0)
            pos.setX(0);
        else
            pos.setX( pos.x() - width() );
    }
    // should the tooltip be shown above or below the ivi ?
    if (rect.bottom() + height() > desk.bottom())
        pos.setY( rect.top() - height() );
    else
        pos.setY(rect.bottom() + 1);

    move(pos);
    update();
}

void CCharTip::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    reposition();
}

bool CCharTip::eventFilter(QObject *, QEvent *e)
{
    switch (e->type())
    {
        case QEvent::Leave:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
            hideTip();
        default: break;
    }

    return false;
}

}

