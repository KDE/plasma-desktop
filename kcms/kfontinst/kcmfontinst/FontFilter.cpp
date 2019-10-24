/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *           2019      Guo Yunhe <i@guoyunhe.me>
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
#include <QMimeDatabase>
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

    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;

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

        std::sort(sorted.begin(), sorted.end());
        QList<SortAction>::ConstIterator s(sorted.constBegin()),
                                         sEnd(sorted.constEnd());

        for(; s!=sEnd; ++s)
            group->addAction((*s).action);
    }
}

CFontFilter::CFontFilter(QWidget *parent)
           : QWidget(parent)
{
    itsIcons[CRIT_FAMILY] = QIcon::fromTheme("draw-text");
    itsTexts[CRIT_FAMILY] = i18n("Family");
    itsIcons[CRIT_STYLE] = QIcon::fromTheme("format-text-bold");
    itsTexts[CRIT_STYLE] = i18n("Style");
    itsIcons[CRIT_FOUNDRY] = QIcon::fromTheme("user-identity");
    itsTexts[CRIT_FOUNDRY] = i18n("Foundry");
    itsIcons[CRIT_FONTCONFIG] = QIcon::fromTheme("system-search");
    itsTexts[CRIT_FONTCONFIG] = i18n("FontConfig Match");
    itsIcons[CRIT_FILETYPE] = QIcon::fromTheme("preferences-desktop-font-installer");
    itsTexts[CRIT_FILETYPE] = i18n("File Type");
    itsIcons[CRIT_FILENAME] = QIcon::fromTheme("application-x-font-type1");
    itsTexts[CRIT_FILENAME] = i18n("File Name");
    itsIcons[CRIT_LOCATION] = QIcon::fromTheme("folder");
    itsTexts[CRIT_LOCATION] = i18n("File Location");
    itsIcons[CRIT_WS] = QIcon::fromTheme("character-set");
    itsTexts[CRIT_WS] = i18n("Writing System");

    m_layout = new QHBoxLayout(this);
    setLayout(m_layout);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setClearButtonEnabled(true);
    m_layout->addWidget(m_lineEdit);

    m_menuButton = new QPushButton(this);
    m_menuButton->setIcon(QIcon::fromTheme("view-filter"));
    m_menuButton->setText(i18n("Set Criteria"));
    m_layout->addWidget(m_menuButton);

    connect(m_lineEdit, &QLineEdit::textChanged, this, &CFontFilter::textChanged);

    m_menu=new QMenu(this);
    m_menuButton->setMenu(m_menu);

    itsActionGroup=new QActionGroup(this);
    addAction(CRIT_FAMILY, true);
    addAction(CRIT_STYLE, false);

    KSelectAction *foundryMenu=new KSelectAction(itsIcons[CRIT_FOUNDRY], itsTexts[CRIT_FOUNDRY], this);
    itsActions[CRIT_FOUNDRY]=foundryMenu;
    m_menu->addAction(itsActions[CRIT_FOUNDRY]);
    foundryMenu->setData((int)CRIT_FOUNDRY);
    foundryMenu->setVisible(false);
    connect(foundryMenu, SIGNAL(triggered(QString)), SLOT(foundryChanged(QString)));

    addAction(CRIT_FONTCONFIG, false);

    KSelectAction *ftMenu=new KSelectAction(itsIcons[CRIT_FILETYPE], itsTexts[CRIT_FILETYPE], this);
    itsActions[CRIT_FILETYPE]=ftMenu;
    m_menu->addAction(itsActions[CRIT_FILETYPE]);
    ftMenu->setData((int)CRIT_FILETYPE);

    QStringList::ConstIterator it(CFontList::fontMimeTypes.constBegin()),
                               end(CFontList::fontMimeTypes.constEnd());
    QMimeDatabase db;
    for(; it!=end; ++it)
        if((*it)!="application/vnd.kde.fontspackage")
        {
            QMimeType mime = db.mimeTypeForName(*it);

            KToggleAction *act=new KToggleAction(QIcon::fromTheme(mime.iconName()), mime.comment(), this);

            ftMenu->addAction(act);
            act->setChecked(false);

            QStringList mimes;
            foreach (QString pattern, mime.globPatterns())
                mimes.append(pattern.remove(QStringLiteral("*.")));
            act->setData(mimes);
        }

    sortActions(ftMenu);
    connect(ftMenu, SIGNAL(triggered(QString)), SLOT(ftChanged(QString)));
    itsCurrentFileTypes.clear();

    addAction(CRIT_FILENAME, false);
    addAction(CRIT_LOCATION, false);

    KSelectAction *wsMenu=new KSelectAction(itsIcons[CRIT_WS], itsTexts[CRIT_WS], this);
    itsActions[CRIT_WS]=wsMenu;
    m_menu->addAction(itsActions[CRIT_WS]);
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
    setStyle(new CFontFilterStyle(this, m_menuButton->width()));
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

    if(!foundries.isEmpty())
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
            m_lineEdit->setText(QString());
            itsCurrentWs=QFontDatabase::Any;
            itsCurrentFileTypes.clear();

            setCriteria(crit);
            m_lineEdit->setPlaceholderText(i18n("Filter by %1...", act->text()));
            m_lineEdit->setReadOnly(false);
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
    m_lineEdit->setReadOnly(true);
    setCriteria(itsCurrentCriteria);
    m_lineEdit->setText(ft);
    m_lineEdit->setPlaceholderText(m_lineEdit->text());
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
    m_lineEdit->setReadOnly(true);
    setCriteria(itsCurrentCriteria);
    m_lineEdit->setText(writingSystemName);
    m_lineEdit->setPlaceholderText(m_lineEdit->text());
}

void CFontFilter::foundryChanged(const QString &foundry)
{
    deselectCurrent((KSelectAction *)itsActions[CRIT_WS]);
    deselectCurrent((KSelectAction *)itsActions[CRIT_FILETYPE]);
    deselectCurrent(itsActionGroup);

    itsCurrentCriteria=CRIT_FOUNDRY;
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setText(foundry);
    m_lineEdit->setPlaceholderText(m_lineEdit->text());
    setCriteria(itsCurrentCriteria);
}

void CFontFilter::textChanged(const QString &text)
{
    emit queryChanged(text);
}

void CFontFilter::addAction(ECriteria crit, bool on)
{
    itsActions[crit]=new KToggleAction(itsIcons[crit], itsTexts[crit], this);
    m_menu->addAction(itsActions[crit]);
    itsActionGroup->addAction(itsActions[crit]);
    itsActions[crit]->setData((int)crit);
    itsActions[crit]->setChecked(on);
    if(on)
        m_lineEdit->setPlaceholderText(i18n("Filter by %1...", itsTexts[crit]));
    connect(itsActions[crit], &QAction::toggled, this, &CFontFilter::filterChanged);
}

void CFontFilter::setCriteria(ECriteria crit)
{
    itsCurrentCriteria=crit;

    emit criteriaChanged(crit, ((qulonglong)1) << (int)itsCurrentWs, itsCurrentFileTypes);
}

}

