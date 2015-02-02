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

#include "FontFilter.h"
#include "FontFilterProxyStyle.h"
#include "FontList.h"
#include <KIconLoader>
#include <KToggleAction>
#include <KSelectAction>
#include <QIcon>
#include <KMimeType>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QSet>
#include <QString>
#include <QMenu>
#include <QMouseEvent>
#include <QApplication>
#include <QActionGroup>

namespace KFI
{

static const int constArrowPad(5);

static void deselectCurrent(QActionGroup *act)
{
    QAction *prev(act->checkedAction());
    if(prev)
        prev->setChecked(false);
}

static void deselectCurrent(KSelectAction *act)
{
    deselectCurrent(act->selectableActionGroup());
}

// FIXME: Go back to using StyleSheets instead of a proxy style
// once Qt has been fixed not to mess with widget font when
// using StyleSheets
class CFontFilterStyle : public CFontFilterProxyStyle
{
    public:

    CFontFilterStyle(CFontFilter *parent, int ol) : CFontFilterProxyStyle(parent), overlap(ol) {}

    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;

    int overlap;
};

QRect CFontFilterStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    if (SE_LineEditContents==element)
    {
        QRect rect(style()->subElementRect(SE_LineEditContents, option, widget));

        return rect.adjusted(overlap, 0, -overlap, 0);
    }

    return CFontFilterProxyStyle::subElementRect(element, option, widget);
}

struct SortAction
{
    SortAction(QAction *a) : action(a)        { }
    bool operator<(const SortAction &o) const { return action->text().localeAwareCompare(o.action->text())<0; }
    QAction *action;
};

static void sortActions(KSelectAction *group)
{
    if(group->actions().count()>1)
    {
        QList<QAction *>                actions=group->actions();
        QList<QAction *>::ConstIterator it(actions.constBegin()),
                                        end(actions.constEnd());
        QList<SortAction>               sorted;

        for(; it!=end; ++it)
        {
            sorted.append(SortAction(*it));
            group->removeAction(*it);
        }

        qSort(sorted);
        QList<SortAction>::ConstIterator s(sorted.constBegin()),
                                         sEnd(sorted.constEnd());

        for(; s!=sEnd; ++s)
            group->addAction((*s).action);
    }
}

CFontFilter::CFontFilter(QWidget *parent)
           : KLineEdit(parent)
{
    setClearButtonShown(true);
    setTrapReturnKey(true);

    itsMenuButton = new QLabel(this);
    itsMenuButton->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    itsMenuButton->setCursor(Qt::ArrowCursor);
    itsMenuButton->setToolTip(i18n("Set Criteria"));

    itsMenu=new QMenu(this);
    itsPixmaps[CRIT_FAMILY]=SmallIcon("draw-text");
    itsPixmaps[CRIT_STYLE]=SmallIcon("format-text-bold");
    itsPixmaps[CRIT_FOUNDRY]=SmallIcon("user-identity");
    itsPixmaps[CRIT_FONTCONFIG]=SmallIcon("system-search");
    itsPixmaps[CRIT_FILETYPE]=SmallIcon("preferences-desktop-font-installer");
    itsPixmaps[CRIT_FILENAME]=SmallIcon("application-x-font-type1");
    itsPixmaps[CRIT_LOCATION]=SmallIcon("folder");
    itsPixmaps[CRIT_WS]=SmallIcon("character-set");

    itsActionGroup=new QActionGroup(this);
    addAction(CRIT_FAMILY, i18n("Family"), true);
    addAction(CRIT_STYLE, i18n("Style"), false);

    KSelectAction *foundryMenu=new KSelectAction(QIcon(itsPixmaps[CRIT_FOUNDRY]), i18n("Foundry"), this);
    itsActions[CRIT_FOUNDRY]=foundryMenu;
    itsMenu->addAction(itsActions[CRIT_FOUNDRY]);
    foundryMenu->setData((int)CRIT_FOUNDRY);
    foundryMenu->setVisible(false);
    connect(foundryMenu, SIGNAL(triggered(QString)), SLOT(foundryChanged(QString)));

    addAction(CRIT_FONTCONFIG, i18n("FontConfig Match"), false);
    
    KSelectAction *ftMenu=new KSelectAction(QIcon(itsPixmaps[CRIT_FILETYPE]), i18n("File Type"), this);
    itsActions[CRIT_FILETYPE]=ftMenu;
    itsMenu->addAction(itsActions[CRIT_FILETYPE]);
    ftMenu->setData((int)CRIT_FILETYPE);
    
    QStringList::ConstIterator it(CFontList::fontMimeTypes.constBegin()),
                               end(CFontList::fontMimeTypes.constEnd());
                               
    for(; it!=end; ++it)
        if((*it)!="application/vnd.kde.fontspackage")
        {
            KMimeType::Ptr mime=KMimeType::mimeType(*it);
            
            KToggleAction *act=new KToggleAction(QIcon::fromTheme(mime->iconName()), mime->comment(), this);

            ftMenu->addAction(act);
            act->setChecked(false);
            
            QStringList::ConstIterator sIt(mime->patterns().constBegin()),
                                       sEnd(mime->patterns().constEnd());
            QStringList                mimes;
                                       
            for(; sIt!=sEnd; ++sIt)
                mimes.append(QString(*sIt).replace("*.", ""));
            act->setData(mimes);
        }

    sortActions(ftMenu);
    connect(ftMenu, SIGNAL(triggered(QString)), SLOT(ftChanged(QString)));
    itsCurrentFileTypes.clear();

    addAction(CRIT_FILENAME, i18n("File Name"), false);
    addAction(CRIT_LOCATION, i18n("File Location"), false);

    KSelectAction *wsMenu=new KSelectAction(QIcon(itsPixmaps[CRIT_WS]), i18n("Writing System"), this);
    itsActions[CRIT_WS]=wsMenu;
    itsMenu->addAction(itsActions[CRIT_WS]);
    wsMenu->setData((int)CRIT_WS);

    itsCurrentWs=QFontDatabase::Any;
    for(int i=QFontDatabase::Latin; i<QFontDatabase::WritingSystemsCount; ++i)
    {
        KToggleAction *wsAct=new KToggleAction(QFontDatabase::Other==i
                                                ? i18n("Symbol/Other")
                                                : QFontDatabase::writingSystemName((QFontDatabase::WritingSystem)i), this);

        wsMenu->addAction(wsAct);
        wsAct->setChecked(false);
        wsAct->setData(i);
    }
    sortActions(wsMenu);
    connect(wsMenu, SIGNAL(triggered(QString)), SLOT(wsChanged(QString)));

    setCriteria(CRIT_FAMILY);
    setStyle(new CFontFilterStyle(this, itsMenuButton->width()));
}

