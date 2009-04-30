/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_changeFontSelection.h"

// app includes
#include "umldoc.h"
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeFontSelection::CmdChangeFontSelection(UMLDoc* doc, UMLScene* scene, QFont fon)
    {
        // [PORT]
        UMLWidget * widget = scene->getFirstMultiSelectedWidget();
        setText(i18n("Change Font")+ " : " + widget->name());
        m_doc = doc;
        m_scene = scene;
        m_newFont = fon;
        m_oldFont = widget->font();
    }

    void CmdChangeFontSelection::undo()
    {
        // [PORT]
        m_scene->selectionSetFont( m_oldFont );
        m_doc->setModified(true);
    }

    void CmdChangeFontSelection::redo()
    {
        // [PORT]
        m_scene->selectionSetFont( m_newFont );
        m_doc->setModified(true);
    }

}
