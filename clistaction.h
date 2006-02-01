/* Copyright (C) 2004-2005 Robert Griebl.  All rights reserved.
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
#ifndef __CLISTACTION_H__
#define __CLISTACTION_H__

#include <qaction.h>
#include <qmap.h>
#include <qvaluevector.h>

class QStringList;

class CListAction : public QActionGroup {
	Q_OBJECT
public:

	class Provider {
	public:
		virtual ~Provider ( )
		{ }
		
		virtual QStringList list ( int &active_index, QValueList <int> &custom_ids ) = 0;
	};

	CListAction ( bool use_numbers, QObject *parent, const char *name );
	virtual ~CListAction ( );

	void setListProvider ( Provider * );
	Provider *listProvider ( ) const;

	void setList ( const QStringList *list );
	const QStringList *list ( ) const;

	virtual bool addTo ( QWidget *w );
	virtual bool removeFrom ( QWidget *w );

	virtual void setText ( const QString &txt );

signals:
	void activated ( int );

private slots:
	void refreshMenu ( );
//	void doEmits ( int );

private:
	const QStringList *m_list;
	Provider *m_provider;
	bool m_use_numbers;

	QMap <QPopupMenu *, QValueVector<int> > m_id_map;
	QMap <QPopupMenu *, int> m_update_menutexts;
};

#endif
