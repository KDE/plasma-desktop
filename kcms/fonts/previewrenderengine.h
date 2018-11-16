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

#ifndef __PREVIEW_RENDER_ENGINE_H
#define __PREVIEW_RENDER_ENGINE_H

#include "kxftconfig.h"
#include "FcEngine.h"

#ifdef HAVE_FONTCONFIG

#include <QImage>

class PreviewRenderEngine : public KFI::CFcEngine
{
public: 
    PreviewRenderEngine(bool init=true);
    ~PreviewRenderEngine() override;
    
    QImage drawAutoSize(const QFont &font, const QColor &txt, const QColor &bgnd, const QString &text);
};

#endif // HAVE_FONTCONFIG

#endif // __PREVIEW_RENDER_ENGINE_H
