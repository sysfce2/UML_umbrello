/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "parmpropdlg.h"

// qt includes
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// kde includes
#include <klocale.h>
#include <kdebug.h>

// local includes
#include "../classifier.h"
#include "../umltemplatelist.h"
#include "../template.h"
#include "../umldoc.h"
#include "../dialog_utils.h"
#include "../object_factory.h"
#include "../stereotype.h"

#include "parmpropdlg.moc"

ParmPropDlg::ParmPropDlg(QWidget * parent, UMLDoc * doc, UMLAttribute * a)
        : KDialog(parent)
{
    setCaption( i18n("Parameter Properties") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    m_pUmldoc = doc;
    m_pAtt = a;
    QString type, text, name, initialValue;
    if(!a) {
        type = text = name = initialValue = "";
    } else {
        type = a -> getTypeName();
        name = a -> getName();
        initialValue = a -> getInitialValue();
        text = a -> getDoc();
    }
    int margin = fontMetrics().height();
    setMinimumSize(300, 400);
    //disableResize();
    QFrame *frame = new QFrame( this );
    setMainWidget( frame );
    QVBoxLayout * topLayout = new QVBoxLayout(frame);
    topLayout -> setSpacing(10);
    topLayout -> setMargin(margin);

    m_pParmGB = new Q3GroupBox(i18n("Properties"), frame);
    topLayout -> addWidget(m_pParmGB);

    QGridLayout * propLayout = new QGridLayout(m_pParmGB);
    propLayout -> setSpacing(10);
    propLayout -> setMargin(margin);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pParmGB);
    propLayout -> addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new KComboBox(m_pParmGB);
    propLayout -> addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField( m_pParmGB, propLayout, 1,
                                    m_pNameL, i18n("&Name:"),
                                    m_pNameLE, name );

    Dialog_Utils::makeLabeledEditField( m_pParmGB, propLayout, 2,
                                    m_pInitialL, i18n("&Initial value:"),
                                    m_pInitialLE, initialValue );

    m_pStereoTypeL = new QLabel( i18n("Stereotype name:"), m_pParmGB );
    propLayout -> addWidget(m_pStereoTypeL, 3, 0);
    m_pStereoTypeCB = new KComboBox(true, m_pParmGB );
    propLayout -> addWidget(m_pStereoTypeCB, 3, 1);

    m_pKind =  new Q3ButtonGroup(i18n("Passing Direction"), frame);
    m_pKind->setExclusive(true);
    m_pKind->setToolTip( i18n("\"in\" is a readonly parameter, \"out\" is a writeonly parameter and \"inout\" is a parameter for reading and writing."));

    QHBoxLayout * kindLayout = new QHBoxLayout( m_pKind );
    kindLayout->setMargin(margin);

    m_pIn =  new QRadioButton( "in", m_pKind );
    kindLayout->addWidget( m_pIn );

    m_pInOut =  new QRadioButton( "inout", m_pKind );
    kindLayout->addWidget( m_pInOut );

    m_pOut =  new QRadioButton( "out", m_pKind );
    kindLayout->addWidget( m_pOut );

    topLayout -> addWidget(m_pKind);

    m_pDocGB = new Q3GroupBox(i18n("Documentation"), frame);
    QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
    docLayout -> setMargin(margin);

    m_pDoc = new Q3MultiLineEdit(m_pDocGB);
    ///////////
    m_pDoc->setWordWrap(Q3MultiLineEdit::WidgetWidth);
    //////////
    m_pDoc -> setText(text);
    docLayout -> addWidget(m_pDoc);
    topLayout -> addWidget(m_pDocGB);

    // Check the proper Kind radiobutton.
    if (a) {
        Uml::Parameter_Direction kind = a->getParmKind();
        if (kind == Uml::pd_Out)
            m_pOut->setChecked(true);
        else if (kind == Uml::pd_InOut)
            m_pInOut->setChecked(true);
        else
            m_pIn->setChecked(true);
    } else
        m_pIn->setChecked(true);

    m_pTypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
    m_pTypeCB->setEditable(true);
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
//    m_pTypeCB->setAutoCompletion(false);

    //add template parameters
    UMLClassifier *pConcept = dynamic_cast<UMLClassifier*>( m_pAtt->parent()->parent() );
    if (pConcept == NULL) {
        uError() << "ParmPropDlg: grandparent of " << m_pAtt->getName()
        << " is not a UMLClassifier" << endl;
    } else {
        UMLTemplateList tmplParams( pConcept->getTemplateList() );
        foreach( UMLTemplate* t, tmplParams ) {
            insertType( t->getName() );
        }
    }
    //now add the Concepts
    UMLClassifierList namesList( m_pUmldoc->getConcepts() );
    foreach(UMLClassifier* obj, namesList ) {
       insertType( obj->getFullyQualifiedName() );
    }

    //work out which one to select
    int typeBoxCount = 0;
    bool foundType = false;
    while (typeBoxCount < m_pTypeCB->count() && foundType == false) {
        QString typeBoxString = m_pTypeCB->itemText(typeBoxCount);
        if ( typeBoxString == type ) { //getTypeName()
            foundType = true;
            m_pTypeCB->setCurrentIndex(typeBoxCount);
        } else {
            typeBoxCount++;
        }
    }

    if (!foundType) {
        insertType( type, 0 );
        m_pTypeCB->setCurrentIndex(0);
    }

    // manage stereotypes
    m_pStereoTypeCB->setDuplicatesEnabled(false); //only allow one of each type in box
    m_pStereoTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    insertStereotype (QString("")); // an empty stereotype is the default
    int defaultStereotype=0;
    bool foundDefaultStereotype = false;
    foreach (UMLStereotype* currentSt, m_pUmldoc->getStereotypes() ) {
        if (!foundDefaultStereotype) {
            if ( m_pAtt->getStereotype() == currentSt->getName()) {
                foundDefaultStereotype = true;
            }
            defaultStereotype++;
        }
        insertStereotype (currentSt->getName());
    }
    // lookup for a default stereotype, if the operation doesn't have one
    if (foundDefaultStereotype)
        m_pStereoTypeCB->setCurrentIndex(defaultStereotype);
    else
        m_pStereoTypeCB->setCurrentIndex(-1);

    // set tab order
    setTabOrder(m_pKind, m_pTypeCB);
    setTabOrder(m_pTypeCB, m_pNameLE);
    setTabOrder(m_pNameLE, m_pInitialLE);
    setTabOrder(m_pInitialLE, m_pStereoTypeCB);
    setTabOrder(m_pStereoTypeCB, m_pIn);
    setTabOrder(m_pIn, m_pDoc);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    m_pNameLE->setFocus();
}

