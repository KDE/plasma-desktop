/*
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

#ifndef CONFIGCONTAINER_H
#define CONFIGCONTAINER_H

#include <KCModule>

class ConfigPlugin;

class ConfigContainer : public KCModule
{
    Q_OBJECT

public:
    explicit ConfigContainer(QWidget *parent,
                            const QVariantList &args = QVariantList());

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent *event) override;

    void load() override;
    void save() override;
    void defaults() override;

    void kcmLoad() { KCModule::load(); }
    void kcmSave() { KCModule::save(); }
    void kcmDefaults() { KCModule::defaults(); }

protected:
    void hideEvent(QHideEvent *) override;

private:
    ConfigPlugin* m_plugin = nullptr;
};

#endif // CONFIGCONTAINER_H
