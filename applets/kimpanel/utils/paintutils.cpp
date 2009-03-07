#include "paintutils.h"

QPixmap renderText(QString text, const QFont &ft, QColor textColor)
{
    //don't try to paint stuff on a future null pixmap because the text is empty
    if (text.isEmpty()) {
        return QPixmap();
    }

    QFont font = ft;
    font.setPixelSize(256);
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
// vim: sw=4 sts=4 et tw=100
