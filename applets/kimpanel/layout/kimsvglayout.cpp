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
        m_path.clear();

        if (QFile::exists(path)) {
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

    void SvgLayout::themeUpdated()
    {
    }
} // namespace KIM

#include "kimsvglayout.moc"
// vim: sw=4 sts=4 et tw=100
