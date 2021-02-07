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
#include <QDataStream>
#include <QCompleter>
#include <QAction>
#include <QAbstractProxyModel>
#include <QAbstractItemView>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QListView>
#include <QDebug>

#include "historylineedit.h"


class HistoryViewDelegate : public QStyledItemDelegate
{
public:
    HistoryViewDelegate(HistoryLineEdit *filter);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

private:
    HistoryLineEdit *m_edit;
};

HistoryViewDelegate::HistoryViewDelegate(HistoryLineEdit *filter)
    : QStyledItemDelegate(filter)
    , m_edit(filter)
{ }

void HistoryViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    QRect r = option.rect;
    r.setLeft(r.left() + r.width() - r.height());
    const int margin = 2;
    r.adjust(margin, margin, -margin, -margin);
    m_edit->m_deleteIcon.paint(painter, r);
}

class HistoryView : public QListView
{
public:
    HistoryView(QWidget *parent)
        : QListView(parent)
    { }

protected:
    void paintEvent(QPaintEvent *e)
    {
        QListView::paintEvent(e);

        if (model()->rowCount() == 0) {
            QPainter p(viewport());
            p.drawText(viewport()->contentsRect(), Qt::AlignCenter,
                       tr("No favorite filters. Read the tooltip."));
        }
    }

    void mousePressEvent(QMouseEvent *me)
    {
        if (me->button() == Qt::LeftButton) {
            QModelIndex idx = indexAt(me->pos());

            if (idx.isValid()) {
                int h = visualRect(idx).height();
                if (me->x() >= (viewport()->width() - h)) {
                    if (auto proxy = qobject_cast<const QAbstractProxyModel *>(idx.model()))
                        idx = proxy->mapToSource(idx);
                    const_cast<QAbstractItemModel *>(idx.model())->removeRow(idx.row());
                    return;
                }
            }
        }
    }
};


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


HistoryLineEdit::HistoryLineEdit(int maximumHistorySize, QWidget *parent)
    : QLineEdit(parent)
    , m_deleteIcon(QIcon::fromTheme("window-close"))
    , m_maximumHistorySize(maximumHistorySize)
{
    auto *comp = new QCompleter(&m_filterModel, this);
    comp->setMaxVisibleItems(maximumHistorySize);

    comp->setCaseSensitivity(Qt::CaseInsensitive);
    comp->setCompletionMode(QCompleter::PopupCompletion);
    comp->setPopup(new HistoryView(this));
    setCompleter(comp);
    auto view = completer()->popup();
    view->setItemDelegate(new HistoryViewDelegate(this));
    setClearButtonEnabled(true);

    m_connection = connect(this, &QLineEdit::editingFinished,
                           this, &HistoryLineEdit::appendToModel, Qt::QueuedConnection);

    // Adding a menuAction() to a QLineEdit leads to a strange activation behvior:
    // only the right side of the icon will react to mouse clicks
    QPixmap filterPix(QIcon::fromTheme("view-filter").pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
    {
        QPainter p(&filterPix);
        QStyleOption so;
        so.initFrom(this);
        so.rect = filterPix.rect();
        int mbi = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &so, this);
#if defined(Q_OS_MACOS)
        mbi += 2;
#else
        mbi -= 6;
#endif
        so.rect = QRect(0, so.rect.bottom() - mbi, mbi, mbi);
        style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &so, &p, this);
    }

    auto *a = addAction(QIcon(filterPix), QLineEdit::LeadingPosition);
    connect(a, &QAction::triggered,
            this, &HistoryLineEdit::showPopup);
}

void HistoryLineEdit::appendToModel()
{
    QString t = text().trimmed();
    if (t.isEmpty())
        return;

    QStringList sl = m_filterModel.stringList();
    sl.removeAll(t);
    sl.append(t);
    while (sl.size() > m_maximumHistorySize)
        sl.removeFirst();
    m_filterModel.setStringList(sl);
}

bool HistoryLineEdit::isInFavoritesMode() const
{
    return m_favoritesMode;
}

void HistoryLineEdit::setToFavoritesMode(bool favoritesMode)
{
    if (favoritesMode != m_favoritesMode) {
        m_favoritesMode = favoritesMode;

        disconnect(m_connection);
        m_connection = connect(this, favoritesMode ? &QLineEdit::returnPressed
                                                   : &QLineEdit::editingFinished,
                               this, &HistoryLineEdit::appendToModel, Qt::QueuedConnection);
    }
}

QString HistoryLineEdit::instructionToolTip() const
{
    return tr("<p>" \
              "Pressing <b>Return</b> will save the current filter expression as a favorite. " \
              "You can recall saved filters by clicking the filter icon on the left or by " \
              "pressing <b>Down Arrow</b> to open a drop down menu. The number of favorites is " \
              "limited to the last %1, but you can delete saved filters from this drop down " \
              "menu as well by clicking the <b>X</b> button on the right." \
              "</p>").arg(m_maximumHistorySize);
}

QByteArray HistoryLineEdit::saveState() const
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << QByteArray("FH") << qint32(3);
    ds << text();
    ds << m_filterModel.stringList();

    return ba;
}

bool HistoryLineEdit::restoreState(const QByteArray &ba)
{
    QDataStream ds(ba);
    QByteArray tag;
    qint32 version;
    ds >> tag >> version;
    if ((ds.status() != QDataStream::Ok) || (tag != "FH") || (version != 3))
        return false;

    QString text;
    QStringList list;
    ds >> text >> list;

    if (ds.status() != QDataStream::Ok)
        return false;

    m_filterModel.setStringList(list);
    setText(text);
    return true;
}

void HistoryLineEdit::showPopup()
{
    //if (text().isEmpty())
    completer()->setCompletionPrefix(QString());
    bool forcePopup = (m_filterModel.rowCount() == 0);
    if (forcePopup)
        m_filterModel.insertRow(0); // dummy row to enable popup
    completer()->complete();
    if (forcePopup)
        m_filterModel.removeRow(0);
}

void HistoryLineEdit::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Down && !ke->modifiers())
        showPopup();
    return QLineEdit::keyPressEvent(ke);
}