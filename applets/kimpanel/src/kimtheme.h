#ifndef LAYOUT_KIMTHEME_H
#define LAYOUT_KIMTHEME_H

#include <plasma/theme.h>
#include <QtGui>
#include "kimsvgscriptlayout.h"

namespace KIM
{
    class ThemePrivate;

    class KDE_EXPORT Theme:public QObject
    {
        Q_OBJECT
    public:
        enum ColorRole {
            StatusbarTextColor = 1, /**<  the text color to be used by items resting on the background */
            StatusbarBackgroundColor = 2, /**< the default background color */
            AuxilaryTextColor = 4, /** text color for buttons */
            AuxilaryBackgroundColor = 8, /** background color for buttons*/
            PreeditTextColor = 16, /** text color for buttons */
            PreeditBackgroundColor = 32, /** background color for buttons*/
            LookupTableLabelTextColor = 64, /** text color for buttons */
            LookupTableLabelBackgroundColor = 128, /** background color for buttons*/
            LookupTableEntryTextColor = 256, /** text color for buttons */
            LookupTableEntryBackgroundColor = 512 /** background color for buttons*/
        };

        enum FontRole {
            DefaultFont = 0, /**< The standard text font */
            StatusbarTextFont,
            AuxilaryTextFont,
            PreeditTextFont,
            TableLabelTextFont,
            TableEntryTextFont
        };

        /**
         * Singleton pattern accessor
         **/
        static Theme *defaultTheme();

        /**
         * Default constructor. Usually you want to use the singleton instead.
         */
        explicit Theme(QObject *parent = 0);
        ~Theme();

        bool isValid() const;

        /**
         * Sets the current theme being used.
         */
        void setThemeName(const QString &themeName);

        /**
         * @return the name of the theme.
         */
        QString themeName() const;

        QStringList listThemeNames() const;

        /**
         * Retrieve the path for an SVG image in the current theme.
         *
         * @arg name the name of the file in the theme directory (without the
         *           ".svg" part or a leading slash)
         * @return the full path to the requested file for the current theme
         */
        QString statusbarImagePath() const;

        /**
         * Retrieve the path for an SVG image in the current theme.
         *
         * @arg name the name of the file in the theme directory (without the
         *           ".svg" part or a leading slash)
         * @return the full path to the requested file for the current theme
         */
        QString statusbarLayoutPath() const;

        /**
         * Retrieve the path for an SVG image in the current theme.
         *
         * @arg name the name of the file in the theme directory (without the
         *           ".svg" part or a leading slash)
         * @return the full path to the requested file for the current theme
         */
        QString lookuptableImagePath() const;

        /**
         * Retrieve the path for an SVG image in the current theme.
         *
         * @arg name the name of the file in the theme directory (without the
         *           ".svg" part or a leading slash)
         * @return the full path to the requested file for the current theme
         */
        QString lookuptableLayoutPath() const;

        /**
         * Returns the text color to be used by items resting on the background
         *
         * @arg role which role (usage pattern) to get the color for
         */
        QColor color(ColorRole role) const;

        /**
         * Returns the font to be used by themed items
         *
         * @arg role which role (usage pattern) to get the font for
         */
        QFont font(FontRole role) const;

        /**
         * Returns the font metrics for the font to be used by themed items
         */
        QFontMetrics fontMetrics(FontRole role) const;

    Q_SIGNALS:
        /**
         * Emitted when the user changes the theme. SVGs should be reloaded at
         * that point
         */
        void themeChanged();

    private:
        friend class ThemePrivate;
        ThemePrivate *const d;
        Q_PRIVATE_SLOT(d,void themeUpdated())
    };
} // namespace KIM

#endif // LAYOUT_KIMTHEME_H

