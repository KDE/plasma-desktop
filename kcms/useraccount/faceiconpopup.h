/**
 *  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef __FACEICONPOPUP_H__
#define __FACEICONPOPUP_H__

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QGridLayout>

const int faceIconSize = 46;

class FaceIconLabel;

class FaceIconPopup : public QWidget 
{
    Q_OBJECT

public:
    explicit FaceIconPopup(QWidget *parent = NULL, 
                           Qt::WindowFlags f = Qt::Popup);
    ~FaceIconPopup();

    void popup(QPoint pos);

    static QPixmap facePixmap(const QString faceIconPath);
    static QIcon faceIcon(const QString faceIconPath);

Q_SIGNALS:
    void clickFaceIcon(QString filePath);

private slots:
    void slotPressed(QString filePath);

private:
    QGridLayout *m_layout;

    FaceIconLabel *m_createFaceIconLabel(QString filePath);
};

class FaceIconLabel : public QLabel 
{
    Q_OBJECT

public:
    explicit FaceIconLabel(QString filePath, QWidget *parent = NULL);

Q_SIGNALS:
    void pressed(QString filePath);

protected:
    void mousePressEvent(QMouseEvent *);

private:
    QString m_filePath;
};

#endif /* __FACEICONPOPUP_H__ */
