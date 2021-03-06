/******************************************************************************
*   *File: xmlsettingshandler.cpp                                             *
*   *Date: 2013-06-01                                                         *
*   *Version: 1.0                                                             *
*   *Author(s): Jochen Bauer <devel@jochenbauer.net>                          *
*               Lukas Kern <lukas.kern@online.de>                             *
*               Carsten Klein <hook-the-master@gmx.net>                       *
*                                                                             *
*   *License information:                                                     *
*                                                                             *
*   Copyright (C) [2013] [Jochen Bauer, Lukas Kern, Carsten Klein]            *
*                                                                             *
*   This file is part of PLCANALYZER. PLCANALYZER is free software; you can   *
*   redistribute it and/or modify it under the terms of the GNU General       *
*   Public License as published by the Free Software Foundation; either       *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This program is distributed in the hope that it will be useful, but       *
*   WITHOUT ANY WARRANTY; without even the implied warranty of                *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA             *
*   02110-1301, USA.                                                          *
*                                                                             *
*   *Program name: PLCANALYZER                                                *
*   *What this program does:    Connects to most SIMATIC S7/S5 Controllers    *
*                               * Reads memory areas                          *
*                               * Draws a graph over time for operands        *
*   *Have fun!                                                                *
******************************************************************************/

#include<QFileDialog>
#include "xmlsettingshandler.h"
#include <iostream>
#ifdef BCCWIN
#include "windows.h"
#endif

xmlSettingsHandler::xmlSettingsHandler(QMainWindow *parent)
    : QMainWindow(parent)
{
    openedConSets = new ConSets;
}

xmlSettingsHandler::~xmlSettingsHandler()
{

}

void xmlSettingsHandler::newProject()
{
    // Todo: implement!
}

bool xmlSettingsHandler::openProject(bool lastSets, bool emptySet)
{
    // Open a new project. Also used for any existent settings from before
    if(!lastSets && !emptySet)
    {
        filename = QFileDialog::getOpenFileName(this,"Öffnen",".","PLCANALYZER-"
                                                "Projekte (*.plcproj)");
    }else if(lastSets)
    {
        filename = ".lastSettings.plcproj";
    }else
    {
        filename = ".emptySettings.plcproj";
    }
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        if(!lastSets && !emptySet)
        {
            std::cout << "Error: Cannot read file " << qPrintable(filename)
                      << ": " << qPrintable(file.errorString())
                      << std::endl;
        }else if(emptySet)
        {
            QMessageBox *msgBoxOpenError = new QMessageBox(this);
            msgBoxOpenError->setIcon(QMessageBox::Critical);
            msgBoxOpenError->setText("Reference file for empty project not found. Load a project or safe an empty"
                                     "connection set by hand using the connection settings dialog. It should be named \""
                                     + filename + "\"");
            msgBoxOpenError->show();
        }
        else return false;

    }
    // Setup the object
    xmlReader.setDevice(&file);
    openedConSlots.clear();
    ConSlot tempSlot;
    // parse the whole file to get the right entries
    while(!xmlReader.atEnd())
    {
        xmlReader.readNext();
        if(xmlReader.isStartElement() && xmlReader.name()=="IP_ADDR")
        {

            openedConSets->IP_Adr = xmlReader.readElementText();
            xmlReader.readNextStartElement();
            openedConSets->useProto = xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            openedConSets->localMPI = xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            openedConSets->plcMPI = xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            openedConSets->plc2MPI = xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            openedConSets->rack = xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            openedConSets->slot = xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            openedConSets->speed = xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
        }else if(xmlReader.isStartElement() &&
                 xmlReader.name().contains("SLOT_")){

            xmlReader.readNextStartElement();
            tempSlot.iAdrBereich=xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            tempSlot.iStartAdr=xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            tempSlot.iAnzFormat=xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            tempSlot.iBitnummer=xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            tempSlot.iDatenlaenge=xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            tempSlot.iDBnummer=xmlReader.readElementText().toInt();
            xmlReader.readNextStartElement();
            tempSlot.graphColor=QColor(xmlReader.readElementText());
            openedConSlots.append(tempSlot);
        }

    }
    // Small error msg
    if(xmlReader.hasError())
    {
        std::cout << "XML-READER Error: " <<
                     xmlReader.errorString().toStdString() << std::endl;
        QMessageBox *msgBoxOpenError = new QMessageBox(this);
        msgBoxOpenError->setIcon(QMessageBox::Critical);
        msgBoxOpenError->setText("Fehler beim oeffnen");
        msgBoxOpenError->show();

    }else
    {
        // Up date settings and slots
        emit newSlotsOpened(openedConSlots);
        emit newSettingsOpened(openedConSets);
    }

    file.close();
    return true;

}

void xmlSettingsHandler::saveProject(ConSets* currentConSets,
                                     QVector<ConSlot> currentConSlots,
                                     bool lastSets)
{

    if(!lastSets)
    {
        filename = QFileDialog::getSaveFileName(this,"Speichern",".",
                                                "PLCANALYZER-Projekte (*.plcproj)");
    }else
    {
        // create file and make it hidden
        filename = ".lastSettings.plcproj";
#ifdef BCCWIN
        // TODO: Set file hidden attribute
#endif
    }
    // Setup the file
    if(!filename.contains(".plcproj")) {filename.append(".plcproj");}
    QFile file(filename);
    file.open(QIODevice::WriteOnly);

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("DATA");
    xmlWriter.writeStartElement("SETTINGS");
    xmlWriter.writeTextElement("IP_ADDR", currentConSets->IP_Adr);
    xmlWriter.writeTextElement("PROTOCOL",
                               QString::number(currentConSets->useProto));
    xmlWriter.writeTextElement("LOCAL_MPI",
                               QString::number(currentConSets->localMPI));
    xmlWriter.writeTextElement("PLC_MPI",
                               QString::number(currentConSets->plcMPI));
    xmlWriter.writeTextElement("PLC_2_MPI",
                               QString::number(currentConSets->plc2MPI));
    xmlWriter.writeTextElement("RACK",
                               QString::number(currentConSets->rack));
    xmlWriter.writeTextElement("SLOT",
                               QString::number(currentConSets->slot));
    xmlWriter.writeTextElement("SPEED",
                               QString::number(currentConSets->speed));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("SLOTS");
    // For each slot fill the values
    for(int i=0;i<currentConSlots.size();++i)
    {
        QString slotDescriptor = "SLOT_" + QString::number(i);
        xmlWriter.writeStartElement(slotDescriptor);
        xmlWriter.writeTextElement("AREA",QString::number(
                                       currentConSlots[i].iAdrBereich));
        xmlWriter.writeTextElement("START_ADDRESS",QString::number(
                                       currentConSlots[i].iStartAdr));
        xmlWriter.writeTextElement("FORMAT",QString::number(
                                       currentConSlots[i].iAnzFormat));
        xmlWriter.writeTextElement("BIT",QString::number(
                                       currentConSlots[i].iBitnummer));
        xmlWriter.writeTextElement("LENGTH",QString::number(
                                       currentConSlots[i].iDatenlaenge));
        xmlWriter.writeTextElement("DB_NUMBER",QString::number(
                                       currentConSlots[i].iDBnummer));
        xmlWriter.writeTextElement("GRAPH_COLOR",currentConSlots[i].graphColor.name());

        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    file.close();
    std::cout << "File Saved!" << std::endl;

}

void xmlSettingsHandler::saveProjectAs()
{

}
