/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_set_visibility.h"

// app includes
#include "umlobject.h"

#include <klocale.h>

namespace Uml
{

    CmdSetVisibility::CmdSetVisibility(UMLObject * obj, Uml::Visibility visibility)
      : m_visibility(visibility), m_umlObject(obj)
    {
        setText(i18n("Change visibility : ") + obj->getName());
        m_oldVisibility = obj->getVisibility();
    }

    CmdSetVisibility::~CmdSetVisibility()
    {
    }

    void CmdSetVisibility::redo()
    {
        m_umlObject->setVisibilitycmd(m_visibility);
    }

    void CmdSetVisibility::undo()
    {
        m_umlObject->setVisibilitycmd(m_oldVisibility);
    }

}
