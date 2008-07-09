/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006-2008                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "classimport.h"

// qt/kde includes
#include <QtCore/QRegExp>
#include <klocale.h>

// app includes
#include "umldoc.h"
#include "uml.h"
#include "idlimport.h"
#include "pythonimport.h"
#include "javaimport.h"
#include "adaimport.h"
#include "pascalimport.h"
#ifndef DISABLE_CPP_IMPORT
#include "cppimport.h"
#endif

void ClassImport::importFiles(const QStringList &fileList)
{
    initialize();
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    uint processedFilesCount = 0;
    for (QStringList::const_iterator fileIT = fileList.begin();
            fileIT != fileList.end(); ++fileIT) {
        QString fileName = (*fileIT);
        umldoc->writeToStatusBar(i18n("Importing file: %1 Progress: %2/%3",
                                 fileName, processedFilesCount, fileList.size()));
        parseFile(fileName);
        processedFilesCount++;
    }
    umldoc->writeToStatusBar(i18nc("ready to status bar", "Ready."));
}

ClassImport *ClassImport::createImporterByFileExt(const QString &filename) 
{
    ClassImport *classImporter;
    if (filename.endsWith(".idl"))
        classImporter = new IDLImport();
    else if (filename.endsWith(".py"))
        classImporter = new PythonImport();
    else if (filename.endsWith(".java"))
        classImporter = new JavaImport();
    else if (filename.contains( QRegExp("\\.ad[sba]$") ))
        classImporter = new AdaImport();
    else if (filename.endsWith(".pas"))
        classImporter = new PascalImport();
#ifndef DISABLE_CPP_IMPORT
    else
        classImporter = new CppImport();  // the default.
#else
    else 
        classImporter = 0;
#endif        
    return classImporter;
}

