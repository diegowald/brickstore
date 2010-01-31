/* Copyright (C) 2004-2008 Robert Griebl. All rights reserved.
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
#include <QLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QEvent>
#include <QComboBox>

#include "bricklink.h"
#include "selectcolor.h"


SelectColor::SelectColor(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    w_filter = new QComboBox();
    w_filter->addItem(tr("All Colors"), 0);
    w_filter->addItem(tr("Popular Colors"), -1);
    w_filter->addItem(tr("Most Popular Colors"), -2);
    w_filter->insertSeparator(w_filter->count());

    for (int i = 0; (1 << i ) & BrickLink::Color::Mask; ++i) {
        BrickLink::Color::TypeFlag flag = static_cast<BrickLink::Color::TypeFlag>(1 << i);
        if (const char *type = BrickLink::Color::typeName(flag))
            w_filter->addItem(tr("Only \"%1\" Colors").arg(QLatin1String(type)), flag);
    }

    w_colors = new QTreeView();
    w_colors->setAlternatingRowColors(true);
    w_colors->setAllColumnsShowFocus(true);
    w_colors->setUniformRowHeights(true);
    w_colors->setRootIsDecorated(false);
    w_colors->setSortingEnabled(true);
    w_colors->setItemDelegate(new BrickLink::ItemDelegate(this, BrickLink::ItemDelegate::AlwaysShowSelection));

    w_colors->setModel(new BrickLink::ColorModel(this));
    w_colors->sortByColumn(0, Qt::AscendingOrder);

    setFocusProxy(w_colors);

    connect(w_colors->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(colorChanged()));
    connect(w_colors, SIGNAL(activated(const QModelIndex &)), this, SLOT(colorConfirmed()));
    connect(w_filter, SIGNAL(currentIndexChanged(int)), this, SLOT(updateColorFilter(int)));

    QBoxLayout *lay = new QVBoxLayout(this);
    lay->setMargin(0);
    lay->addWidget(w_filter);
    lay->addWidget(w_colors);
}

void SelectColor::setWidthToContents(bool b)
{
    if (b) {
        int w1 = static_cast<QAbstractItemView *>(w_colors)->sizeHintForColumn(0) + 2 * w_colors->frameWidth() + w_colors->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 4;
        int w2 = w_filter->minimumSizeHint().width();
        w_filter->setFixedWidth(qMax(w1, w2));
        w_colors->setFixedWidth(qMax(w1, w2));
    }
    else {
        w_colors->setMaximumWidth(INT_MAX);
        w_filter->setMaximumWidth(INT_MAX);
    }
}

void SelectColor::updateColorFilter(int index)
{
    int filter = w_filter->itemData(index < 0 ? 0 : index).toInt();
    BrickLink::ColorModel *model = qobject_cast<BrickLink::ColorModel *>(w_colors->model());

    if (filter > 0) {
        model->setFilterType(static_cast<BrickLink::Color::TypeFlag>(filter));
        model->setFilterPopularity(0);
    } else {
        qreal popularity = 0;
        if (filter == -1)
            popularity = qreal(0.8);
        else if (filter == -2)
            popularity = qreal(0.5);

        model->setFilterType(0);
        model->setFilterPopularity(popularity);
    }
}

const BrickLink::Color *SelectColor::currentColor() const
{
    if (w_colors->selectionModel()->hasSelection()) {
        QModelIndex idx = w_colors->selectionModel()->selectedIndexes().front();
        return qvariant_cast<const BrickLink::Color *>(w_colors->model()->data(idx, BrickLink::ColorPointerRole));
    }
    else
        return 0;
}

void SelectColor::setCurrentColor(const BrickLink::Color *color)
{
    BrickLink::ColorModel *model = qobject_cast<BrickLink::ColorModel *>(w_colors->model());

    w_colors->setCurrentIndex(model->index(color));
}

void SelectColor::colorChanged()
{
    emit colorSelected(currentColor(), false);
}

void SelectColor::colorConfirmed()
{
    emit colorSelected(currentColor(), true);
}

void SelectColor::showEvent(QShowEvent *)
{
    const BrickLink::Color *color = currentColor();

    if (color) {
        BrickLink::ColorModel *model = qobject_cast<BrickLink::ColorModel *>(w_colors->model());

        w_colors->scrollTo(model->index(color), QAbstractItemView::PositionAtCenter);
    }
}

void SelectColor::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::EnabledChange) {
        if (!isEnabled())
            setCurrentColor(BrickLink::core()->color(0));
    }
    QWidget::changeEvent(e);
}