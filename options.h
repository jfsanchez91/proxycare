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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>
#include <QDialog>
#include <QFileDialog>
#include <iostream>
#include "ui_options.h"

namespace Ui {
    class Options;
}

class Options : public QDialog, private Ui_Options
{
    Q_OBJECT

public:
    explicit Options(bool hidden,  QString cntlmPath, bool start, QWidget *parent = 0);
    ~Options();
private:
    QString cntlmPath;
signals:
    void accepted(bool hide,QString cntlmPath, bool start);
public slots:
    void pushButtonClicked();
    void selectCntlmPath();

};

#endif // OPTIONS_H
