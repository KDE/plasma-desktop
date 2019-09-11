/*
 * KCMStyle's container dialog for custom style setup dialogs
 *
 * (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef STYLE_CONF_DIALOG
#define STYLE_CONF_DIALOG

#include <QDialog>
class QHBoxLayout;

class StyleConfigDialog: public QDialog
{
  Q_OBJECT
public:
  StyleConfigDialog(QWidget* parent, const QString &styleName);

  bool isDirty() const;

  void setMainWidget(QWidget *w);
public Q_SLOTS:
  void setDirty(bool dirty);

Q_SIGNALS:
  void defaults();
  void save();

private:
  void slotAccept();
  bool m_dirty;
  QHBoxLayout *mMainLayout = nullptr;
};

#endif
