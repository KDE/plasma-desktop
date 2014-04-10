/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz                                      *
 *   peter.penz@gmx.at                                                     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#include "konq_statusbarmessagelabel.h"
#include <QStyle>
#include <QTextDocument>

#include <kcolorscheme.h>
#include <kiconloader.h>
#include <kicon.h>
#include <klocale.h>
#include <kdebug.h>

#include <QFontMetrics>
#include <QPainter>
#include <QKeyEvent>
#include <QPixmap>
#include <QToolButton>
#include <QTimer>

enum { GeometryTimeout = 100 };
enum { BorderGap = 2 };

class KonqStatusBarMessageLabel::Private
{
public:
    Private() :
        m_type(Default),
        m_state(DefaultState),
        m_illumination(0),
        m_minTextHeight(-1),
        m_timer(0),
        m_closeButton(0)
    {}

    bool isRichText() const { return m_text.startsWith("<html>") || m_text.startsWith("<qt>"); }

    KonqStatusBarMessageLabel::Type m_type;
    KonqStatusBarMessageLabel::State m_state;
    int m_illumination;
    int m_minTextHeight;
    QTimer* m_timer;
    QString m_text;
    QString m_defaultText;
    QTextDocument m_textDocument;
    QList<QString> m_pendingMessages;
    QPixmap m_pixmap;
    QToolButton* m_closeButton;
};

KonqStatusBarMessageLabel::KonqStatusBarMessageLabel(QWidget* parent) :
    QWidget(parent), d(new Private)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum /*the sizeHint is the max*/);

    d->m_timer = new QTimer(this);
    connect(d->m_timer, SIGNAL(timeout()),
            this, SLOT(timerDone()));

    d->m_closeButton = new QToolButton(this);
    d->m_closeButton->setAutoRaise(true);
    d->m_closeButton->setIcon(KIcon("dialog-close"));
    d->m_closeButton->setToolTip(i18nc("@info", "Close"));
    d->m_closeButton->setAccessibleName(i18n("Close"));
    d->m_closeButton->hide();
    connect(d->m_closeButton, SIGNAL(clicked()),
            this, SLOT(closeErrorMessage()));
}

KonqStatusBarMessageLabel::~KonqStatusBarMessageLabel()
{
    delete d;
}

void KonqStatusBarMessageLabel::setMessage(const QString& text,
                                           Type type)
{
    if ((text == d->m_text) && (type == d->m_type)) {
        return;
    }

    if (d->m_type == Error) {
        if (type == Error) {
            d->m_pendingMessages.insert(0, d->m_text);
        } else if ((d->m_state != DefaultState) || !d->m_pendingMessages.isEmpty()) {
            // a non-error message should not be shown, as there
            // are other pending error messages in the queue
            return;
        }
    }

    d->m_text = text;
    d->m_type = type;

    if (d->isRichText()) {
        d->m_textDocument.setTextWidth(-1);
        d->m_textDocument.setDefaultFont(font());
        QString html = "<html><font color=\"";
        html += palette().windowText().color().name();
        html += "\">";
        html += d->m_text;
        d->m_textDocument.setHtml(html);
    }

    d->m_timer->stop();
    d->m_illumination = 0;
    d->m_state = DefaultState;

    const char* iconName = 0;
    QPixmap pixmap;
    switch (type) {
    case OperationCompleted:
        iconName = "dialog-ok";
        // "ok" icon should probably be "dialog-success", but we don't have that icon in KDE 4.0
        d->m_closeButton->hide();
        break;

    case Information:
        iconName = "dialog-information";
        d->m_closeButton->hide();
        break;

    case Error:
        d->m_timer->start(100);
        d->m_state = Illuminate;

        updateCloseButtonPosition();
        d->m_closeButton->show();
        updateGeometry();
        break;

    case Default:
    default:
        d->m_closeButton->hide();
        updateGeometry();
        break;
    }

    d->m_pixmap = (iconName == 0) ? QPixmap() : SmallIcon(iconName);
    QTimer::singleShot(GeometryTimeout, this, SLOT(assureVisibleText()));

    if (type == Error) {
        setAccessibleName(i18n("Error: %1", text));
    } else {
        setAccessibleName(text);
    }

    update();
}

KonqStatusBarMessageLabel::Type KonqStatusBarMessageLabel::type() const
{
    return d->m_type;
}

QString KonqStatusBarMessageLabel::text() const
{
    return d->m_text;
}

void KonqStatusBarMessageLabel::setDefaultText(const QString& text)
{
    d->m_defaultText = text;
}

QString KonqStatusBarMessageLabel::defaultText() const
{
    return d->m_defaultText;
}

void KonqStatusBarMessageLabel::setMinimumTextHeight(int min)
{
    if (min != d->m_minTextHeight) {
        d->m_minTextHeight = min;
        setMinimumHeight(min);
        if (d->m_closeButton->height() > min) {
            d->m_closeButton->setFixedHeight(min);
        }
    }
}

int KonqStatusBarMessageLabel::minimumTextHeight() const
{
    return d->m_minTextHeight;
}

