/*
 *  Copyright (C) 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H
#include <ibus.h>
#include <QByteArray>
#include <QList>
#include <QString>
#include <QMap>
#include <QQueue>
#include <QStringList>

class EngineManager {
public:
    EngineManager();
    virtual ~EngineManager();

    void setEngines(IBusEngineDesc** engines);
    IBusEngineDesc** engines() { return m_engines; }
    size_t length() { return m_length; }
    void setUseGlobalEngine(gboolean g_variant_get_boolean);
    void setCurrentContext(const gchar* input_context_path);
    QString currentEngine();
    bool useGlobalEngine() { return m_useGlobalEngine; }
    void setCurrentEngine(const char* name);
    const char* navigate(IBusEngineDesc* engine, bool forward);
    void moveToFirst(IBusEngineDesc* engine_desc);
    QStringList engineOrder();
    void setOrder(const gchar** engine_names, size_t len);
    size_t getIndexByName(const char* name);

private:
    QQueue<QString> m_history;
    QMap<QString, QString> m_engineMap;
    QString m_currentContext;
    IBusEngineDesc** m_engines;
    size_t m_length;
    bool m_useGlobalEngine;
    void freeOldEngine();
};

#endif // PROPERTYMANAGER_H
