/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz                                      *
 *   peter.penz@gmx.at                                                     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#ifndef KONQ_STATUSBARMESSAGELABEL_H
#define KONQ_STATUSBARMESSAGELABEL_H

#include "libkonq_export.h"
#include <QWidget>

class QPaintEvent;
class QResizeEvent;

/**
 * @brief Represents a message text label as part of the status bar.
 *
 * Dependent from the given type automatically a corresponding icon
 * is shown in front of the text. For message texts having the type
 * KonqStatusBarMessageLabel::Error a dynamic color blending is done to get the
 * attention from the user.
 */
class LIBKONQ_EXPORT KonqStatusBarMessageLabel : public QWidget
{
    Q_OBJECT

public:
    explicit KonqStatusBarMessageLabel(QWidget* parent);
    virtual ~KonqStatusBarMessageLabel();

    /**
     * Describes the type of the message text. Dependent
     * from the type a corresponding icon and color is
     * used for the message text.
     */
    enum Type {
        Default,
        OperationCompleted,
        Information,
        Error
    };

    void setMessage(const QString& text, Type type);

    Type type() const;

    QString text() const;

    void setDefaultText(const QString& text);
    QString defaultText() const;

    // TODO: maybe a better approach is possible with the size hint
    void setMinimumTextHeight(int min);
    int minimumTextHeight() const;

    /** @see QWidget::sizeHint */
    virtual QSize sizeHint() const;
    /** @see QWidget::minimumSizeHint */
    virtual QSize minimumSizeHint() const;

protected:
    /** @see QWidget::paintEvent() */
    virtual void paintEvent(QPaintEvent* event);

    /** @see QWidget::resizeEvent() */
    virtual void resizeEvent(QResizeEvent* event);

private Q_SLOTS:
    void timerDone();

    /**
     * Increases the height of the message label so that
     * the given text fits into given area.
     */
    void assureVisibleText();

    /**
     * Returns the available width in pixels for the text.
     */
    int availableTextWidth() const;

    /**
     * Moves the close button to the upper right corner
     * of the message label.
     */
    void updateCloseButtonPosition();

    /**
     * Closes the currently shown error message and replaces it
     * by the next pending message.
     */
    void closeErrorMessage();

private:
    /**
     * Shows the next pending error message. If no pending message
     * was in the queue, false is returned.
     */
    bool showPendingMessage();

    /**
     * Resets the message label properties. This is useful when the
     * result of invoking KonqStatusBarMessageLabel::setMessage() should
     * not rely on previous states.
     */
    void reset();

private:
    enum State
    {
        DefaultState,
        Illuminate,
        Illuminated,
        Desaturate
    };

    class Private;
    Private* const d;
};

#endif
