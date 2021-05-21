/*
    SPDX-FileCopyrightText: 1998 Luca Montecchiani <m.luca@usa.net>
    SPDX-License-Identifier: GPL-2.0-or-later

*/
#ifndef main_included
#define main_included

#include <KCModule>

class Dtime;
class QTabWidget;

class KclockModule : public KCModule
{
    Q_OBJECT

public:
    explicit KclockModule(QWidget *parent, const QVariantList &);

    void save() override;
    void load() override;

private:
    bool kauthSave();
    bool timedatedSave();

    QTabWidget *tab;
    Dtime *dtime;

    bool m_haveTimedated = false;
};

#endif // main_included
