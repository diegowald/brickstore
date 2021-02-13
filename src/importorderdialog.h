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

#include "bricklinkfwd.h"
#include "ui_importorderdialog.h"

class Transfer;
class TransferJob;
class OrderModel;


class ImportOrderDialog : public QDialog, private Ui::ImportOrderDialog
{
    Q_OBJECT
public:
    ImportOrderDialog(QWidget *parent = nullptr);
    ~ImportOrderDialog() override;

    void updateOrders();

protected:
    virtual void changeEvent(QEvent *e);
    void languageChange();

protected slots:
    void checkSelected();
    void activateItem();
    void updateStatusLabel();

    void downloadFinished(TransferJob *job);
    void importOrders();

private:
    Transfer *m_trans;
    QPushButton *w_import;
    QDateTime m_lastUpdated;
    QVector<TransferJob *> m_currentUpdate;
    QVector<TransferJob *> m_orderDownloads;
    OrderModel *m_orderModel;
};
