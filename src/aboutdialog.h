/* Copyright (C) 2004-2021 Robert Griebl. All rights reserved.
**
** This file is part of BrickStore.
**
** This file may be distributed and/or modified under the terms of the GNU
** General Public License version 2 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://fsf.org/licensing/licenses/gpl.html for GPL licensing information.
*/
#pragma once

#include <QDialog>
#include <QMap>

#include "ui_aboutdialog.h"


class AboutDialog : public QDialog, private Ui::AboutDialog
{
    Q_OBJECT
public:
    AboutDialog(QWidget *parent = nullptr);

protected:
    void reject() override;
    void closeEvent(QCloseEvent *) override;
    void changeEvent(QEvent *e) override;

private slots:
    void enableOk();
    void gotoPage(const QString &url);

private:
    QMap<QString, QString> m_pages;
};
