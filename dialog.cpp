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

#include "dialog.h"

#include <QDebug>
#include <QProgressBar>
#include <QThreadPool>
#include <QFile>

Dialog::Dialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	
	aes = new BigAES();
	QString token = "`1234567890-=~!@#$%^&*()_+qwertyuiop[]}proxycare1.0{POIUYTREWQ|asdfghjkl;':LKJHGFDSAzxcvbnm,./?><MNBVCXZ";
	ppassword = QCryptographicHash::hash(token.toUtf8(), QCryptographicHash::Md5).toHex();
	
	
#ifdef Q_OS_LINUX
	cntlmPath = "/usr/lib/proxycare/cntlm";
#endif
#ifdef Q_OS_WIN32
	cntlmPath = "proxycare/cntlm";
#endif
	createActions();
	createTrayIcon();
	process = new QProcess();
	connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	        this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
	//connect(optionsPushButton,SIGNAL(clicked()),this,SLOT(showOptionsDialog()));
	connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(standardOutput()));
	connect(process, SIGNAL(readyReadStandardError()), this, SLOT(standardError()));
	connect(process, SIGNAL(readyRead()), this, SLOT(all()));
	connect(process, SIGNAL(started()), this, SLOT(processStarted()));
	connect(process, SIGNAL(finished(int)), this, SLOT(processFinished()));
	connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(cancellPushButton, SIGNAL(clicked()), this, SLOT(terminateProcess()));
	connect(acceptPushButton, SIGNAL(clicked()), this, SLOT(acceptPushButtonClicked()));
	connect(this, SIGNAL(readyToConnect()), this, SLOT(uiReadyToConnect()));
	connect(this, SIGNAL(connected()), this, SLOT(uiConnected()));
	trayIcon->show();
	setFixedSize(270, 305);

	m_connected = false;
	timer = new QTimer(this);
	qmanager = new QuotaManager();
	connect(qmanager, SIGNAL(updated(QString, QString, QString, QString)),
	        this, SLOT(onQuotaUpdated(QString, QString, QString, QString)));
	connect(cmdRefresh, SIGNAL(clicked()), qmanager, SLOT(update()));
	temp_message = false;
	connect(checkBox1, SIGNAL(stateChanged(int)), this, SLOT(onCheckBox1Changed(int)));
	connect(checkBox2, SIGNAL(stateChanged(int)), this, SLOT(onCheckBox2Changed(int)));
	workPath = QDir::homePath() + "/.proxycare/";
	configPath = workPath + "proxycare.conf";
	readConfiguration();
	emit readyToConnect();
}

Dialog::~Dialog()
{
	delete qmanager;
	delete aes;
}

void Dialog::acceptPushButtonClicked()
{
	QString user = userLineEdit->text();
	QString domain = domainLineEdit->text();
	QString proxy = proxyLineEdit->text();
	QString proxyPort = QVariant(proxyPortSpinBox->value()).toString();
	QString port = QVariant(portSpinBox->value()).toString();
	QString passw = passLineEdit->text();
	QRegExp exp;
	exp = QRegExp("^[\\w-\\.]{3,}$");

	if (!exp.exactMatch(user))
	{
		QMessageBox::warning(this, tr("Advertencia"), tr("Usuario incorrecto"), QMessageBox::Ok);
		return;
	}

	exp = QRegExp("^([\\w-]{2,}\\.)*([\\w-]{2,}\\.)[\\w-]{2,4}$");

	if (!exp.exactMatch(domain))
	{
		QMessageBox::warning(this, tr("Advertencia"), tr("Dominio incorrecto"), QMessageBox::Ok);
		return;
	}

	exp = QRegExp("^(([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]).){3}([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");

	if (!exp.exactMatch(proxy))
	{
		QMessageBox::warning(this, tr("Advertencia"), tr("Proxy incorrecto"), QMessageBox::Ok);
		return;
	}

	exp = QRegExp("^\\d+$");

	if (!exp.exactMatch(proxyPort))
	{
		QMessageBox::warning(this, tr("Advertencia"), tr("Puerto del proxy incorrecto"), QMessageBox::Ok);
		return;
	}

	exp = QRegExp("^\\d+$");

	if (!exp.exactMatch(port))
	{
		QMessageBox::warning(this, tr("Advertencia"), tr("Puerto incorrecto"), QMessageBox::Ok);
		return;
	}

	exp = QRegExp("^\\s+$");

	if (passw.isEmpty() || exp.exactMatch(passw))
	{
		QMessageBox::warning(this, tr("Advertencia"), tr("Contraseña incorrecta"), QMessageBox::Ok);
		return;
	}

	QString program = cntlmPath + " -fl " + port + " -p " + passw + " -u " + user + "@" + domain + " " + proxy + ":" + proxyPort;
	std::cout << "Starting cntlm" << std::endl;
	process->start(program);

	goConnected();
}

