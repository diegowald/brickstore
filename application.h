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
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <QApplication>
#include <QStringList>


class FrameWork;
class QTranslator;

class Application : public QApplication {
    Q_OBJECT
public:
    Application(bool rebuild_db_only, int argc, char **argv);
    virtual ~Application();

    void enableEmitOpenDocument(bool b = true);

    QString appName() const;
    QString appVersion() const;
    QString appURL() const;
    QString sysName() const;
    QString sysVersion() const;

    bool pixmapAlphaSupported() const;

public slots:
    void about();
    void checkForUpdates();
    void updateTranslations();
    void registration();

signals:
    void openDocument(const QString &);

protected:
    virtual bool event(QEvent *e);

private slots:
    void doEmitOpenDocument();
    void demoVersion();
    void rebuildDatabase();

private:
    bool initBrickLink();
    void exitBrickLink();

private:
    QStringList m_files_to_open;
    bool m_enable_emit;
    bool m_has_alpha;

    QTranslator *m_trans_qt;
    QTranslator *m_trans_brickstore;
};

extern Application *cApp;

#endif