/*
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

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

#ifndef COMPONENTCHOOSERDATA_H
#define COMPONENTCHOOSERDATA_H

#include <QObject>

#include <KCModuleData>

class ComponentChooser;

class ComponentChooserData : public KCModuleData
{
public:
    ComponentChooserData(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    void load();
    void save();
    void defaults();

    bool isDefaults() const override;
    bool isSaveNeeded() const;

    ComponentChooser *browsers() const;
    ComponentChooser *fileManagers() const;
    ComponentChooser *terminalEmulators() const;
    ComponentChooser *emailClients() const;

private:
    ComponentChooser *m_browsers;
    ComponentChooser *m_fileManagers;
    ComponentChooser *m_terminalEmulators;
    ComponentChooser *m_emailClients;
};

#endif // COMPONENTCHOOSERDATA_H