void Dialog::goConnected()
{
	m_connected = true;
	QString user = userLineEdit->text();
	QString password = passLineEdit->text();
	QString domain = domainLineEdit->text();
	qmanager->setUser(user);
	qmanager->setPassword(password);
	qmanager->setDomain(domain);
	qmanager->setEnabled(true);
	qmanager->update();

	connect(timer, SIGNAL(timeout()), qmanager, SLOT(update()));
	timer->start(spin1->value() * 60 * 1000);
	cmdRefresh->setEnabled(true);
}

void Dialog::goDisconnected()
{
	m_connected = false;
	cmdRefresh->setEnabled(false);
	spin1->setEnabled(true);
	qmanager->setEnabled(false);
	timer->stop();
	trayIcon->setToolTip(QApplication::translate("Dialog", "Desconectado"));
}


void Dialog::onQuotaUpdated(QString user, QString quote_used, QString quote_assigned, QString nav_level)
{
	int max = quote_assigned.toInt();
	int val = int(quote_used.toDouble());
	QString str_val = QString::number(val);
// 	qDebug() << "\n[DEBUG]:" << max;
// 	qDebug() << "[DEBUG]:" << val;
	progressBar->setMaximum(max);
	progressBar->setValue(val);
	progressBar->setToolTip(QString("Cuenta usada: %1.\nCuenta asignada: %2.").arg(str_val).arg(quote_assigned));
	this->nav_level->setText(nav_level);

	QString text = QString("Conectado.\nUsuario: %1.\nCuenta: %2/%3.\n%4.").arg(user).arg(str_val).arg(quote_assigned).arg(nav_level);
	trayIcon->setToolTip(text);
}

void Dialog::setVisible(bool visible)
{
	minimizeAction->setEnabled(visible);
	restoreAction->setEnabled(isMaximized() || !visible);
	QDialog::setVisible(visible);
}

void Dialog::closeEvent(QCloseEvent *event)
{
	if (trayIcon->isVisible() && m_connected)
	{
		hide();
		event->ignore();
		QString message = "ProxyCare se encuentra \nactualmente activo en segundo plano.\nPara cerrar escoja quitar en el menu\ncontextual.";
		showMessage(tr("Aviso"), message, 5000);
	}
	else
	{
		quit();
	}
}

void Dialog::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
		case QSystemTrayIcon::Trigger:

			// showMessage("Informacion","Trigger");
		case QSystemTrayIcon::DoubleClick:
			//   iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1)  % iconComboBox->count());
			//showMessage("Informacion","DoubleClick");
			this->showNormal();
			break;

		case QSystemTrayIcon::MiddleClick:
			//showMessage("Informacion","MiddleClick");
			break;

		default:
			;
	}
}

void Dialog::showMessage(QString title, QString message, int time)
{
	if (!temp_message && !checkBox2->isChecked())
	{
		QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
		trayIcon->showMessage(title, message, icon, time);
		temp_message = true;
	}
}

void Dialog::messageClicked()
{
}

void Dialog::createActions()
{
	minimizeAction = new QAction(tr("Mi&nimizar"), this);
	minimizeAction->setIcon(QtIconLoader::icon("view-restore"));
	connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

	restoreAction = new QAction(tr("&Restaurar"), this);
	restoreAction->setIcon(QtIconLoader::icon("view-fullscreen"));
	connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

	quitAction = new QAction(tr("&Quitar"), this);
	quitAction->setIcon(QtIconLoader::icon("window-close"));
	connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
}

