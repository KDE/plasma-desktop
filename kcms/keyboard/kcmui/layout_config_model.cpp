/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "layout_config_model.h"

#include <QDebug>
#include <algorithm>
#include <KLocalizedString>

#include "../xkb_rules.h"

#include "layout_list_model_selected.h"

const char* LayoutConfigModel::switch_modes[4] = {
    "Global", "Desktop", "Application", "Window"
};

LayoutConfigModel::LayoutConfigModel(QObject* parent)
    : QObject(parent)
    , m_layoutListModels(new LayoutListModels(this))
{
    m_mainShiftKeyIndex = 0;
    m_thirdLevelShortcutIndex = 0;
}

bool LayoutConfigModel::showLayoutIndicator() const
{
    return m_showLayoutIndicator;
}

void LayoutConfigModel::setShowLayoutIndicator(bool showLayoutIndicator)
{
    if (showLayoutIndicator != m_showLayoutIndicator) {
        m_showLayoutIndicator = showLayoutIndicator;
        emit showLayoutIndicatorChanged();
    }
}

bool LayoutConfigModel::showForSingleLayout() const
{
    return m_showForSingleLayout;
}

void LayoutConfigModel::setShowForSingleLayout(bool showForSingleLayout)
{
    if (m_showForSingleLayout != showForSingleLayout) {
        m_showForSingleLayout = showForSingleLayout;
        emit showForSingleLayoutChanged();
    }
}

bool LayoutConfigModel::layoutIndicatorShowFlag() const
{
    return m_layoutIndicatorShowFlag;
}

void LayoutConfigModel::setLayoutIndicatorShowFlag(bool b)
{
    if (m_layoutIndicatorShowFlag != b) {
        m_layoutIndicatorShowFlag = b;
        emit layoutIndicatorShowFlagChanged();
    }
}

bool LayoutConfigModel::layoutIndicatorShowLabel() const
{
    return m_layoutIndicatorShowLabel;
}

void LayoutConfigModel::setLayoutIndicatorShowLabel(bool b)
{
    if (m_layoutIndicatorShowLabel != b) {
        m_layoutIndicatorShowLabel = b;
        emit layoutIndicatorShowLabelChanged();
    }
}

int LayoutConfigModel::switchingPolicyIndex() const
{
    return m_switchingPolicyIndex;
}

void LayoutConfigModel::setSwitchingPolicyIndex(int switchingPolicyIndex)
{
    if (m_switchingPolicyIndex != switchingPolicyIndex) {
        m_switchingPolicyIndex = switchingPolicyIndex;
        emit switchingPolicyIndexChanged();
    }
}

QString LayoutConfigModel::alternativeShortcut() const
{
    return m_alternativeShortcuts;
}

void LayoutConfigModel::setAlternativeShortcut(QString alternativeShortcut)
{
    if (m_alternativeShortcuts != alternativeShortcut) {
        m_alternativeShortcuts = alternativeShortcut;
        emit alternativeShortcutChanged();
    }
}

LayoutListFilterDuplicatesProxyModel* LayoutConfigModel::layoutListModel() const
{
    return m_layoutListModels->entireLayoutListModel();
}

LayoutListSortByPriorityProxyModel* LayoutConfigModel::currentLayoutListModel() const
{
    return m_layoutListModels->configuredLayoutListModel();
}

void LayoutConfigModel::loadEnabledLayouts()
{
    m_layoutListModels->loadConfig();
}

void LayoutConfigModel::saveEnabledLayouts()
{
    m_layoutListModels->saveConfig();
}

QStringList LayoutConfigModel::mainShiftKeyModel() const
{
    QStringList list;
    list << i18n("None");
    for (auto const& item : XkbRules::self()->optionGroupInfos) {
        if (item->name == "grp") {
            for (auto const& option : item->optionInfos) {
                list << option->description;
            }
            break;
        }
    }
    return list;
}

QStringList LayoutConfigModel::thirdLevelShortcutModel() const
{
    QStringList list;
    list << i18n("None");
    for (auto const& item : XkbRules::self()->optionGroupInfos) {
        qDebug() << item ->name << item ->description;
        if (item->name == "lv3") {
            for (auto const& option : item->optionInfos) {
                list << option->description;
            }
            break;
        }
    }
    return list;
}

int LayoutConfigModel::mainShiftKeyIndex() const
{
    return m_mainShiftKeyIndex;
}

QString LayoutConfigModel::mainShiftKey() const
{
    if (m_mainShiftKeyIndex > 0) {
        for (auto const& item : XkbRules::self()->optionGroupInfos) {
            if (item->name == "grp") {
                return item->optionInfos[m_mainShiftKeyIndex - 1]->name;
            }
        }
    }
    return "";
}

void LayoutConfigModel::setMainShiftKeyIndex(int mainShiftKeyIndex)
{
    if (m_mainShiftKeyIndex != mainShiftKeyIndex) {
        m_mainShiftKeyIndex = mainShiftKeyIndex;
        emit mainShiftKeyIndexChanged();
    }
}

void LayoutConfigModel::setMainShiftKey(const QString &name)
{
    for (auto const& item : XkbRules::self()->optionGroupInfos) {
        if (item->name == "grp") {
            for (int i = 0; i < item->optionInfos.size(); ++i) {
                if (item->optionInfos[i]->name == name) {
                    setMainShiftKeyIndex(i);
                }
            }
            break;
        }
    }
}

int LayoutConfigModel::thirdLevelShortcutIndex() const
{
    return m_thirdLevelShortcutIndex;
}

QString LayoutConfigModel::thirdLevelShortcut() const
{
    if (m_thirdLevelShortcutIndex > 0) {
        for (auto const& item : XkbRules::self()->optionGroupInfos) {
            if (item->name == "lv3") {
                return item->optionInfos[m_thirdLevelShortcutIndex - 1]->name;
            }
        }
    }
    return "";
}

void LayoutConfigModel::setThirdLevelShortcutIndex(int thirdLevelShortcut)
{
    if (m_thirdLevelShortcutIndex != thirdLevelShortcut) {
        m_thirdLevelShortcutIndex = thirdLevelShortcut;
        emit thirdLevelShortcutChanged();
    }
}

void LayoutConfigModel::setThirdLevelShortcut(const QString &name)
{
    for (auto const& item : XkbRules::self()->optionGroupInfos) {
        if (item->name == "lv3") {
            for (int i = 0; i < item->optionInfos.size(); ++i) {
                if (item->optionInfos[i]->name == name) {
                    setThirdLevelShortcutIndex(i);
                }
            }
            break;
        }
    }
}
