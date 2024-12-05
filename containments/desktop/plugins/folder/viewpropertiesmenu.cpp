/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewpropertiesmenu.h"

#include <QActionGroup>
#include <QApplication>
#include <QMenu>

#include <KDirModel>
#include <KLocalizedString>

ViewPropertiesMenu::ViewPropertiesMenu(QObject *parent)
    : QObject(parent)
{
    m_menu = new QMenu();

    QMenu *menu = m_menu->addMenu(QIcon::fromTheme(QStringLiteral("view-sort")), i18n("Sort By"));
    m_sortMode = new QActionGroup(this);
    connect(m_sortMode, &QActionGroup::triggered, this, &ViewPropertiesMenu::sortModeChanged);
    QAction *action = menu->addAction(i18nc("@item:inmenu Sort icons manually", "Unsorted"));
    action->setCheckable(true);
    action->setData(-1);
    m_sortMode->addAction(action);
    action = menu->addAction(i18nc("@item:inmenu Sort icons by name", "Name"));
    action->setCheckable(true);
    action->setData(int(KDirModel::Name));
    m_sortMode->addAction(action);
    action = menu->addAction(i18nc("@item:inmenu Sort icons by size", "Size"));
    action->setCheckable(true);
    action->setData(int(KDirModel::Size));
    m_sortMode->addAction(action);
    action = menu->addAction(i18nc("@item:inmenu Sort icons by file type", "Type"));
    action->setCheckable(true);
    action->setData(int(KDirModel::Type));
    m_sortMode->addAction(action);
    action = menu->addAction(i18nc("@item:inmenu Sort icons by date", "Date"));
    action->setCheckable(true);
    action->setData(int(KDirModel::ModifiedTime));
    m_sortMode->addAction(action);
    menu->addSeparator();
    m_sortDesc = menu->addAction(i18nc("@item:inmenu Sort icons in descending order", "Descending"), this, &ViewPropertiesMenu::sortDescChanged);
    m_sortDesc->setCheckable(true);
    m_sortDirsFirst = menu->addAction(i18nc("@item:inmenu Sort icons with folders first", "Folders First"), this, &ViewPropertiesMenu::sortDirsFirstChanged);
    m_sortDirsFirst->setCheckable(true);

    m_iconSizeMenu = m_menu->addMenu(QIcon::fromTheme(QStringLiteral("transform-scale")), i18n("Icon Size"));
    m_iconSize = new QActionGroup(this);
    connect(m_iconSize, &QActionGroup::triggered, this, &ViewPropertiesMenu::iconSizeChanged);
    const QStringList iconSizes{i18nc("@item:inmenu size of the icons", "Tiny"),
                                i18nc("@item:inmenu size of the icons", "Very Small"),
                                i18nc("@item:inmenu size of the icons", "Small"),
                                i18nc("@item:inmenu size of the icons", "Small-Medium"),
                                i18nc("@item:inmenu size of the icons", "Medium"),
                                i18nc("@item:inmenu size of the icons", "Large"),
                                i18nc("@item:inmenu size of the icons", "Huge")};
    for (int i = 0; i < iconSizes.count(); ++i) {
        action = m_iconSizeMenu->addAction(iconSizes.at(i));
        action->setCheckable(true);
        action->setData(i);
        m_iconSize->addAction(action);
    }

    m_arrangementMenu = m_menu->addMenu(QIcon::fromTheme(QStringLiteral("object-rows")), i18nc("@item:inmenu arrangement of icons", "Arrange"));
    m_arrangement = new QActionGroup(this);
    connect(m_arrangement, &QActionGroup::triggered, this, &ViewPropertiesMenu::arrangementChanged);
    const auto isLtR = qApp->layoutDirection() == Qt::LeftToRight;
    action = m_arrangementMenu->addAction(isLtR ? i18nc("@item:inmenu arrangement of icons", "Left to Right")
                                                : i18nc("@item:inmenu arrangement of icons", "Right to Left"));
    action->setCheckable(true);
    action->setData(0);
    m_arrangement->addAction(action);
    action = m_arrangementMenu->addAction(i18nc("@item:inmenu arrangement of icons", "Top to Bottom"));
    action->setData(1);
    action->setCheckable(true);
    m_arrangement->addAction(action);

    m_alignmentMenu = m_menu->addMenu(QIcon::fromTheme(QStringLiteral("align-horizontal-left")), i18n("Align"));
    m_alignment = new QActionGroup(this);
    connect(m_alignment, &QActionGroup::triggered, this, &ViewPropertiesMenu::alignmentChanged);
    action = m_alignmentMenu->addAction(i18nc("@item:inmenu alignment of icons", "Left"));
    action->setCheckable(true);
    action->setData(0);
    m_alignment->addAction(action);
    action = m_alignmentMenu->addAction(i18nc("@item:inmenu alignment of icons", "Right"));
    action->setCheckable(true);
    action->setData(1);
    m_alignment->addAction(action);

    m_previews = m_menu->addAction(QIcon::fromTheme(QStringLiteral("view-preview")), i18n("Show Previews"), this, &ViewPropertiesMenu::previewsChanged);
    m_previews->setCheckable(true);

    m_locked = m_menu->addAction(QIcon::fromTheme(QStringLiteral("lock")),
                                 i18nc("@item:inmenu lock icon positions in place", "Locked"),
                                 this,
                                 &ViewPropertiesMenu::lockedChanged);
    m_locked->setCheckable(true);
}

