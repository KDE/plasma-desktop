/*
   Copyright (c) 2018 Julian Wolff <wolff@julianwolff.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QApplication>
#include <QPainter>
#include <QPalette>

#include <KWindowSystem>

#include "kxftconfig.h"
#include "previewimageprovider.h"
#include "previewrenderengine.h"

QImage combineImages(const QList<QImage>& images, const QColor& bgnd, int spacing=0)
{
    int width = 0;
    int height = 0;
    QImage::Format format = QImage::Format_Invalid;
    int devicePixelRatio = 1;
    for(const auto& image : images) {
        if(width < image.width()) {
            width = image.width();
        }
        height += image.height() + spacing;
        format = image.format();
        devicePixelRatio = image.devicePixelRatio();
    }
    height -= spacing;
    
    // To correctly align the image pixels on a high dpi display, 
    // the image dimensions need to be a multiple of devicePixelRatio
    width = (width + devicePixelRatio - 1) / devicePixelRatio * devicePixelRatio;
    height = (height + devicePixelRatio - 1) / devicePixelRatio * devicePixelRatio;
    
    QImage combinedImage(width, height, format);
    combinedImage.setDevicePixelRatio(devicePixelRatio);
    combinedImage.fill(bgnd);
    
    int offset = 0;
    QPainter p(&combinedImage);
    for(const auto& image : images) {
        p.drawImage(0, offset, image);
        offset += (image.height() + spacing) / devicePixelRatio;
    }
    
    return combinedImage;
}

PreviewImageProvider::PreviewImageProvider(const QFont& font)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_font(font)
{
}

QImage PreviewImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize)
    if (!KWindowSystem::isPlatformX11()) {
        return QImage();
    }

    int subPixelIndex = 0;
    int hintingIndex = 0;
    
    const auto idpart = id.splitRef(QLatin1Char('.'))[0];
    const auto sections = idpart.split(QLatin1Char('_'));

    if (sections.size() >= 2) {
        subPixelIndex = sections[0].toInt() + KXftConfig::SubPixel::None;
        hintingIndex = sections[1].toInt() + KXftConfig::Hint::None;
    } else {
        return QImage();
    }

    KXftConfig xft;
    

    KXftConfig::AntiAliasing::State oldAntialiasing = xft.getAntiAliasing();
    double oldStart = 0;
    double oldEnd = 0;
    xft.getExcludeRange(oldStart, oldEnd);
    KXftConfig::SubPixel::Type oldSubPixelType = KXftConfig::SubPixel::NotSet;
    xft.getSubPixelType(oldSubPixelType);
    KXftConfig::Hint::Style oldHintStyle = KXftConfig::Hint::NotSet;
    xft.getHintStyle(oldHintStyle);
    
    
    xft.setAntiAliasing(KXftConfig::AntiAliasing::Enabled);
    xft.setExcludeRange(0, 0);
    
    KXftConfig::SubPixel::Type subPixelType = (KXftConfig::SubPixel::Type)subPixelIndex;
    xft.setSubPixelType(subPixelType);
    
    KXftConfig::Hint::Style hintStyle = (KXftConfig::Hint::Style)hintingIndex;
    xft.setHintStyle(hintStyle);
    
    xft.apply();
    
    
    QColor text(QApplication::palette().color(QPalette::Text));
    QColor bgnd(QApplication::palette().color(QPalette::Window));
    
    PreviewRenderEngine eng(true);
    QList<QImage> lines;
    
    lines << eng.drawAutoSize(m_font, text, bgnd, eng.getDefaultPreviewString());
    
    QImage img = combineImages(lines, bgnd, lines[0].height()*.25);
    
    
    xft.setAntiAliasing(oldAntialiasing);
    xft.setExcludeRange(oldStart, oldEnd);
    xft.setSubPixelType(oldSubPixelType);
    xft.setHintStyle(oldHintStyle);
    
    xft.apply();
    
    *size = img.size();
    
    return img;
}
