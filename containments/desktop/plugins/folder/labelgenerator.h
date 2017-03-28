/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef LABELGENERATOR_H
#define LABELGENERATOR_H

#include <QObject>
#include <QPointer>

class KFilePlacesModel;
class FolderModel;

class LabelGenerator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(FolderModel *folderModel READ folderModel WRITE setFolderModel NOTIFY folderModelChanged)
    Q_PROPERTY(bool rtl READ rtl WRITE setRtl NOTIFY rtlChanged)
    Q_PROPERTY(int labelMode READ labelMode WRITE setLabelMode NOTIFY labelModeChanged)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText NOTIFY labelTextChanged)
    Q_PROPERTY(QString displayLabel READ displayLabel NOTIFY displayLabelChanged)

    public:
        LabelGenerator(QObject *parent = 0);
        ~LabelGenerator();

        FolderModel *folderModel() const;
        void setFolderModel(FolderModel *folderModel);

        bool rtl() const;
        void setRtl(bool rtl);

        int labelMode() const;
        void setLabelMode(int mode);

        QString labelText() const;
        void setLabelText(const QString &text);

        QString displayLabel();

    Q_SIGNALS:
        void folderModelChanged();
        void rtlChanged();
        void labelModeChanged();
        void labelTextChanged();
        void displayLabelChanged();

    private:
        KFilePlacesModel* m_placesModel;
        QPointer<FolderModel> m_folderModel;
        bool m_rtl;
        int m_labelMode;
        QString m_labelText;
};

#endif