void Dialog::quit()
{
	saveConfiguration();
	terminateProcess();
	qApp->quit();
}

void Dialog::hide()
{
	QDialog::hide();
}

void Dialog::createTrayIcon()
{
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(minimizeAction);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	QByteArray category = qgetenv("SNI_CATEGORY");

	if (!category.isEmpty())
	{
		trayIcon->setProperty("_qt_sni_category", QString::fromLocal8Bit(category));
	}

	trayIcon->setIcon(QIcon(":/images/logo-off.svg"));
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->setToolTip(QApplication::translate("Dialog", "Desconectado"));

#if defined(Q_WS_X11)
	trayIcon->installEventFilter(this);
#endif
}

void Dialog::readConfiguration()
{
	QByteArray data = decrypt(configPath);
	QDomDocument doc;
	
	if (!doc.setContent(data, false))
	{
		quit();
		return;
	}

	QDomElement docElem = doc.documentElement();
	QDomNodeList nodeList = docElem.elementsByTagName("Option");

	for (int i = 0; i < nodeList.length(); i++)
	{
		QDomElement element = nodeList.at(i).toElement();

		if (element.attribute("name") == "user")
			userLineEdit->setText(element.attribute("value"));
		else if (element.attribute("name") == "password")
		{
			passLineEdit->setText(element.attribute("value"));
			savePassCheckBox->setChecked(true);
		}
		else if (element.attribute("name") == "domain")
			domainLineEdit->setText(element.attribute("value"));
		else if (element.attribute("name") == "proxy")
			proxyLineEdit->setText(element.attribute("value"));
		else if (element.attribute("name") == "proxyPort")
			proxyPortSpinBox->setValue(element.attribute("value").toInt());
		else if (element.attribute("name") == "port")
			portSpinBox->setValue(element.attribute("value").toInt());

		else if (element.attribute("name") == "interval")
			spin1->setValue(element.attribute("value").toInt());

		else if (element.attribute("name") == "start")
			checkBox1->setChecked(QVariant(element.attribute("value")).toBool());
		else if (element.attribute("name") == "hidden")
			checkBox2->setChecked(QVariant(element.attribute("value")).toBool());
	}

	if (checkBox1->isChecked()) // auto connect
	{
		acceptPushButtonClicked();
	}

	if (checkBox2->isChecked()) // hide on start
	{
		QTimer::singleShot(0, this, SLOT(hide()));
		hide();
	}
}

