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

#include <QtGui>
#include <QApplication>

#include "dialog.h"

int main(int argc, char **argv)
{
//     #ifdef Q_OS_LINUX
//     QTextCodec *linuxCodec = QTextCodec::codecForName("UTF-8");
//     QTextCodec::setCodecForTr(linuxCodec);
//     QTextCodec::setCodecForCStrings(linuxCodec);
//     QTextCodec::setCodecForLocale(linuxCodec);
//     #endif
    Q_INIT_RESOURCE(systray);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Qt-cntlm"),
                              QObject::tr("No se ha detectado el system tray "
                                          "en su sistema."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    Dialog dialog;
    dialog.show();
    return app.exec();
}
