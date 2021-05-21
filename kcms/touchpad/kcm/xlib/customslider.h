/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QSlider>

class CustomSlider : public QSlider
{
    Q_OBJECT

    Q_PROPERTY(double minimum READ doubleMinimum WRITE setDoubleMinimum)
    Q_PROPERTY(double maximum READ doubleMaximum WRITE setDoubleMaximum)
    Q_PROPERTY(double value READ doubleValue WRITE setDoubleValue NOTIFY valueChanged USER true)

public:
    explicit CustomSlider(QWidget *parent = nullptr);

    void setDoubleMinimum(double);
    double doubleMinimum() const;

    void setDoubleMaximum(double);
    double doubleMaximum() const;

    class Interpolator
    {
    public:
        Interpolator()
        {
        }
        virtual double absolute(double relative, double minimum, double maximum) const;
        virtual double relative(double absolute, double minimum, double maximum) const;

        virtual ~Interpolator();
    };

    class SqrtInterpolator : public Interpolator
    {
    public:
        SqrtInterpolator()
        {
        }
        double absolute(double relative, double minimum, double maximum) const override;
        double relative(double absolute, double minimum, double maximum) const override;
    };

    const Interpolator *interpolator() const;
    void setInterpolator(const Interpolator *);

    double doubleValue() const;
    double fixup(double) const;

public Q_SLOTS:
    void setDoubleValue(double);

Q_SIGNALS:
    void valueChanged(double);

protected:
    void resizeEvent(QResizeEvent *) override;

private Q_SLOTS:
    void updateValue();

private:
    void updateRange(const QSize &);
    void moveSlider();
    int doubleToInt(double) const;
    double intToDouble(int) const;

    static const Interpolator lerp;

    double m_min, m_max, m_value;
    const Interpolator *m_interpolator;
};

#endif // CUSTOMSLIDER_H