void Dialog::saveConfiguration()
{
	QString user = userLineEdit->text();
	QString password = passLineEdit->text();
	QString domain = domainLineEdit->text();
	QString proxy = proxyLineEdit->text();
	QString proxyPort = QVariant(proxyPortSpinBox->value()).toString();
	QString port = QVariant(portSpinBox->value()).toString();
	QDomDocument domDocument;
	domDocument.appendChild(domDocument.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
	QDomElement rootNode = domDocument.createElement("Configuration");

	QDomElement node = domDocument.createElement("Option");
	node.setAttribute("name", "user");
	node.setAttribute("value", user);
	rootNode.appendChild(node);

	node = domDocument.createElement("Option");
	node.setAttribute("name", "domain");
	node.setAttribute("value", domain);
	rootNode.appendChild(node);

	node = domDocument.createElement("Option");
	node.setAttribute("name", "proxy");
	node.setAttribute("value", proxy);
	rootNode.appendChild(node);


	node = domDocument.createElement("Option");
	node.setAttribute("name", "proxyPort");
	node.setAttribute("value", proxyPort);
	rootNode.appendChild(node);

	node = domDocument.createElement("Option");
	node.setAttribute("name", "port");
	node.setAttribute("value", port);
	rootNode.appendChild(node);

	node = domDocument.createElement("Option");
	node.setAttribute("name", "interval");
	node.setAttribute("value", QVariant(spin1->value()).toString());
	rootNode.appendChild(node);

	node = domDocument.createElement("Option");
	node.setAttribute("name", "start");
	node.setAttribute("value", QVariant(checkBox1->isChecked()).toString());
	rootNode.appendChild(node);

	node = domDocument.createElement("Option");
	node.setAttribute("name", "hidden");
	node.setAttribute("value", QVariant(checkBox2->isChecked()).toString());
	rootNode.appendChild(node);


	if (savePassCheckBox->isChecked())
	{
		node = domDocument.createElement("Option");
		node.setAttribute("name", "password");
		node.setAttribute("value", password);
		rootNode.appendChild(node);
	}


	domDocument.appendChild(rootNode);

	QDir workDir(workPath);

	if (!workDir.exists())
		workDir.mkpath(workPath);
	
	encrypt(configPath, domDocument.toByteArray());

}

void Dialog::standardOutput()
{
	QString out = process->readAllStandardOutput();
	QRegExp delRegExp("\\s*cntlm\\[\\d+\\]\\:\\s*");
	out.remove(delRegExp);
	qDebug() << out;
	//std::cout<<out.toAscii().data()<<std::endl;
}

void Dialog::standardError()
{
	QString out = process->readAllStandardError();
	QRegExp exp( "^.*Exitting with error.*$" );

	if (exp.exactMatch(out))
	{
		QString message = "No se ha podido establecer la conexión,\nverifique que cntlm no esté actualmente en ejecución\ny que los parámetros estén correctos";
		QMessageBox::warning(this, tr("Error"), message, QMessageBox::Ok);
		this->showNormal();
	}

	QRegExp delRegExp("\\s*cntlm\\[\\d+\\]\\:\\s*");
	out = out.remove(delRegExp);
	qDebug() << out;
	//   std::cout<<out.toAscii().data()<<std::endl;
}

void Dialog::all()
{
	QString out = process->readAll();
	QRegExp delRegExp("\\s*cntlm\\[\\d+\\]\\:\\s*");
	out.remove(delRegExp);
	qDebug() << out;
	//   std::cout<<out.toAscii().data()<<std::endl;
}

void Dialog::processFinished()
{
	std::cout << "Cntlm is now closed" << std::endl;
	trayIcon->setIcon(QIcon(":/images/logo-off.svg"));
	emit readyToConnect();
}


void Dialog::terminateProcess()
{
	if (process->state() == QProcess::Running)
	{
		std::cout << "Closing cntlm" << std::endl;
#ifdef Q_OS_LINUX
		process->execute("killall -s KILL cntlm");
#endif
#ifdef Q_OS_WIN32
		process->kill();
#endif
	}

	emit readyToConnect();
	goDisconnected();
}


void Dialog::processStarted()
{
	trayIcon->setIcon(QIcon(":/images/logo-on.svg"));
	emit connected();
}

void Dialog::processError(QProcess::ProcessError error)
{
	if (error == QProcess::FailedToStart)
	{
		std::cout << "Failed to start cntlm process" << std::endl;
		QString message = "No se ha podido establecer la conexión,\nno se encuentra " + cntlmPath;
		QMessageBox::warning(this, tr("Error"), message, QMessageBox::Ok);
		emit readyToConnect();
		this->showNormal();
	}

	goDisconnected();
}

void Dialog::uiReadyToConnect()
{
	frame1->setEnabled(true);
	frame2->setEnabled(true);
	spin1->setEnabled(true);
	acceptPushButton->setEnabled(true);
	cancellPushButton->setEnabled(false);
}

void Dialog::uiConnected()
{
	frame1->setEnabled(false);
	frame2->setEnabled(false);
	spin1->setEnabled(false);
	acceptPushButton->setEnabled(false);
	cancellPushButton->setEnabled(true);
}

void Dialog::onCheckBox1Changed(int)
{
// 	qDebug() << Q_FUNC_INFO;
}

void Dialog::onCheckBox2Changed(int)
{

}


void Dialog::encrypt(QString fileName, QByteArray data)
{
	QByteArray encrypted = aes->Encrypt(data, ppassword);
	
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly)){
		quit();
	}
	file.write(encrypted);
	file.close();
}

QByteArray Dialog::decrypt(QString fileName)
{
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly)){
		quit();
	}
	QByteArray input = file.readAll();
	file.close();
	
	QByteArray decrypted = aes->Decrypt(input, ppassword);
	
	return decrypted;
}





