void CFontFilter::setFoundries(const QSet<QString> &currentFoundries)
{
    QAction                         *act(((KSelectAction *)itsActions[CRIT_FOUNDRY])->currentAction());
    QString                         prev(act && act->isChecked() ? act->text() : QString());
    bool                            changed(false);
    QList<QAction *>                prevFoundries(((KSelectAction *)itsActions[CRIT_FOUNDRY])->actions());
    QList<QAction *>::ConstIterator fIt(prevFoundries.constBegin()),
                                    fEnd(prevFoundries.constEnd());
    QSet<QString>                   foundries(currentFoundries);

    // Determine which of 'foundries' are new ones, and which old ones need to be removed...
    for(; fIt!=fEnd; ++fIt)
    {
        if(foundries.contains((*fIt)->text()))
            foundries.remove((*fIt)->text());
        else
        {
            ((KSelectAction *)itsActions[CRIT_FOUNDRY])->removeAction(*fIt);
            (*fIt)->deleteLater();
            changed=true;
        }
    }

    if(foundries.count())
    {
        // Add foundries to menu - replacing '&' with '&&', as '&' is taken to be
        // a shortcut!
        QSet<QString>::ConstIterator it(foundries.begin()),
                                     end(foundries.end());

        for(; it!=end; ++it)
        {
            QString foundry(*it);

            foundry.replace("&", "&&");
            ((KSelectAction *)itsActions[CRIT_FOUNDRY])->addAction(foundry);
        }
        changed=true;
    }

    if(changed)
    {
        sortActions((KSelectAction *)itsActions[CRIT_FOUNDRY]);
        if(!prev.isEmpty())
        {
            act=((KSelectAction *)itsActions[CRIT_FOUNDRY])->action(prev);
            if(act)
                ((KSelectAction *)itsActions[CRIT_FOUNDRY])->setCurrentAction(act);
            else
                ((KSelectAction *)itsActions[CRIT_FOUNDRY])->setCurrentItem(0);
        }
    
        itsActions[CRIT_FOUNDRY]->setVisible(((KSelectAction *)itsActions[CRIT_FOUNDRY])->actions().count());
    }
}

QSize CFontFilter::sizeHint() const
{
    return QSize(fontMetrics().width(clickMessage())+56, KLineEdit::sizeHint().height());
}

