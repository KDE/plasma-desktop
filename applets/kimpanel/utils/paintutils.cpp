#include "paintutils.h"
#include <kdebug.h>

namespace KIM
{
    QPixmap renderText(QString text, RenderType type)
    {
        Theme *theme = Theme::defaultTheme();
        switch (type) {
        case Statusbar:
            return renderText(text,theme->color(Theme::StatusbarTextColor),
                    theme->color(Theme::StatusbarBackgroundColor));
        case Auxilary:
            return renderText(text,theme->color(Theme::AuxilaryTextColor),
                    theme->color(Theme::AuxilaryBackgroundColor));
        case Preedit:
            return renderText(text,theme->color(Theme::PreeditTextColor),
                    theme->color(Theme::PreeditBackgroundColor));
        case TableLabel:
            return renderText(text,theme->color(Theme::LookupTableLabelTextColor),
                    theme->color(Theme::LookupTableLabelBackgroundColor));
        case TableEntry:
            return renderText(text,theme->color(Theme::LookupTableEntryTextColor),
                    theme->color(Theme::LookupTableEntryBackgroundColor));
        default:
            return renderText(text,theme->color(Theme::StatusbarTextColor),
                    theme->color(Theme::StatusbarBackgroundColor));
        }
    }

    QPixmap renderText(QString text, QColor textColor, QColor bgColor, const QFont &ft)
    {
        //don't try to paint stuff on a future null pixmap because the text is empty
        if (text.isEmpty()) {
            return QPixmap();
        }

        QFont font = ft;
        // Draw text
        QFontMetrics fm(font);
        QSize textSize = fm.size(0,text);
        QPixmap textPixmap(textSize);
        textPixmap.fill(bgColor);
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

} // namespace KIM
// vim: sw=4 sts=4 et tw=100
