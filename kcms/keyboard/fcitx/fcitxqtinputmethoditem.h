/*
 * SPDX-FileCopyrightText: (C) 2011~2012 by CSSlayer <https://www.csslayer.info/wordpress/author/csslayer/>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

// Qt
#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtDBus/QDBusArgument>


class FcitxQtInputMethodItem
{
public:
    const QString& name() const;
    const QString& uniqueName() const;
    const QString& langCode() const;
    bool enabled() const;

    void setName(const QString& name);
    void setUniqueName(const QString& name);
    void setLangCode(const QString& name);
    void setEnabled(bool name);
    static void registerMetaType();

    static QString saveName(QString uniqueName);

    inline bool operator < (const FcitxQtInputMethodItem& im) const {
        if (m_enabled == true && im.m_enabled == false)
            return true;
        return false;
    }
private:
    QString m_name;
    QString m_uniqueName;
    QString m_langCode;
    bool m_enabled;
};

typedef QList<FcitxQtInputMethodItem> FcitxQtInputMethodItemList;

QDBusArgument& operator<<(QDBusArgument& argument, const FcitxQtInputMethodItem& im);
const QDBusArgument& operator>>(const QDBusArgument& argument, FcitxQtInputMethodItem& im);

Q_DECLARE_METATYPE(FcitxQtInputMethodItem)
Q_DECLARE_METATYPE(FcitxQtInputMethodItemList)
