/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef INPUTBACKEND_H
#define INPUTBACKEND_H

#include <QObject>
#include <QVector>
#include <QVariantHash>


enum class InputBackendMode {
    KWinWayland = 0,
    XLibinput = 1,
    XEvdev = 2
};

class InputBackend : public QObject
{
    Q_OBJECT

protected:
    explicit InputBackend(QObject *parent) : QObject(parent) {}
    InputBackendMode m_mode;

public:
    static InputBackend *implementation(QObject *parent = nullptr);

    InputBackendMode mode() {
        return m_mode;
    }

    virtual void kcmInit() {}

    virtual bool isValid() const { return false; }

    virtual void load() {}

    virtual bool applyConfig(const QVariantHash &) { return false; }
    virtual bool getConfig(QVariantHash &) { return false; }

    virtual bool applyConfig() { return false; }
    virtual bool getConfig() { return false; }

    virtual bool getDefaultConfig() { return false; }
    virtual bool isChangedConfig() const { return false; }

    virtual QString errorString() const { return QString(); }

    virtual int deviceCount() const { return 0; }
    virtual QVector<QObject*> getDevices() const { return QVector<QObject*>(); }

Q_SIGNALS:
    void deviceAdded(bool success);
    void deviceRemoved(int index);
};

#endif // INPUTBACKEND_H
