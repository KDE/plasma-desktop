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

#include "faceiconpopup.h"

#include <QDir>
#include <QFileInfo>

static const unsigned int columnCount = 5;

FaceIconPopup::FaceIconPopup(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_layout(NULL)
{
    setFixedSize(280, 320);

    m_layout = new QGridLayout;
    setLayout(m_layout);

    QDir dir("/usr/share/pixmaps/faces");
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();

    dir.setPath("/usr/share/kdm/pics/users");
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    list << dir.entryInfoList();

    for (int i = 0; i < list.size() / columnCount; i++) {
        QFileInfo fileInfo = list.at(i);
        for (int j = 0; j < columnCount; j++) {
            QFileInfo fileInfo = list.at(i * columnCount + j);
            m_layout->addWidget(
                m_createFaceIconLabel(fileInfo.absoluteFilePath()),
                i, j);
        }
    }
}

FaceIconPopup::~FaceIconPopup() 
{
    if (m_layout) {
        delete m_layout;
        m_layout = NULL;
    }
}

void FaceIconPopup::popup(QPoint pos) 
{
    move(pos);
    show();
}

QPixmap FaceIconPopup::facePixmap(const QString faceIconPath) 
{
    if (!QFile::exists(faceIconPath)) {
        QPixmap pixmap(faceIconSize, faceIconSize);
        pixmap.fill();
        return pixmap;
    }

    return QPixmap(faceIconPath);
}

QIcon FaceIconPopup::faceIcon(const QString faceIconPath)
{                                                                                  
    return QIcon(facePixmap(faceIconPath));
}

void FaceIconPopup::slotPressed(QString filePath)
{
    emit clickFaceIcon(filePath);
    close();
}

FaceIconLabel *FaceIconPopup::m_createFaceIconLabel(QString filePath)
{
    FaceIconLabel *label = new FaceIconLabel(filePath);
    connect(label, SIGNAL(pressed(QString)), this, SLOT(slotPressed(QString)));
    return label;
}

FaceIconLabel::FaceIconLabel(QString filePath, QWidget *parent)
  : QLabel(parent),
    m_filePath(filePath)
{
    setPixmap(FaceIconPopup::facePixmap(m_filePath).scaled(faceIconSize, faceIconSize));
    setMaximumWidth(faceIconSize);
    setMaximumHeight(faceIconSize);
}

void FaceIconLabel::mousePressEvent(QMouseEvent *) 
{
    emit pressed(m_filePath);
}
