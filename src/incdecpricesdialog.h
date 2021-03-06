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
#include "currency.h"
#include "ui_incdecpricesdialog.h"


class IncDecPricesDialog : public QDialog, private Ui::IncDecPricesDialog
{
    Q_OBJECT
public:
    IncDecPricesDialog(bool showTiers, const QString &currencycode, QWidget *parent = nullptr);

    double fixed() const;
    double percent() const;
    bool applyToTiers() const;

private slots:
    void updateValidators();
    void checkValue();
    
private:
    QDoubleValidator *m_pos_percent_validator;
    QDoubleValidator *m_neg_percent_validator;
    QDoubleValidator *m_fixed_validator;
    QString m_currencycode;
};
