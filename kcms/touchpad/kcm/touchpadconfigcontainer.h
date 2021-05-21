/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TOUCHPADCONFIGCONTAINER_H
#define TOUCHPADCONFIGCONTAINER_H

#include <KCModule>

class TouchpadConfigPlugin;
class TouchpadConfigLibinput;
class TouchpadConfigXlib;

class TouchpadConfigContainer : public KCModule
{
    Q_OBJECT

    friend TouchpadConfigXlib;
    friend TouchpadConfigLibinput;

public:
    explicit TouchpadConfigContainer(QWidget *parent, const QVariantList &args = QVariantList());

    static void kcmInit();

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
    TouchpadConfigPlugin *m_plugin = nullptr;
};

#endif // TOUCHPADCONFIGCONTAINER_H
