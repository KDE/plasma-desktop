/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitnode.h"

SplitMaterial::SplitMaterial()
{
    setFlag(Blending);
}

QSGMaterialType *SplitMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *SplitMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new SplitShader;
}

int SplitMaterial::compare(const QSGMaterial *o) const
{
    const SplitMaterial *other = static_cast<const SplitMaterial *>(o);

    if (!texture1 || !other->texture1) {
        return texture1 ? 1 : -1;
    }

    if (!texture2 || !other->texture2) {
        return texture2 ? 1 : -1;
    }

    qint64 diff = texture1->comparisonKey() - other->texture1->comparisonKey();
    if (diff != 0) {
        return diff < 0 ? -1 : 1;
    }

    diff = texture2->comparisonKey() - other->texture2->comparisonKey();
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
}

SplitShader::SplitShader()
{
    setShaderFileName(VertexStage, QStringLiteral(":/kcms/landingpage/shaders/split.vert.qsb"));
    setShaderFileName(FragmentStage, QStringLiteral(":/kcms/landingpage/shaders/split.frag.qsb"));
}

bool SplitShader::updateUniformData(RenderState &state, QSGMaterial *newMat, QSGMaterial *oldMat)
{
    bool changed = false;
    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= 80);

    SplitMaterial *newMaterial = static_cast<SplitMaterial *>(newMat);
    SplitMaterial *oldMaterial = static_cast<SplitMaterial *>(oldMat);

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + 64, &opacity, 4);
        changed = true;
    }

    if (!oldMaterial || oldMaterial->tilt != newMaterial->tilt) {
        memcpy(buf->data() + 72, &newMaterial->tilt, 8);
        changed = true;
    }

    return changed;
}

void SplitShader::updateSampledImage(RenderState &state, int binding, QSGTexture **texture, QSGMaterial *newMaterial, QSGMaterial *)
{
    Q_UNUSED(state);

    SplitMaterial *material = static_cast<SplitMaterial *>(newMaterial);
    switch (binding) {
    case 1:
        *texture = material->texture1;
        break;
    case 2:
        *texture = material->texture2;
        break;
    default:
        return;
    }
}

SplitNode::SplitNode()
{
    setFlag(UsePreprocess);

    m_contentNode.setGeometry(new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4));
    m_contentNode.setFlag(OwnsGeometry);

    m_material = new SplitMaterial;
    m_contentNode.setMaterial(m_material);
    m_contentNode.setFlag(OwnsMaterial);
}

void SplitNode::preprocess()
{
    if (m_first) {
        m_material->texture1 = m_first->texture();
        if (QSGDynamicTexture *dynamicTexture = qobject_cast<QSGDynamicTexture *>(m_first->texture())) {
            dynamicTexture->updateTexture();
        }
    }

    if (m_second) {
        m_material->texture2 = m_second->texture();
        if (QSGDynamicTexture *dynamicTexture = qobject_cast<QSGDynamicTexture *>(m_second->texture())) {
            dynamicTexture->updateTexture();
        }
    }

    const qreal angle = std::atan(m_shutter);
    m_material->tilt = QVector2D(std::cos(angle), std::sin(angle));

    if (m_contentNode.parent() && !(m_material->texture1 && m_material->texture2)) {
        removeChildNode(&m_contentNode);
    } else if (!m_contentNode.parent() && (m_material->texture1 && m_material->texture2)) {
        appendChildNode(&m_contentNode);
    }
}

void SplitNode::setFirst(QSGTextureProvider *provider)
{
    if (m_first == provider) {
        return;
    }

    if (m_first) {
        disconnect(m_first, &QSGTextureProvider::textureChanged, this, &SplitNode::markMaterialDirty);
    }

    m_first = provider;
    if (m_first) {
        connect(m_first, &QSGTextureProvider::textureChanged, this, &SplitNode::markMaterialDirty, Qt::DirectConnection);
    }
}

void SplitNode::setSecond(QSGTextureProvider *provider)
{
    if (m_second == provider) {
        return;
    }

    if (m_second) {
        disconnect(m_second, &QSGTextureProvider::textureChanged, this, &SplitNode::markMaterialDirty);
    }

    m_second = provider;
    if (m_second) {
        connect(m_second, &QSGTextureProvider::textureChanged, this, &SplitNode::markMaterialDirty, Qt::DirectConnection);
    }
}

void SplitNode::setRect(const QRectF &rect)
{
    if (m_rect != rect) {
        m_rect = rect;
        QSGGeometry::updateTexturedRectGeometry(m_contentNode.geometry(), m_rect, QRectF(0, 0, 1, 1));
        markDirty(DirtyGeometry);
    }
}

void SplitNode::setShutter(qreal shutter)
{
    if (m_shutter != shutter) {
        m_shutter = shutter;
        markDirty(DirtyMaterial);
    }
}

void SplitNode::markMaterialDirty()
{
    markDirty(DirtyMaterial);
}

#include "moc_splitnode.cpp"
