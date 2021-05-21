/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGCONTAINER_H
#define CONFIGCONTAINER_H

#include <KCModule>

class ConfigPlugin;

class ConfigContainer : public KCModule
{
    Q_OBJECT

public:
    explicit ConfigContainer(QWidget *parent, const QVariantList &args = QVariantList());

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent *event) override;

    void load() override;
    void save() override;
    void defaults() override;

    void kcmLoad()
    {
        KCModule::load();
    }
    void kcmSave()
    {
        KCModule::save();
    }
    void kcmDefaults()
    {
        KCModule::defaults();
    }

protected:
    void hideEvent(QHideEvent *) override;

private:
    ConfigPlugin *m_plugin = nullptr;
};

#endif // CONFIGCONTAINER_H
