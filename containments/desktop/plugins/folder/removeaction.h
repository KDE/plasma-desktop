/*
    SPDX-FileCopyrightText: 2021 Derek Christ <christ.derek@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOVEACTION_H
#define REMOVEACTION_H

#include <KActionCollection>
#include <QAction>
#include <QPointer>

/**
 * A QAction that manages the delete based on the current state of
 * the Shift key or the parameter passed to update.
 *
 * This class expects the presence of both the "del" and
 * "trash" actions in @ref collection.
 */
class RemoveAction : public QAction
{
    Q_OBJECT

public:
    explicit RemoveAction(KActionCollection *collection, QObject *parent = nullptr);

    enum class ShiftState { Unknown, Pressed, Released };

    /**
     * Updates this action key based on @p shiftState.
     * Default value is Unknown, meaning it will query QGuiApplication::modifiers().
     */
    void update(ShiftState shiftState = ShiftState::Unknown);

    /**
     * Returns the current action that RemoveAction performs.
     */
    const QAction *proxyAction() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPointer<KActionCollection> m_collection;
    QPointer<QAction> m_action;
};

#endif // REMOVEACTION_H