void ParmPropDlg::insertType( const QString& type, int index )
{
    m_pTypeCB->insertItem( index, type );
    m_pTypeCB->completionObject()->addItem( type );
}

void ParmPropDlg::insertStereotype( const QString& type, int index )
{
    m_pStereoTypeCB->insertItem( index, type );
    m_pStereoTypeCB->completionObject()->addItem( type );
}

Uml::Parameter_Direction ParmPropDlg::getParmKind() {
    Uml::Parameter_Direction pk = Uml::pd_In;
    if (m_pOut->isChecked())
        pk = Uml::pd_Out;
    else if (m_pInOut->isChecked())
        pk = Uml::pd_InOut;
    return pk;
}

void ParmPropDlg::slotOk() {
    if (m_pAtt != NULL) {
        m_pAtt->setParmKind( getParmKind() );
        m_pAtt->setStereotype( m_pStereoTypeCB->currentText() );
        QString typeName = m_pTypeCB->currentText();
        UMLClassifier * pConcept = dynamic_cast<UMLClassifier*>( m_pAtt->parent()->parent() );
        if (pConcept == NULL) {
            uError() << "grandparent of " << m_pAtt->getName() << " is not a UMLClassifier" << endl;
        } else {
            UMLTemplate *tmplParam = pConcept->findTemplate(typeName);
            if (tmplParam) {
                m_pAtt->setType(tmplParam);
                accept();
                return;
            }
        }
        UMLClassifierList namesList( m_pUmldoc->getConcepts() );
        UMLClassifier * obj = NULL;
        foreach ( obj, namesList) {
            if (obj->getFullyQualifiedName() == typeName) {
                m_pAtt->setType( obj );
                break;
            }
        }
        if (obj == NULL) {
            // Nothing found: Create a new type on the fly.
            // @todo There should be an extra dialog to decide whether to
            // create a datatype or a class. For now, we create a class.
            uDebug() << "" << typeName << " not found."
                << " Creating a new class for the type." << endl;
            UMLObject *o = Object_Factory::createUMLObject(Uml::ot_Class, typeName);
            m_pAtt->setType(o);
        }

    }
    accept();
}

ParmPropDlg::~ParmPropDlg() {}

