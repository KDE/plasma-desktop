/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>
#include <KActionCollection>

#include "layout_list_models.h"

class LayoutConfigModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool showLayoutIndicator READ showLayoutIndicator WRITE setShowLayoutIndicator NOTIFY showLayoutIndicatorChanged)
    Q_PROPERTY(bool showForSingleLayout READ showForSingleLayout WRITE setShowForSingleLayout NOTIFY showForSingleLayoutChanged)
    Q_PROPERTY(bool layoutIndicatorShowFlag READ layoutIndicatorShowFlag WRITE setLayoutIndicatorShowFlag NOTIFY layoutIndicatorShowFlagChanged)
    Q_PROPERTY(bool layoutIndicatorShowLabel READ layoutIndicatorShowLabel WRITE setLayoutIndicatorShowLabel NOTIFY layoutIndicatorShowLabelChanged)
    Q_PROPERTY(int switchingPolicyIndex READ switchingPolicyIndex WRITE setSwitchingPolicyIndex NOTIFY switchingPolicyIndexChanged)
    Q_PROPERTY(QString alternativeShortcut READ alternativeShortcut WRITE setAlternativeShortcut NOTIFY alternativeShortcutChanged)
    Q_PROPERTY(LayoutListFilterDuplicatesProxyModel *layoutListModel READ layoutListModel NOTIFY layoutListModelChanged)
    Q_PROPERTY(LayoutListSortByPriorityProxyModel *currentLayoutListModel READ currentLayoutListModel NOTIFY currentLayoutListModelChanged)
    Q_PROPERTY(QStringList mainShiftKeyModel READ mainShiftKeyModel)
    Q_PROPERTY(int mainShiftKeyIndex READ mainShiftKeyIndex WRITE setMainShiftKeyIndex NOTIFY mainShiftKeyIndexChanged)
    Q_PROPERTY(QStringList thirdLevelShortcutModel READ thirdLevelShortcutModel)
    Q_PROPERTY(int thirdLevelShortcutIndex READ thirdLevelShortcutIndex WRITE setThirdLevelShortcutIndex NOTIFY thirdLevelShortcutChanged)

public:
    explicit LayoutConfigModel(QObject *parent);

public:
    bool showLayoutIndicator() const;
    void setShowLayoutIndicator(bool showLayoutIndicator);

    bool showForSingleLayout() const;
    void setShowForSingleLayout(bool showForSingleLayout);

    bool layoutIndicatorShowFlag() const;
    void setLayoutIndicatorShowFlag(bool b);

    bool layoutIndicatorShowLabel() const;
    void setLayoutIndicatorShowLabel(bool b);

    int switchingPolicyIndex() const;
    void setSwitchingPolicyIndex(int switchingPolicyIndex);

    QString alternativeShortcut() const;
    void setAlternativeShortcut(QString alternativeShortcut);

    LayoutListFilterDuplicatesProxyModel* layoutListModel() const;
    LayoutListSortByPriorityProxyModel* currentLayoutListModel() const;

    void loadEnabledLayouts();
    void saveEnabledLayouts();

    QStringList mainShiftKeyModel() const;
    QStringList thirdLevelShortcutModel() const;

    static const char* switch_modes[4];

    int mainShiftKeyIndex() const;
    QString mainShiftKey() const;
    void setMainShiftKeyIndex(int mainShiftKeyIndex);
    void setMainShiftKey(QString const& name);

    int thirdLevelShortcutIndex() const;
    QString thirdLevelShortcut() const;
    void setThirdLevelShortcutIndex(int thirdLevelShortcut);
    void setThirdLevelShortcut(QString const& name);

Q_SIGNALS:
    void showLayoutIndicatorChanged();
    void showForSingleLayoutChanged();
    void layoutIndicatorShowFlagChanged();
    void layoutIndicatorShowLabelChanged();
    void switchingPolicyIndexChanged();
    void alternativeShortcutChanged();
    void layoutListModelChanged();
    void currentLayoutListModelChanged();
    void mainShiftKeyIndexChanged();
    void thirdLevelShortcutChanged();

private:
    bool m_showLayoutIndicator;
    bool m_showForSingleLayout;

    bool m_layoutIndicatorShowFlag;
    bool m_layoutIndicatorShowLabel;

    int m_switchingPolicyIndex;
    int m_mainShiftKeyIndex;
    int m_thirdLevelShortcutIndex;

    QString m_alternativeShortcuts;

    LayoutListModels *m_layoutListModels;
};