ViewPropertiesMenu::~ViewPropertiesMenu()
{
    delete m_menu;
}

QObject *ViewPropertiesMenu::menu() const
{
    return m_menu;
}

bool ViewPropertiesMenu::showLayoutActions() const
{
    return m_alignmentMenu->menuAction()->isVisible() && m_arrangementMenu->menuAction()->isVisible();
}

void ViewPropertiesMenu::setShowLayoutActions(bool show)
{
    if (showLayoutActions() != show) {
        m_arrangementMenu->menuAction()->setVisible(show);
        m_alignmentMenu->menuAction()->setVisible(show);

        Q_EMIT showLayoutActionsChanged();
    }
}

bool ViewPropertiesMenu::showLockAction() const
{
    return m_locked->isVisible();
}

void ViewPropertiesMenu::setShowLockAction(bool show)
{
    if (m_locked->isVisible() != show) {
        m_locked->setVisible(show);

        Q_EMIT showLockActionChanged();
    }
}

bool ViewPropertiesMenu::showIconSizeActions() const
{
    return m_iconSizeMenu->menuAction()->isVisible();
}

void ViewPropertiesMenu::setShowIconSizeActions(bool show)
{
    if (showIconSizeActions() != show) {
        m_iconSizeMenu->menuAction()->setVisible(show);
        Q_EMIT showIconSizeActionsChanged();
    }
}

int ViewPropertiesMenu::arrangement() const
{
    return m_arrangement->checkedAction()->data().toInt();
}

void ViewPropertiesMenu::setArrangement(int arrangement)
{
    if (!m_arrangement->checkedAction() || m_arrangement->checkedAction()->data().toInt() != arrangement) {
        const QList<QAction *> actions = m_arrangement->actions();
        for (QAction *action : actions) {
            if (action->data().toInt() == arrangement) {
                action->setChecked(true);
                break;
            }
        }
    }
}

int ViewPropertiesMenu::alignment() const
{
    return m_alignment->checkedAction()->data().toInt();
}

void ViewPropertiesMenu::setAlignment(int alignment)
{
    if (!m_alignment->checkedAction() || m_alignment->checkedAction()->data().toInt() != alignment) {
        const QList<QAction *> actions = m_alignment->actions();
        for (QAction *action : actions) {
            if (action->data().toInt() == alignment) {
                action->setChecked(true);
                break;
            }
        }
    }
}

bool ViewPropertiesMenu::previews() const
{
    return m_previews->isChecked();
}

void ViewPropertiesMenu::setPreviews(bool previews)
{
    if (m_previews->isChecked() != previews) {
        m_previews->setChecked(previews);
    }
}

bool ViewPropertiesMenu::locked() const
{
    return m_locked->isChecked();
}

void ViewPropertiesMenu::setLocked(bool locked)
{
    if (m_locked->isChecked() != locked) {
        m_locked->setChecked(locked);
    }
}

bool ViewPropertiesMenu::lockedEnabled() const
{
    return m_locked->isEnabled();
}

void ViewPropertiesMenu::setLockedEnabled(bool enabled)
{
    if (m_locked->isEnabled() != enabled) {
        m_locked->setEnabled(enabled);
        Q_EMIT lockedEnabledChanged();
    }
}

int ViewPropertiesMenu::sortMode() const
{
    return m_sortMode->checkedAction()->data().toInt();
}

void ViewPropertiesMenu::setSortMode(int sortMode)
{
    if (!m_sortMode->checkedAction() || m_sortMode->checkedAction()->data().toInt() != sortMode) {
        const QList<QAction *> actions = m_sortMode->actions();
        for (QAction *action : actions) {
            if (action->data().toInt() == sortMode) {
                action->setChecked(true);
                break;
            }
        }
    }
}

bool ViewPropertiesMenu::sortDesc() const
{
    return m_sortDesc->isChecked();
}

void ViewPropertiesMenu::setSortDesc(bool sortDesc)
{
    if (m_sortDesc->isChecked() != sortDesc) {
        m_sortDesc->setChecked(sortDesc);
    }
}

bool ViewPropertiesMenu::sortDirsFirst() const
{
    return m_sortDirsFirst->isChecked();
}

void ViewPropertiesMenu::setSortDirsFirst(bool sortDirsFirst)
{
    if (m_sortDirsFirst->isChecked() != sortDirsFirst) {
        m_sortDirsFirst->setChecked(sortDirsFirst);
    }
}

int ViewPropertiesMenu::iconSize() const
{
    return m_iconSize->checkedAction()->data().toInt();
}

void ViewPropertiesMenu::setIconSize(int iconSize)
{
    if (!m_iconSize->checkedAction() || m_iconSize->checkedAction()->data().toInt() != iconSize) {
        QAction *action = m_iconSize->actions().value(iconSize);
        if (action) {
            action->setChecked(true);
        }
    }
}

#include "moc_viewpropertiesmenu.cpp"
