/*
    Copyright (c) 1998, 2008, 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KONQ_NAMEANDURLINPUTDIALOG_H
#define KONQ_NAMEANDURLINPUTDIALOG_H

#include <libkonq_export.h>
#include <kdialog.h>
#include <kurl.h>
class KLineEdit;
class KUrlRequester;
class KonqNameAndUrlInputDialogPrivate;

/**
 * Dialog to ask for a name (e.g. filename) and a URL
 * Basically a merge of KLineEditDlg and KUrlRequesterDlg ;)
 * @since 4.4
 * @author David Faure <faure@kde.org>
 *
 * @deprecated since 4.5, use KNameAndUrlInputDialog from kdelibs/kfile instead.
 */
class LIBKONQ_EXPORT_DEPRECATED KonqNameAndUrlInputDialog : public KDialog
{
    Q_OBJECT
public:
    /**
     * @param nameLabel label for the name field
     * @param urlLabel label for the URL requester
     * @param startDir start directory for the URL requester (optional)
     * @param parent parent widget
     */
    KonqNameAndUrlInputDialog(const QString& nameLabel, const QString& urlLabel, const KUrl& startDir, QWidget *parent);

    /**
     * Destructor.
     */
    virtual ~KonqNameAndUrlInputDialog();

    /**
     * Pre-fill the name lineedit.
     */
    void setSuggestedName(const QString& name);
    /**
     * Pre-fill the URL requester.
     */
    void setSuggestedUrl(const KUrl& url);

    /**
     * @return the name the user entered
     * Empty if the dialog was cancelled.
     */
    QString name() const;
    /**
     * @return the URL the user entered
     * Empty if the dialog was cancelled.
     */
    KUrl url() const;

private:
    Q_PRIVATE_SLOT(d, void _k_slotClear())
    Q_PRIVATE_SLOT(d, void _k_slotNameTextChanged(const QString&))
    Q_PRIVATE_SLOT(d, void _k_slotURLTextChanged(const QString&))

    KonqNameAndUrlInputDialogPrivate* const d;
};

#endif
