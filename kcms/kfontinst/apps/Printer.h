/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2011 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __PRINTER_H__
#define __PRINTER_H__

#include <QDialog>
#include <QThread>
#include <QList>
#include <QFont>
#include "Misc.h"

class QLabel;
class QProgressBar;
class QPrinter;

namespace KFI
{

class CActionLabel;

class CPrintThread : public QThread
{
    Q_OBJECT

    public:
        
    CPrintThread(QPrinter *printer, const QList<Misc::TFont> &items, int size, QObject *parent);
    ~CPrintThread() override;
   
    void run() override;

    Q_SIGNALS:
        
    void progress(int p, const QString &f);

    public Q_SLOTS:
    
    void cancel();

    private:

    QPrinter           *itsPrinter;
    QList<Misc::TFont> itsItems;
    int                itsSize;
    bool               itsCancelled;
};

class CPrinter : public QDialog
{
    Q_OBJECT

    public:

    explicit CPrinter(QWidget *parent);
    ~CPrinter() override;

    void print(const QList<Misc::TFont> &items, int size);

    Q_SIGNALS:
        
    void cancelled();

    public Q_SLOTS:
        
    void progress(int p, const QString &label);
    void slotCancelClicked();

    private:
    
    void closeEvent(QCloseEvent *e) override;

    private:

    QLabel         *itsStatusLabel;
    QProgressBar   *itsProgress;
    CActionLabel   *itsActionLabel;
};

}

#endif
