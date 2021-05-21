/*
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2020 Méven Car <meven.car@kdemail.net>
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "componentchooserbrowser.h"

#include "browser_settings.h"

ComponentChooserBrowser::ComponentChooserBrowser(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("x-scheme-handler/http"),
                       QStringLiteral("WebBrowser"),
                       QStringLiteral("org.kde.falkon.desktop"),
                       i18n("Select default browser"))
{
}

void ComponentChooserBrowser::save()
{
    const QString storageId = m_applications[m_index].toMap()["storageId"].toString();

    BrowserSettings browserSettings;
    browserSettings.setBrowserApplication(storageId);
    browserSettings.save();

    saveMimeTypeAssociation(QStringLiteral("x-scheme-handler/http"), storageId);
    saveMimeTypeAssociation(QStringLiteral("x-scheme-handler/https"), storageId);
}
