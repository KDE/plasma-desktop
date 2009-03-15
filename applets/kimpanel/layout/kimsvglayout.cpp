#include "kimsvglayout.h"
#include <KGlobalSettings>
#include <QtCore>
#include <QtGui>

namespace KIM
{
    SvgLayout::SvgLayout(QObject *parent)
        :QObject(parent),
         m_renderer(new KSvgRenderer())
    {

    }

    SvgLayout::~SvgLayout()
    {

    }

    void SvgLayout::setImagePath(const QString &path)
    {
        bool isThemed = !QDir::isAbsolutePath(path);

        // lets check to see if we're already set to this file
        if (isThemed == m_themed &&
            ((m_themed && m_themePath == path) ||
             (!m_themed && m_path == path))) {
            return;
        }

        // if we don't have any path right now and are going to set one,
        // then lets not schedule a repaint because we are just initializing!
        if (m_themed) {
            QObject::disconnect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
                                this, SLOT(themeChanged()));
//X             QObject::disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
//X                                 this, SLOT(colorsChanged()));
        }

        m_themed = isThemed;
        m_path.clear();
        m_themePath.clear();

        if (m_themed) {
            m_themePath = path;
            m_path = Plasma::Theme::defaultTheme()->imagePath(m_themePath);
            m_renderer->load(m_path);
            QObject::connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
                             this, SLOT(themeChanged()));
        } else if (QFile::exists(path)) {
            m_path = path;
            m_renderer->load(m_path);
        } else {
            kDebug() << "file '" << path << "' does not exist!";
        }
    }

    QRectF SvgLayout::elementRect(const QString &elem) const
    {
        return QRectF();
    }
} // namespace KIM

#include "kimsvglayout.moc"
// vim: sw=4 sts=4 et tw=100
