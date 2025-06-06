/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QSGMaterial>
#include <QSGNode>
#include <QSGTextureProvider>

class SplitMaterial : public QSGMaterial
{
public:
    SplitMaterial();

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;
    int compare(const QSGMaterial *other) const override;

    QSGTexture *texture1 = nullptr;
    QSGTexture *texture2 = nullptr;
    QVector2D tilt;
};

class SplitShader : public QSGMaterialShader
{
public:
    SplitShader();

    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
};

class SplitNode : public QObject, public QSGNode
{
    Q_OBJECT

public:
    explicit SplitNode();

    void preprocess() override;

    void setFirst(QSGTextureProvider *provider);
    void setSecond(QSGTextureProvider *provider);
    void setRect(const QRectF &rect);
    void setShutter(qreal shutter);

private:
    void markMaterialDirty();

    QPointer<QSGTextureProvider> m_first;
    QPointer<QSGTextureProvider> m_second;
    QRectF m_rect;
    qreal m_shutter = 0.0;
    QSGGeometryNode m_contentNode;
    SplitMaterial *m_material = nullptr;
};

class SoftwareSplitNode : public QObject, public QSGNode
{
    Q_OBJECT

public:
    explicit SoftwareSplitNode();

    void preprocess() override;

    void setFirst(QSGTextureProvider *provider);
    void setSecond(QSGTextureProvider *provider);
    void setRect(const QRectF &rect);
    void setWindow(QQuickWindow *window);

private:
    void markMaterialDirty();

    QQuickWindow *m_window = nullptr;
    QPointer<QSGTextureProvider> m_first;
    QSGImageNode *m_firstNode = nullptr;
    QPointer<QSGTextureProvider> m_second;
    QSGImageNode *m_secondNode = nullptr;
    QRectF m_rect;
};
