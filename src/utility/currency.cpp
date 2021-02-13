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
#include <cstdlib>
#include <clocale>
#include <climits>

#include <QApplication>
#include <QLocale>
#include <QValidator>
#include <QRegExp>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDataStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QStringBuilder>

#include "application.h"
#include "config.h"
#include "utility.h"
#include "currency.h"
#include "messagebox.h"
#include "framework.h"


// all rates as downloaded are relative to the Euro, but
// we need everything relative to the US Dollar
static void baseConvert(QMap<QString, qreal> &rates)
{
    QMap<QString, qreal>::iterator usd_it = rates.find(QLatin1String("USD"));

    if (usd_it == rates.end()) {
        rates.clear();
        return;
    }
    qreal usd_eur = qreal(1) / usd_it.value();

    for (QMap<QString, qreal>::iterator it = rates.begin(); it != rates.end(); ++it)
        it.value() = (it == usd_it) ? 1 : it.value() *= usd_eur;

    rates.insert(QLatin1String("EUR"), usd_eur);
}


Currency *Currency::s_inst = nullptr;

Currency::Currency()
    : m_nam(nullptr)
{
    m_lastUpdate = Config::inst()->value("/Rates/LastUpdate").toDateTime();
    QStringList rates = Config::inst()->value("/Rates/Normal").toString().split(QLatin1Char(','));
    QStringList customRates = Config::inst()->value("/Rates/Custom").toString().split(QLatin1Char(','));

    parseRates(rates, m_rates);
    parseRates(customRates, m_customRates);
}

Currency::~Currency()
{
    Config::inst()->setValue("/Rates/LastUpdate", m_lastUpdate);

    QStringList sl;
    for (auto it = m_rates.constBegin(); it != m_rates.constEnd(); ++it) {
        QString s = QLatin1String("%1|%2");
        sl << s.arg(it.key()).arg(it.value());
    }
    Config::inst()->setValue("/Rates/Normal", sl.join(QLatin1String(",")));

    sl.clear();
    for (auto it = m_customRates.constBegin(); it != m_customRates.constEnd(); ++it) {
        QString s = QLatin1String("%1|%2");
        sl << s.arg(it.key()).arg(it.value());
    }
    Config::inst()->setValue("/Rates/Custom", sl.join(QLatin1String(",")));

    s_inst = nullptr;
}

Currency *Currency::inst()
{
    if (!s_inst)
        s_inst = new Currency;
    return s_inst;
}

void Currency::parseRates(const QStringList &ratesList, QMap<QString, double> &ratesMap)
{
    for (const QString &s : ratesList) {
        QStringList sl = s.split(QLatin1Char('|'));
        if (sl.count() == 2) {
            QString sym = sl[0];
            qreal rate = sl[1].toDouble();

            if (!sym.isEmpty() && rate > 0)
                ratesMap.insert(sym, rate);
        }
    }
}


QStringList Currency::currencyCodes() const
{
    return m_rates.keys();
}

QMap<QString, qreal> Currency::rates() const
{
    return m_rates;
}

QMap<QString, qreal> Currency::customRates() const
{
    return m_customRates;
}

qreal Currency::rate(const QString &currencyCode) const
{
    return m_rates.value(currencyCode);
}

qreal Currency::customRate(const QString &currencyCode) const
{
    return m_customRates.value(currencyCode);
}

void Currency::setCustomRate(const QString &currencyCode, qreal rate)
{
    m_customRates.insert(currencyCode, rate);
}

void Currency::unsetCustomRate(const QString &currencyCode)
{
    m_customRates.remove(currencyCode);
}

void Currency::updateRates(bool silent)
{
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        connect(m_nam, &QNetworkAccessManager::finished,
                this, [this, silent](QNetworkReply *reply) {
            if (reply->error() != QNetworkReply::NoError) {
                if (Application::inst()->isOnline() && !silent)
                    MessageBox::warning(nullptr, { }, tr("There was an error downloading the exchange rates from the ECB server:<br>%1").arg(reply->errorString()));
            } else {
                auto r = reply->readAll();
                QXmlStreamReader reader(r);
                QMap<QString, qreal> newRates;
                static QLocale c = QLocale::c();

                while (!reader.atEnd()) {
                    if (reader.readNext() == QXmlStreamReader::StartElement &&
                        reader.name() == QLatin1String("Cube") &&
                        reader.attributes().hasAttribute(QLatin1String("currency"))) {

                        QString currency = reader.attributes().value(QLatin1String("currency")).toString();
                        qreal rate = c.toDouble(reader.attributes().value(QLatin1String("rate")).toString());

                        if (currency.length() == 3 && rate > 0)
                            newRates.insert(currency, rate);
                    }
                }
                baseConvert(newRates);

                if (reader.hasError() || newRates.isEmpty()) {
                    if (!silent) {
                        QString err;
                        if (reader.hasError())
                            err = tr("%1 (line %2, column %3)").arg(reader.errorString()).arg(reader.lineNumber()).arg(reader.columnNumber());
                        else
                            err = tr("no currency data found");
                        MessageBox::warning(nullptr, { }, tr("There was an error parsing the exchange rates from the ECB server:\n%1").arg(err));
                    }
                } else {
                    m_rates = newRates;
                    m_lastUpdate = QDateTime::currentDateTime();
                    emit ratesChanged();
                }
            }
            reply->deleteLater();
        });
    }
    if (Application::inst()->isOnline()) {
        m_nam->get(QNetworkRequest(QUrl("http://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml")));
    }
}

QDateTime Currency::lastUpdate() const
{
    return m_lastUpdate;
}

QString Currency::toString(double value, const QString &currencyCode, int precision)
{
    QLocale loc;
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    if (currencyCode.isEmpty())
        return loc.toString(value, 'f', precision);
    else
        return currencyCode % u' ' % loc.toString(value, 'f', precision);
}

double Currency::fromString(const QString &str)
{
    QString s = str.trimmed();
    if (s.isEmpty())
        return 0;
    return QLocale().toDouble(s);
}

#include "moc_currency.cpp"
