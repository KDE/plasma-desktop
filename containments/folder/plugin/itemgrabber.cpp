/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                   *
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

#include "itemgrabber.h"

#include <QQuickItem>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSGTextureProvider>
#include <QOpenGLFunctions>

ItemGrabber::ItemGrabber(QObject *parent) : QObject(parent)
{
}

ItemGrabber::~ItemGrabber()
{
}

QQuickItem *ItemGrabber::item() const
{
    return m_item;
}

void ItemGrabber::setItem(QQuickItem *item)
{
    if (m_item != item) {
        m_item = item;

        emit itemChanged();
    }

    if (m_item && m_item->window()) {
        connect(m_item->window(), SIGNAL(afterRendering()), this, SLOT(grab()), Qt::DirectConnection);
        item->update();
    } else {
        m_image = QImage();
        emit nullChanged();
    }
}

bool ItemGrabber::null() const
{
    return m_image.isNull();
}

QImage ItemGrabber::image() const
{
    return m_image;
}

void ItemGrabber::grab()
{
    if (!m_item || !m_item->window()) {
        return;
    }

    QSGTextureProvider *provider = m_item->textureProvider();

    if (!provider) {
        return;
    }

    QSGTexture *texture = provider->texture();

    if (!texture) {
        return;
    }

    QOpenGLFunctions *f = m_item->window()->openglContext()->functions();

    m_image = QImage(m_item->width(), m_item->height(), QImage::Format_RGBA8888_Premultiplied);
    m_image.fill(Qt::transparent);

    GLint prevfbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevfbo);

    GLuint fbo;
    f->glGenFramebuffers(1, &fbo);
    f->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->textureId(), 0);
    glReadPixels(0, 0, m_image.width(), m_image.height(), GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
    f->glDeleteFramebuffers(1, &fbo);

    f->glBindFramebuffer(GL_FRAMEBUFFER, prevfbo);

    emit nullChanged();
    emit imageChanged();

    if (m_item && m_item->window()) {
        disconnect(m_item->window(), SIGNAL(afterRendering()), this, SLOT(grab()));
    }
}
