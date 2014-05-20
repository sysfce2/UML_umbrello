/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CLASSPROPDIALOG_H
#define CLASSPROPDIALOG_H

#include "dialogbase.h"

class ClassAssociationsPage;
class ClassGeneralPage;
class ClassifierListPage;
class ClassOptionsPage;
class ConstraintListPage;
class KFontChooser;
class PkgContentsPage;
class ObjectWidget;
class UMLDoc;
class UMLObject;
class UMLWidget;
class UMLWidgetStylePage;

/**
 * @author Paul Hensgen <phensgen@techie.com>
 * @version 1.0
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ClassPropDialog : public DialogBase
{
    Q_OBJECT
public:
    ClassPropDialog(QWidget *parent, UMLObject *c, bool assoc = false);
    ClassPropDialog(QWidget *parent, ObjectWidget * o);
    ClassPropDialog(QWidget *parent, UMLWidget * o);

    ~ClassPropDialog();

    enum Page{page_gen = 0, page_att, page_op, page_entatt, page_constraint, page_template,
              page_assoc, page_options, page_color, page_font};

    virtual void apply();

protected slots:
    void slotOk();
    void slotApply();

protected:
    void setupPages(bool assoc = false);
    void setupGeneralPage();
    void setupStylePage();
    void setupDisplayPage();
    void setupAttributesPage();
    void setupOperationsPage();
    void setupTemplatesPage();
    void setupEnumLiteralsPage();
    void setupEntityAttributesPage();
    void setupEntityConstraintsPage();
    void setupContentsPage();
    void setupAssociationsPage();
    void setupInstancePages();
    void setupFontPage();

private:
    KFontChooser*        m_pChooser;
    ClassGeneralPage*    m_pGenPage;
    ClassifierListPage*  m_pAttPage;
    ClassifierListPage*  m_pOpsPage;
    ClassifierListPage*  m_pTemplatePage;
    ClassifierListPage*  m_pEnumLiteralPage;
    ClassifierListPage*  m_pEntityAttributePage;
    ConstraintListPage*  m_pEntityConstraintPage;
    PkgContentsPage*     m_pPkgContentsPage;
    ClassAssociationsPage* m_pAssocPage;
    ClassOptionsPage*    m_pOptionsPage;
    UMLWidgetStylePage*  m_pStylePage;

    UMLDoc*              m_doc;
    UMLObject*           m_pObject;
    UMLWidget*           m_pWidget;

    enum Page_Type {
        pt_Object = 1,    // Show General page + Assoc. page if Class i.e. no colors page
        pt_ObjectWidget,  // Shows pages needed for an ObjectWidget
        pt_Widget         // Shows pages needed for any other widget
    };

    Page_Type m_Type;

private:
    void init();

};

#endif
