/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QQueue>
#include <QString>
#include <QStringList>
#include <ibus.h>

class EngineManager
{
public:
    EngineManager();
    virtual ~EngineManager();

    void setEngines(IBusEngineDesc **engines);
    IBusEngineDesc **engines()
    {
        return m_engines;
    }
    size_t length()
    {
        return m_length;
    }
    void setUseGlobalEngine(gboolean g_variant_get_boolean);
    void setCurrentContext(const gchar *input_context_path);
    QString currentEngine();
    bool useGlobalEngine()
    {
        return m_useGlobalEngine;
    }
    void setCurrentEngine(const char *name);
    const char *navigate(IBusEngineDesc *engine, bool forward);
    void moveToFirst(IBusEngineDesc *engine_desc);
    QStringList engineOrder();
    void setOrder(const gchar **engine_names, size_t len);
    size_t getIndexByName(const char *name);

private:
    QQueue<QString> m_history;
    QMap<QString, QString> m_engineMap;
    QString m_currentContext;
    IBusEngineDesc **m_engines;
    size_t m_length;
    bool m_useGlobalEngine;
    void freeOldEngine();
};

#endif // PROPERTYMANAGER_H
