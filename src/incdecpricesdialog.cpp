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
#include <QPushButton>

#include "smartvalidator.h"
#include "incdecpricesdialog.h"
#include "framework.h"


IncDecPricesDialog::IncDecPricesDialog(bool showTiers, const QString &currencycode, QWidget *parent)
    : QDialog(parent)
    , m_currencycode(currencycode)
{
    setupUi(this);
    
    w_apply_to_tiers->setVisible(showTiers);

    w_value->setText(QLatin1String("0"));
    w_fixed->setText(currencycode);

    m_pos_percent_validator = new SmartDoubleValidator(0., 1000., 2, 0, this);
    m_neg_percent_validator = new SmartDoubleValidator(0., 99.99, 2, 0, this);
    m_fixed_validator       = new SmartDoubleValidator(0, FrameWork::maxPrice, 3, 0, this);

    connect(w_increase, &QAbstractButton::toggled,
            this, &IncDecPricesDialog::updateValidators);
    connect(w_percent, &QAbstractButton::toggled,
            this, &IncDecPricesDialog::updateValidators);
    connect(w_value, &QLineEdit::textChanged,
            this, &IncDecPricesDialog::checkValue);

    w_buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    updateValidators();

    w_value->selectAll();
}

void IncDecPricesDialog::checkValue()
{
    w_buttons->button(QDialogButtonBox::Ok)->setEnabled(w_value->hasAcceptableInput());
}

void IncDecPricesDialog::updateValidators()
{
    if (w_percent->isChecked()) {
        w_value->setValidator(w_increase->isChecked() ? m_pos_percent_validator : m_neg_percent_validator);
        w_value->setText(QLatin1String("0"));
    } else {
        w_value->setValidator(m_fixed_validator);
        w_value->setText(Currency::toString(0));
    }
    checkValue();
}


double IncDecPricesDialog::fixed() const
{
    if (w_value->hasAcceptableInput() && w_fixed->isChecked()) {
        double v = Currency::fromString(w_value->text());
        return w_increase->isChecked() ? v : -v;
    } else {
        return 0;
    }
}

double IncDecPricesDialog::percent() const
{
    if (w_value->hasAcceptableInput() && w_percent->isChecked()) {
        double v = QLocale().toDouble(w_value->text());
        return w_increase->isChecked() ? v : -v;
    } else {
        return 0.;
    }
}

bool IncDecPricesDialog::applyToTiers() const
{
    return w_apply_to_tiers->isChecked();
}

#include "moc_incdecpricesdialog.cpp"
