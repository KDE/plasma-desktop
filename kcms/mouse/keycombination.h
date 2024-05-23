/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <QKeySequence>
#include <QObject>

class KeyCombination : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString display READ display NOTIFY keySequenceChanged)
    Q_PROPERTY(QKeySequence keySequence READ keySequence WRITE setKeySequence NOTIFY keySequenceChanged)
    Q_PROPERTY(bool shift READ shift WRITE setShift NOTIFY keySequenceChanged)
    Q_PROPERTY(bool control READ control WRITE setControl NOTIFY keySequenceChanged)
    Q_PROPERTY(bool alt READ alt WRITE setAlt NOTIFY keySequenceChanged)
    Q_PROPERTY(bool meta READ meta WRITE setMeta NOTIFY keySequenceChanged)
    Q_PROPERTY(Qt::Key key READ key WRITE setKey NOTIFY keySequenceChanged)
public:
    explicit KeyCombination(QObject *parent = nullptr);
    virtual ~KeyCombination() override;

    /* Basically, an observable toString(). */
    QString display() const;

    QKeySequence keySequence() const;
    void setKeySequence(const QKeySequence &sequence);

    bool shift() const;
    void setShift(bool on);

    bool control() const;
    void setControl(bool on);

    bool alt() const;
    void setAlt(bool on);

    bool meta() const;
    void setMeta(bool on);

    Qt::Key key() const;
    void setKey(Qt::Key key);

Q_SIGNALS:
    /** This signal is emitted whenever the key combination changes in any way. */
    void keySequenceChanged();

    /** This signal is only emitted when individual properties (parts) of the key combination are modified through setters. */
    void keySequenceModified();

private:
    QKeySequence m_keySequence;

    void setKeyboardModifier(Qt::KeyboardModifier keyboardModifier, bool on);
};
