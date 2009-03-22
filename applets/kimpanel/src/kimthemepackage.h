#ifndef KIMTHEMEPACKAGE_H
#define KIMTHEMEPACKAGE_H

#include <plasma/packagestructure.h>
#include <QWidget>

#include "kimpanelruntime_export.h"

namespace KIM
{

class KIMPANELRUNTIME_EXPORT ThemePackage : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    explicit ThemePackage(QObject *parent = 0);

    bool installTheme(const QString &package);
};

} // namespace KIM

#endif // KIMTHEMEPACKAGE_H
