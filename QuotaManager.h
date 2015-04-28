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

#ifndef QUOTAMANAGER_H
#define QUOTAMANAGER_H

#include <QNetworkAccessManager>


class QuotaManager : public QObject
{
	Q_OBJECT
	
public:
	QuotaManager(QString user, QString password, QString domain="uci.cu", QObject *parent=0);
	QuotaManager(QObject *parent=0);
	QString user() {return this->m_user;}
	QString password() {return this->m_password;}
	QString domain() {return this->m_domain;}
	void setUser(QString n_user) {this->m_user = n_user;}
	void setPassword(QString n_password) {this->m_password = n_password;}
	void setDomain(QString n_domain) {this->m_domain = n_domain;}
	QString quoteUsed() { return this->m_quote_used;};
	QString quoteAssigned() { return this->m_quote_assigned;};
	QString navLevel() { return this->m_nav_level;};
	void setEnabled(bool n_enabled) { this->m_enabled = n_enabled;}
	bool enabled() { return this->m_enabled;}
	~QuotaManager();
	
private:
	QString m_user, m_password, m_domain, m_quote_used, m_quote_assigned, m_nav_level;
	QNetworkAccessManager *manager;
	bool m_enabled;
	
	void init();
	
private slots:
	void finished(QNetworkReply *reply);
	void sslErrors(QNetworkReply *reply, QList<QSslError> list);
	
public slots:
	void update();
	
signals:
	void updated(QString user, QString quote_used, QString quote_assigned, QString nav_level);
};


#endif //QUOTAMANAGER_H