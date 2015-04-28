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

#include "options.h"

Options::Options(bool hidden,  QString cntlmPath, bool start, QWidget *parent) :
QDialog(parent)
{
   setupUi(this);

   hideCheckBox->setChecked(hidden);
   this->cntlmPath=cntlmPath;
   startCheckBox->setChecked(start);

    connect(acceptPushButton,SIGNAL(clicked()),this,SLOT(pushButtonClicked()));
       connect(pathToolButton,SIGNAL(clicked()),this,SLOT(selectCntlmPath()));
}

   void Options::pushButtonClicked()
   {
       emit accepted(hideCheckBox->isChecked(),cntlmPath,startCheckBox->isChecked());
       this->close();
   }

   void Options::selectCntlmPath()
   {
       QFileDialog::Options options;
       //options |= QFileDialog::DontUseNativeDialog;
       QString selectedFilter;
       QString result= QFileDialog::getOpenFileName(this,
                                   tr("Archivo binario"),
                                   cntlmPath,
                                   tr("Binario (*)"),
                                   &selectedFilter,options);
       if(!result.isEmpty())
       {
           cntlmPath=result;
           cntlmPath.replace(" ","\\ ");
       }
           std::cout<<"path: "<<cntlmPath.toStdString()<<std::endl;
   }

Options::~Options()
{
}