void CFontFilter::filterChanged()
{
    QAction *act(itsActionGroup->checkedAction());

    if(act)
    {
        ECriteria crit((ECriteria)act->data().toInt());

        if(itsCurrentCriteria!=crit)
        {
            deselectCurrent((KSelectAction *)itsActions[CRIT_FOUNDRY]);
            deselectCurrent((KSelectAction *)itsActions[CRIT_FILETYPE]);
            deselectCurrent((KSelectAction *)itsActions[CRIT_WS]);
            setText(QString());
            itsCurrentWs=QFontDatabase::Any;
            itsCurrentFileTypes.clear();

            setCriteria(crit);
            setClickMessage(i18n("Type here to filter on %1", act->text().toLower()));
            setReadOnly(false);
        }
    }
}

void CFontFilter::ftChanged(const QString &ft)
{
    deselectCurrent((KSelectAction *)itsActions[CRIT_FOUNDRY]);
    deselectCurrent((KSelectAction *)itsActions[CRIT_WS]);
    deselectCurrent(itsActionGroup);

    QAction *act(((KSelectAction *)itsActions[CRIT_FILETYPE])->currentAction());

    if(act)
        itsCurrentFileTypes=act->data().toStringList();
    itsCurrentCriteria=CRIT_FILETYPE;
    setReadOnly(true);
    setCriteria(itsCurrentCriteria);
    setText(ft);
    setClickMessage(text());
}

void CFontFilter::wsChanged(const QString &writingSystemName)
{
    deselectCurrent((KSelectAction *)itsActions[CRIT_FOUNDRY]);
    deselectCurrent((KSelectAction *)itsActions[CRIT_FILETYPE]);
    deselectCurrent(itsActionGroup);

    QAction *act(((KSelectAction *)itsActions[CRIT_WS])->currentAction());

    if(act)
        itsCurrentWs=(QFontDatabase::WritingSystem)act->data().toInt();
    itsCurrentCriteria=CRIT_WS;
    setReadOnly(true);
    setCriteria(itsCurrentCriteria);
    setText(writingSystemName);
    setClickMessage(text());
}

void CFontFilter::foundryChanged(const QString &foundry)
{
    deselectCurrent((KSelectAction *)itsActions[CRIT_WS]);
    deselectCurrent((KSelectAction *)itsActions[CRIT_FILETYPE]);
    deselectCurrent(itsActionGroup);

    itsCurrentCriteria=CRIT_FOUNDRY;
    setReadOnly(true);
    setText(foundry);
    setClickMessage(text());
    setCriteria(itsCurrentCriteria);
}

void CFontFilter::addAction(ECriteria crit, const QString &text, bool on)
{
    itsActions[crit]=new KToggleAction(QIcon(itsPixmaps[crit]),
                                       text, this);
    itsMenu->addAction(itsActions[crit]);
    itsActionGroup->addAction(itsActions[crit]);
    itsActions[crit]->setData((int)crit);
    itsActions[crit]->setChecked(on);
    if(on)
        setClickMessage(i18n("Type here to filter on %1", text.toLower()));
    connect(itsActions[crit], SIGNAL(toggled(bool)), SLOT(filterChanged()));
}

void CFontFilter::resizeEvent(QResizeEvent *ev)
{
    KLineEdit::resizeEvent(ev);

    int frameWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth)),
        y((height()-itsMenuButton->height())/2);

    if (qApp->isLeftToRight())
        itsMenuButton->move(frameWidth + 2, y);
    else
        itsMenuButton->move(size().width() - frameWidth - itsMenuButton->width() - 2, y);
}

void CFontFilter::mousePressEvent(QMouseEvent *ev)
{
    if(Qt::LeftButton==ev->button() && itsMenuButton->underMouse())
        itsMenu->popup(mapToGlobal(QPoint(0, height())), 0);
    else
        KLineEdit::mousePressEvent(ev);
}

void CFontFilter::setCriteria(ECriteria crit)
{
    QPixmap arrowmap(itsPixmaps[crit].width()+constArrowPad, itsPixmaps[crit].height());

    QColor bgnd(palette().color(QPalette::Active, QPalette::Base));
    bgnd.setAlphaF(0.0);
    arrowmap.fill(bgnd);

    QPainter p(&arrowmap);

    p.drawPixmap(0, 0, itsPixmaps[crit]);
    QStyleOption opt;
    opt.state = QStyle::State_Enabled;
    opt.rect = QRect(arrowmap.width()-(constArrowPad+1), arrowmap.height()-(constArrowPad+1), constArrowPad, constArrowPad);
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, &p, itsMenuButton);
    p.end();

    itsMenuButton->setPixmap(arrowmap);
    itsMenuButton->resize(arrowmap.width(), arrowmap.height());
    itsCurrentCriteria=crit;

    emit criteriaChanged(crit, ((qulonglong)1) << (int)itsCurrentWs, itsCurrentFileTypes);
}

}

#include "FontFilter.moc"
