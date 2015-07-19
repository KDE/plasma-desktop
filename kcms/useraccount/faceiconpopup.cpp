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

#include <QGridLayout>
#include <QDir>
#include <QFileInfo>

static const unsigned int columnCount = 5;

FaceIconPopup::FaceIconPopup(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
    setFixedSize(400, 300);

    QGridLayout *layout = new QGridLayout;
    setLayout(layout);

    QDir dir("/usr/share/pixmaps/faces");
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size() / columnCount; i++) {
        QFileInfo fileInfo = list.at(i);
        for (int j = 0; j < columnCount; j++) {
            QFileInfo fileInfo = list.at(i * columnCount + j);
            layout->addWidget(m_createPixmapButton(fileInfo.absoluteFilePath()), 
                              i, j);
        }
    }
}

FaceIconPopup::~FaceIconPopup() 
{
}

void FaceIconPopup::popup(QPoint pos) 
{
    move(pos);
    show();
}

QIcon FaceIconPopup::faceIcon(QString faceIconPath)                          
{                                                                                  
    if (faceIconPath == "") {                                                      
        QPixmap pixmap(faceIconSize, faceIconSize);                                
        pixmap.fill();                                                             
        return QIcon(pixmap);                                                             
    }                                                                              
                                                                                   
    return QIcon(QPixmap(faceIconPath));                                                  
}

void FaceIconPopup::slotButtonClicked(QString filePath) 
{
    emit clickFaceIcon(filePath);
    close();
}

QPushButton *FaceIconPopup::m_createPixmapButton(QString filePath)
{
    QPushButton *button = new QPushButton;
    button->setIcon(faceIcon(filePath));
    button->setMinimumWidth(faceIconSize);
    button->setMinimumHeight(faceIconSize);
    button->setIconSize(QSize(faceIconSize, faceIconSize));
    connect(button, SIGNAL(clicked()), this, SLOT(slotButtonClicked(filePath)));
    return button;                                                                  
}