void KonqStatusBarMessageLabel::paintEvent(QPaintEvent* /* event */)
{
    QPainter painter(this);

    if (d->m_illumination > 0) {
        // at this point, a: we are a second label being drawn over the already
        // painted status area, so we can be translucent, and b: our palette's
        // window color (bg only) seems to be wrong (always black)
        KColorScheme scheme(palette().currentColorGroup(), KColorScheme::Window);
        QColor backgroundColor = scheme.background(KColorScheme::NegativeBackground).color();
        backgroundColor.setAlpha(qMin(255, d->m_illumination * 2));
        painter.setBrush(backgroundColor);
        painter.setPen(Qt::NoPen);
        painter.drawRect(QRect(0, 0, width(), height()));
    }

    // draw pixmap
    int x = BorderGap;
    const int y = (d->m_minTextHeight - d->m_pixmap.height()) / 2;

    if (!d->m_pixmap.isNull()) {
        painter.drawPixmap(x, y, d->m_pixmap);
        x += d->m_pixmap.width() + BorderGap;
    }

    // draw text

    const QRect availTextRect(x, 0, availableTextWidth(), height());

    if (d->isRichText()) {
        const QSize sz = d->m_textDocument.size().toSize();

        // Vertical centering
        const QRect textRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignLeft | Qt::AlignVCenter, sz, availTextRect);
        //kDebug() << d->m_text << " sz=" << sz << textRect;

        // What about wordwrap here?

        painter.translate(textRect.left(), textRect.top());
        d->m_textDocument.drawContents(&painter);
    } else {
        // plain text
        painter.setPen(palette().windowText().color());
        int flags = Qt::AlignVCenter;
        if (height() > d->m_minTextHeight) {
            flags = flags | Qt::TextWordWrap;
        }
        painter.drawText(availTextRect, flags, d->m_text);
    }
    painter.end();
}

void KonqStatusBarMessageLabel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateCloseButtonPosition();
    QTimer::singleShot(GeometryTimeout, this, SLOT(assureVisibleText()));
}

void KonqStatusBarMessageLabel::timerDone()
{
    switch (d->m_state) {
    case Illuminate: {
        // increase the illumination
        const int illumination_max = 128;
        if (d->m_illumination < illumination_max) {
            d->m_illumination += 32;
            if (d->m_illumination > illumination_max) {
                d->m_illumination = illumination_max;
            }
            update();
        } else {
            d->m_state = Illuminated;
            d->m_timer->start(5000);
        }
        break;
    }

    case Illuminated: {
        // start desaturation
        d->m_state = Desaturate;
        d->m_timer->start(100);
        break;
    }

    case Desaturate: {
        // desaturate
        if (d->m_illumination > 0) {
            d->m_illumination -= 5;
            update();
        } else {
            d->m_state = DefaultState;
            d->m_timer->stop();
        }
        break;
    }

    default:
        break;
    }
}

void KonqStatusBarMessageLabel::assureVisibleText()
{
    if (d->m_text.isEmpty()) {
        return;
    }

    int requiredHeight = d->m_minTextHeight;
    if (d->m_type != Default) {
        // Calculate the required height of the widget thats
        // needed for having a fully visible text. Note that for the default
        // statusbar type (e. g. hover information) increasing the text height
        // is not wanted, as this might rearrange the layout of items.

        QFontMetrics fontMetrics(font());
        const QRect bounds(fontMetrics.boundingRect(0, 0, availableTextWidth(), height(),
                           Qt::AlignVCenter | Qt::TextWordWrap, d->m_text));
        requiredHeight = bounds.height();
        if (requiredHeight < d->m_minTextHeight) {
            requiredHeight = d->m_minTextHeight;
        }
    }

    // Increase/decrease the current height of the widget to the
    // required height. The increasing/decreasing is done in several
    // steps to have an animation if the height is modified
    // (see KonqStatusBarMessageLabel::resizeEvent())
    const int gap = d->m_minTextHeight / 2;
    int minHeight = minimumHeight();
    if (minHeight < requiredHeight) {
        minHeight += gap;
        if (minHeight > requiredHeight) {
            minHeight = requiredHeight;
        }
        setMinimumHeight(minHeight);
        updateGeometry();
    } else if (minHeight > requiredHeight) {
        minHeight -= gap;
        if (minHeight < requiredHeight) {
            minHeight = requiredHeight;
        }
        setMinimumHeight(minHeight);
        updateGeometry();
    }

    updateCloseButtonPosition();
}

int KonqStatusBarMessageLabel::availableTextWidth() const
{
    const int buttonWidth = (d->m_type == Error) ?
                            d->m_closeButton->width() + BorderGap : 0;
    return width() - d->m_pixmap.width() - (BorderGap * 4) - buttonWidth;
}

void KonqStatusBarMessageLabel::updateCloseButtonPosition()
{
    const int x = width() - d->m_closeButton->width() - BorderGap;
    d->m_closeButton->move(x, 0);
}

void KonqStatusBarMessageLabel::closeErrorMessage()
{
    if (!showPendingMessage()) {
        d->m_state = DefaultState;
        setMessage(d->m_defaultText, Default);
    }
}

bool KonqStatusBarMessageLabel::showPendingMessage()
{
    if (!d->m_pendingMessages.isEmpty()) {
        reset();
        setMessage(d->m_pendingMessages.takeFirst(), Error);
        return true;
    }
    return false;
}

void KonqStatusBarMessageLabel::reset()
{
    d->m_text.clear();
    d->m_type = Default;
}

QSize KonqStatusBarMessageLabel::sizeHint() const
{
    return minimumSizeHint();
}

QSize KonqStatusBarMessageLabel::minimumSizeHint() const
{
    const int fontHeight = fontMetrics().height();
    QSize sz(100, fontHeight);
    if (d->m_closeButton->isVisible()) {
        const QSize toolButtonSize = d->m_closeButton->sizeHint();
        sz = toolButtonSize.expandedTo(sz);
    }
    return sz;
}

#include "konq_statusbarmessagelabel.moc"
