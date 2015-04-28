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

#ifndef DIALOG_H
#define DIALOG_H

#include <QSystemTrayIcon>
#include <QDomDocument>
#include <QDialog>
#include <iostream>
#include <QList>
#include <QProcess>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "qticonloader.h"
#include "ui_dialog.h"
#include <QTimer>
#include <BigAES.h>

#include "QuotaManager.h"

//#include "options.h"

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog, private Ui_Dialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void setVisible(bool visible);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
	void hide();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage(QString title, QString message, int time);
    void messageClicked();
    void readConfiguration();
    void acceptPushButtonClicked();
    void saveConfiguration();
    //void showOptionsDialog();
    //void changeOptions(bool hide,QString cntlmPath, bool start);
    void quit();
    void standardOutput();
    void standardError();
    void all();
    void terminateProcess();
    void processFinished();
    void processStarted();
    void processError(QProcess::ProcessError error);
    void uiReadyToConnect();
    void uiConnected();
	
	void onQuotaUpdated(QString user, QString quote_used, QString quote_assigned, QString nav_level);
	void onCheckBox1Changed(int);
	void onCheckBox2Changed(int);

private:
    void createIconGroupBox();
    void createMessageGroupBox();
    void createActions();
    void createTrayIcon();
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QString configPath;
    QProcess* process;
    bool hidden;
    QString workPath;
    QString cntlmPath;
    bool start , temp_message, m_connected;
	
	QuotaManager *qmanager;
	QTimer *timer;
	
	BigAES *aes;
	QByteArray ppassword;
	
	void goConnected();
	void goDisconnected();
	void encrypt(QString fileName, QByteArray data);
	QByteArray decrypt(QString fileName);

signals:
    void readyToConnect();
    void connected();
};

#endif // DIALOG_H
