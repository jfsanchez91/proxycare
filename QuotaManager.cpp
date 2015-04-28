/**
* ProxyCare a CNTLM proxy GUI.
* Copyright (C) 2015  Jorge Fernández Sánchez <jfsanchez@estudiantes.uci.cu>
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/


#include "QuotaManager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QUrl>
#include <QDebug>

QuotaManager::QuotaManager(QString user, QString password, QString domain, QObject *parent):
m_user(user), m_password(password), m_domain(domain), QObject(parent)
{
	init();
}

QuotaManager::QuotaManager(QObject *parent):
QObject(parent)
{
	m_user = "";
	m_password = "";
	m_domain = "";
	init();
}

void QuotaManager::init()
{
	m_enabled = true;
	manager = new QNetworkAccessManager(this);
	connect(manager,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
	connect(manager,SIGNAL(finished(QNetworkReply*)), SLOT(finished(QNetworkReply*)));
}

QuotaManager::~QuotaManager()
{
	delete manager;
}

void QuotaManager::update()
{
	if(!m_enabled) return;
	QUrl url("https://cuotas.uci.cu/servicios/v1/InetCuotasWS.php?wsdl");
	QString msg = 	"<soapenv:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""\
					"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\""\
					"xmlns:urn=\"urn:InetCuotasWS\">" \
					"<soapenv:Header/>"\
					"<soapenv:Body>"\
					"<urn:ObtenerCuota soapenv:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"\
					"<usuario xsi:type=\"xsd:string\">$user$</usuario>"\
					"<clave xsi:type=\"xsd:string\">$passwd$</clave>"\
					"<dominio xsi:type=\"xsd:string\">$domain$</dominio>"\
					"</urn:ObtenerCuota>"\
					"</soapenv:Body>"\
					"</soapenv:Envelope>";
					
	msg = msg.replace("$user$", m_user);
	msg = msg.replace("$passwd$", m_password);
	msg = msg.replace("$domain$", m_domain);
	
	QNetworkRequest request(url);
	manager->post(request, msg.toUtf8());
}

void QuotaManager::sslErrors(QNetworkReply *reply, QList< QSslError > list)
{
	foreach(QSslError err, list)
	{
		reply->ignoreSslErrors();
	}
}

void QuotaManager::finished(QNetworkReply *reply)
{
	QString data = QString(reply->readAll());
	QRegExp re(">(\\d+)</cuota>.+>(\\d+.?\\d+)</cuota_usada>.+\">(\\D+)</nivel_navegacion>");
	re.indexIn(data);
	m_quote_assigned = re.cap(1);
	m_quote_used = re.cap(2);
	m_nav_level = re.cap(3);
	emit updated(m_user, m_quote_used, m_quote_assigned, m_nav_level);
}
