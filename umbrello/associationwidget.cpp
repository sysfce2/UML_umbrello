/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>

#include "activitywidget.h"
#include "umlview.h"
#include "umlwidget.h"
#include "listpopupmenu.h"
#include "association.h"
#include "associationwidget.h"
#include "associationwidgetdata.h"
#include "assocrules.h"
#include "floatingtext.h"
#include "floatingtextdata.h"
#include "objectwidget.h"
#include "dialogs/assocpropdlg.h"
#include "dialogs/selectopdlg.h"

#include <kdebug.h>
#include <klineeditdlg.h>
#include <klocale.h>

AssociationWidget::AssociationWidget(QWidget *parent, AssociationWidgetData& aData)
 	: QObject(parent), AssociationWidgetData(aData)
{

 	init(parent);

  	connect(m_pView, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
  	connect(m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );

}

AssociationWidget::AssociationWidget(QWidget *parent, UMLWidget* WidgetA,
				     Association_Type Type, UMLWidget* WidgetB)
 	: QObject(parent)
{

 	init(parent);

 	setWidgetA(WidgetA);
 	setWidgetB(WidgetB);

  	setAssocType(Type);

  	calculateEndingPoints();

  	//The AssociationWidget is set to Activated because it already has its side widgets
  	//and the AssociationWidgetData is brand new
  	setActivated(true);

 	synchronizeData(); // for sync of child text widgets

 	// sync UML meta-data to settings in associationwidgetdata
 	// this _should_ be done _after_ synchronizeData or some info will be lost
 	// (the widget id's for one, there might be others)
 	mergeAssociationDataIntoUMLRepresentation();

 	connect( m_pView, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
  	connect( m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );

 	//collaboration messages need a name label because it's that
  	//which handles the right click menu options
  	if (m_AssocType == at_Coll_Message) {
 		setName("");
 		setNameData( dynamic_cast<FloatingTextData*>(getNameWidget()->getData()) );
 		m_pName->setUMLObject( m_pName->getUMLObject() );
  	}

}

AssociationWidget::~AssociationWidget() {
 	// CHECK : associations removed from umldocument
}

AssociationWidget& AssociationWidget::operator=(AssociationWidget & /*Other*/) {
  	//to be implemented
	return *this;
}

bool AssociationWidget::operator==(AssociationWidget & Other) {
	return ((AssociationWidgetData&) *this == (AssociationWidgetData&) Other);
}

bool AssociationWidget::operator!=(AssociationWidget & Other) {
	return !(*this == Other);
}

/** Read property of FloatingText* m_pMultiA. */
FloatingText* AssociationWidget::getMultiAWidget() {
	return m_pMultiA;
}

/** Returns the m_pMultiA's text. */
QString AssociationWidget::getMultiA()
{
	if(m_pMultiA) {
		return m_pMultiA->getText();
	} else {
		return "";
	}
}

/** Read property of FloatingText* m_pMultiB. */
FloatingText* AssociationWidget::getMultiBWidget() {
	return m_pMultiB;
}

/** Returns the m_pMultiB's text. */
QString AssociationWidget::getMultiB() {
	if(m_pMultiB)
	{
		return m_pMultiB->getText();
	}
	return "";
}

FloatingText* AssociationWidget::getNameWidget()
{
	return m_pName;
}

QString AssociationWidget::getName() {
	if(m_pName) {
		return m_pName->getText();
	} else {
		return "";
	}
}

QString AssociationWidget::getDoc() {
	return m_pAssociation->getDoc();
}

FloatingText* AssociationWidget::getRoleAWidget()
{
	return m_pRoleA;
}

FloatingText* AssociationWidget::getRoleBWidget()
{
	return m_pRoleB;
}

FloatingText* AssociationWidget::getChangeWidgetA()
{
	return m_pChangeWidgetA;
}

FloatingText* AssociationWidget::getChangeWidgetB()
{
	return m_pChangeWidgetB;
}

QString AssociationWidget::getRoleNameA() {
	if(m_pRoleA) {
		return m_pRoleA->getText();
	} else {
		return "";
	}
}

QString AssociationWidget::getRoleADoc() {
	return m_pAssociation->getRoleADoc();
}

QString AssociationWidget::getRoleNameB() {
	if(m_pRoleB) {
		return m_pRoleB->getText();
	} else {
		return "";
	}
}

QString AssociationWidget::getRoleBDoc() {
	return m_pAssociation->getRoleBDoc();
}

void AssociationWidget::setName(QString strName) {
	bool newLabel = false;
        if(!m_pName) {
		newLabel = true;
                m_pName = new FloatingText(m_pView, CalculateNameType(tr_Name), strName);
                m_pName->setAssoc(this);
        } else {
		if (m_pName->getText() == "") {
			newLabel = true;
		}
                m_pName->setText(strName);
        }
        m_pName->setActivated();

	// set attribute of UMLAssociation associated with this associationwidget
	m_pAssociation->setName(strName);

	if (newLabel) {
		setTextPosition( tr_Name, calculateTextPosition(tr_Name) );
	}

        if(FloatingText::isTextValid(m_pName->getText()))
                m_pName -> show();
        else
                m_pName -> hide();
        synchronizeData();
}

bool AssociationWidget::setMultiA(QString strMultiA) {
	bool newLabel = false;

	Association_Type type = getAssocType();
	//if the association is not supposed to have a Multiplicity FloatingText
	if( !AssocRules::allowMultiplicity( type, getWidgetA() -> getBaseType() ) )
		return false;
	if(!m_pMultiA) {
		newLabel = true;
		m_pMultiA = new FloatingText(m_pView, tr_MultiA, strMultiA);
		m_pMultiA->setAssoc(this);
	} else {
		if (m_pMultiA->getText() == "") {
			newLabel = true;
		}
		m_pMultiA->setText(strMultiA);
	}

	// set attribute of UMLAssociation associated with this associationwidget
	m_pAssociation->setMultiA(strMultiA);

	m_pMultiA->setActivated();

	if (newLabel) {
		setTextPosition( tr_MultiA, calculateTextPosition(tr_MultiA) );
	}

	if(FloatingText::isTextValid(m_pMultiA->getText()))
		m_pMultiA -> show();
	else
		m_pMultiA -> hide();
	synchronizeData();
	return true;
}

bool AssociationWidget::setMultiB(QString strMultiB) {
	bool newLabel = false;
	Association_Type type = getAssocType();
	//if the association is not supposed to have a Multiplicity FloatingText
	if( !AssocRules::allowMultiplicity( type, getWidgetB() -> getBaseType() ) )
		return false;

	if(!m_pMultiB) {
		newLabel = true;
		m_pMultiB = new FloatingText(m_pView, tr_MultiB, strMultiB);
		m_pMultiB->setAssoc(this);
	} else {
		if (m_pMultiB->getText() == "") {
			newLabel = true;
		}
		m_pMultiB->setText(strMultiB);
	}

	// set attribute of UMLAssociation associated with this associationwidget
	m_pAssociation->setMultiB(strMultiB);

	m_pMultiB->setActivated();

	if (newLabel) {
		setTextPosition( tr_MultiB, calculateTextPosition(tr_MultiB) );
	}

	if(FloatingText::isTextValid(m_pMultiB->getText()))
		m_pMultiB -> show();
	else
		m_pMultiB -> hide();
	synchronizeData();
	return true;
}


bool AssociationWidget::setRoleNameA (QString strRole) {
	bool newLabel = false;
	Association_Type type = getAssocType();
	//if the association is not supposed to have a Role FloatingText
	if( !AssocRules::allowRole( type ) )
		return false;

	if(!m_pRoleA) {
		newLabel = true;
		m_pRoleA = new FloatingText(m_pView, tr_RoleAName, strRole);
		m_pRoleA->setAssoc(this);
		m_pRoleA->setPreText(UMLAssociation::ScopeToString(getVisibilityA()));
	} else {
		if (m_pRoleA->getText() == "") {
			newLabel = true;
		}
		m_pRoleA->setText(strRole);
	}

	// set attribute of UMLAssociation associated with this associationwidget
	m_pAssociation->setRoleNameA(strRole);

	m_pRoleA->setActivated();

	if (newLabel) {
		setTextPosition( tr_RoleAName, calculateTextPosition(tr_RoleAName) );
	}

	if(FloatingText::isTextValid(m_pRoleA->getText()))
		m_pRoleA -> show();
	else
		m_pRoleA -> hide();
	synchronizeData();
	return true;
}

void AssociationWidget::setDoc (QString doc) {
	m_pAssociation->setDoc(doc);
	AssociationWidgetData::setDoc(doc);
}

void AssociationWidget::setRoleADoc (QString doc) {
	m_pAssociation->setRoleADoc(doc);
	AssociationWidgetData::setRoleADoc(doc);
}

bool AssociationWidget::setRoleNameB(QString strRole) {
	bool newLabel = false;
        Association_Type type = getAssocType();
        //if the association is not supposed to have a Role FloatingText
        if( !AssocRules::allowRole( type ) )
                return false;
        if(!m_pRoleB) {
		newLabel = true;
                m_pRoleB = new FloatingText(m_pView, tr_RoleBName, strRole);
                m_pRoleB->setAssoc(this);
		m_pRoleB->setPreText(UMLAssociation::ScopeToString(getVisibilityB()));
        } else {
		if (m_pRoleB->getText() == "") {
			newLabel = true;
		}
                m_pRoleB->setText(strRole);
        }

	// set attribute of UMLAssociation associated with this associationwidget
	m_pAssociation->setRoleNameB(strRole);

        m_pRoleB->setActivated();

	if (newLabel) {
		setTextPosition( tr_RoleBName, calculateTextPosition(tr_RoleBName) );
	}

        if(FloatingText::isTextValid(m_pRoleB->getText()))
                m_pRoleB -> show();
        else
                m_pRoleB -> hide();
        synchronizeData();
        return true;
}

void AssociationWidget::setRoleBDoc (QString doc) {
	m_pAssociation->setRoleBDoc(doc);
	AssociationWidgetData::setRoleBDoc(doc);
}

Scope AssociationWidget::getVisibilityA ( )
{
	// simply get it from our data object
	return AssociationWidgetData::getVisibilityA();
}

void AssociationWidget::setVisibilityA (Scope value)
{
	Scope old = getVisibilityA();
	if (value != old) {
		QString scopeString = UMLAssociation::ScopeToString(value);
		// update our attribute
		AssociationWidgetData::setVisibilityA(value);
		m_pAssociation->setVisibilityA(value);
		// update RoleA pre-text attribute as appropriate
		getRoleAWidget()->setPreText(scopeString);
        	synchronizeData(); // because FloatingText of RoleA is updated
		// this is a terrible method, btw
	}
}

Scope AssociationWidget::getVisibilityB ( )
{
	// simply get it from our data object
	return AssociationWidgetData::getVisibilityB();
}

void AssociationWidget::setVisibilityB (Scope value)
{
	Scope old = getVisibilityB();
	if (value != old) {
		QString scopeString = UMLAssociation::ScopeToString(value);
		// update our attribute
		AssociationWidgetData::setVisibilityB(value);
		m_pAssociation->setVisibilityB(value);
		// update RoleB pre-text attribute as appropriate
		getRoleBWidget()->setPreText(scopeString);
        	synchronizeData(); // because FloatingText of RoleB is updated
		// this is a terrible method, btw
	}
}

Changeability_Type AssociationWidget::getChangeabilityA( )
{
	return AssociationWidgetData::getChangeabilityA();
}

void AssociationWidget::setChangeabilityA (Changeability_Type value)
{
	Changeability_Type old = getChangeabilityA();
	if (value != old) {
		QString changeString = UMLAssociation::ChangeabilityToString(value);
		// update our attribute
		AssociationWidgetData::setChangeabilityA(value);
		m_pAssociation->setChangeabilityA(value);

		// update our string representation
		setChangeWidgetA(changeString);
	}
}

// should be _private_ so that we dont goof up sync between UMLAssociation and
// AssociationWidgetData objects owned by this associationwidget
bool AssociationWidget::setChangeWidgetA(QString strChangeWidgetA) {
	bool newLabel = false;

        if(!m_pChangeWidgetA) {
		newLabel = true;
                m_pChangeWidgetA = new FloatingText(m_pView, tr_ChangeA, strChangeWidgetA);
                m_pChangeWidgetA->setAssoc(this);
		m_pChangeWidgetA->setPreText("{"); // all types have this
		m_pChangeWidgetA->setPostText("}"); // all types have this
        } else {
		if (m_pChangeWidgetA->getText() == "") {
			newLabel = true;
		}
                m_pChangeWidgetA->setText(strChangeWidgetA);
        }
        m_pChangeWidgetA->setActivated();

	if (newLabel) {
		setTextPosition( tr_ChangeA, calculateTextPosition( tr_ChangeA ) );
	}

        if(FloatingText::isTextValid(m_pChangeWidgetA->getText()))
                m_pChangeWidgetA -> show();
        else
                m_pChangeWidgetA -> hide();

        synchronizeData();

        return true;
}

Changeability_Type AssociationWidget::getChangeabilityB( )
{
	return AssociationWidgetData::getChangeabilityB();
}

void AssociationWidget::setChangeabilityB(Changeability_Type value) {
	Changeability_Type old = getChangeabilityB();
	if (value != old) {
		QString changeString = UMLAssociation::ChangeabilityToString(value);
		// update our attribute
		AssociationWidgetData::setChangeabilityB(value);
		m_pAssociation->setChangeabilityB(value);
		// update our string representation
		setChangeWidgetB(changeString);
	}

}

bool AssociationWidget::setChangeWidgetB(QString strChangeWidgetB) {
	bool newLabel = false;

        if(!m_pChangeWidgetB) {
		newLabel = true;
                m_pChangeWidgetB = new FloatingText(m_pView, tr_ChangeB, strChangeWidgetB);
                m_pChangeWidgetB->setAssoc(this);
		m_pChangeWidgetB->setPreText("{"); // all types have this
		m_pChangeWidgetB->setPostText("}"); // all types have this
        } else {
		if (m_pChangeWidgetB->getText() == "") {
			newLabel = true;
		}
                m_pChangeWidgetB->setText(strChangeWidgetB);
        }
        m_pChangeWidgetB->setActivated();

	if (newLabel) {
		setTextPosition( tr_ChangeB, calculateTextPosition( tr_ChangeB ) );
	}

        if(FloatingText::isTextValid(m_pChangeWidgetB->getText()))
                m_pChangeWidgetB -> show();
        else
                m_pChangeWidgetB -> hide();

        synchronizeData();
        return true;
}


bool AssociationWidget::activate() {

	if(isActivated())
		return true;

	bool status = true;
	Association_Type type = getAssocType();

	setWidgetA(m_pView->findWidget(getWidgetAID()));
	setWidgetB(m_pView->findWidget(getWidgetBID()));

	if(!m_pWidgetA || !m_pWidgetB) {
		kdDebug() << "Can't make association" << endl;
		return false;
	}

	m_LinePath.activate();
	calculateEndingPoints();
	FloatingText* ft = 0;
	/*
	  There used to be calls to to setRole( ), setMultiA, setMultiB here.  But I
	  have removed them and cut and pasted most of that code here - why??
	  There was a call to SynchronizeData in each of them which deleted the
	  data we needed.  The other way is to add a loading variable to miss that call
	  but just as easy to put the code we need here.
	*/
	if( getRoleAData() && AssocRules::allowRole( type ) ) {
		ft = new FloatingText(m_pView ,const_cast<FloatingTextData*>(getRoleAData()));
		if(m_pRoleA) {
			m_pView->removeWidget(m_pRoleA);
		}
		m_pRoleA = ft;
		m_pRoleA->setAssoc(this);
		m_pRoleA->setRole( tr_RoleAName) ;
		m_pRoleA->setPreText(UMLAssociation::ScopeToString(getVisibilityA()));
		if(FloatingText::isTextValid(m_pRoleA->getText()) ) {
			m_pRoleA -> show();
		} else {
			m_pRoleA -> hide();
		}

		if( m_pView->getType() == dt_Collaboration && m_pRoleA) {
			m_pRoleA->setUMLObject(m_pWidgetB->getUMLObject());
		}

		m_pRoleA->activate();
	}

        if( getRoleBData() && AssocRules::allowRole( type ) ) {
                ft = new FloatingText(m_pView ,const_cast<FloatingTextData*>(getRoleBData()));
                if(m_pRoleB) {
                        m_pView->removeWidget(m_pRoleB);
                }
                m_pRoleB = ft;
                m_pRoleB->setAssoc(this);
                m_pRoleB->setRole( tr_RoleBName) ;
		m_pRoleB->setPreText(UMLAssociation::ScopeToString(getVisibilityB()));
                if(FloatingText::isTextValid(m_pRoleB->getText()) ) {
                        m_pRoleB -> show();
                } else {
                        m_pRoleB -> hide();
                }

                if( m_pView->getType() == dt_Collaboration && m_pRoleB) {
                        m_pRoleB->setUMLObject(m_pWidgetB->getUMLObject());
                }
		m_pRoleB->activate();
        }

        if( getNameData() ) {
		ft = new FloatingText( m_pView, const_cast<FloatingTextData*>(getNameData()) );
		m_pName = ft;
		m_pName->setAssoc(this);
                m_pName->setRole( CalculateNameType(tr_Name) );

		if ( FloatingText::isTextValid(m_pName->getText()) ) {
			m_pName-> show();
		} else {
			m_pName-> hide();
		}

		m_pName->activate();
		calculateNameTextSegment();
	}

	if(getMultiDataA()) {
		if( AssocRules::allowMultiplicity( type, getWidgetA() -> getBaseType() ) ) {
			ft = new FloatingText( m_pView, const_cast<FloatingTextData*>(getMultiDataA()) );
			if(m_pMultiA) {
				m_pView->removeWidget(m_pMultiA);
			}
			m_pMultiA = ft;
			m_pMultiA->setAssoc(this);

			m_pMultiA->setRole(tr_MultiA);
			if ( FloatingText::isTextValid(m_pMultiA->getText()) ) {
				m_pMultiA -> show();
			} else {
				m_pMultiA -> hide();
			}
			m_pMultiA->activate();
		}//end else
	}//end if data
	if(getMultiDataB()) {
		if( AssocRules::allowMultiplicity( type, getWidgetA() -> getBaseType() ) ) {
			ft = new FloatingText(m_pView, const_cast<FloatingTextData*>(getMultiDataB()));
			if(m_pMultiB) {
				m_pView->removeWidget(m_pMultiB);
			}
			m_pMultiB = ft;
			m_pMultiB->setAssoc(this);
			m_pMultiB->setRole( tr_MultiB );
			if (FloatingText::isTextValid(m_pMultiB->getText()) ) {
				m_pMultiB -> show();
			} else {
				m_pMultiB -> hide();
			}
			m_pMultiB->activate();
		}//end else
	}//end if data

	if( getChangeDataA() ) {
		ft = new FloatingText(m_pView, const_cast<FloatingTextData*>(getChangeDataA()));
		if(m_pChangeWidgetA) {
			m_pView->removeWidget(m_pChangeWidgetA);
		}
		m_pChangeWidgetA = ft;
		m_pChangeWidgetA->setAssoc(this);
		m_pChangeWidgetA->setRole( tr_ChangeA );
		if (FloatingText::isTextValid(m_pChangeWidgetA->getText()) ) {
			m_pChangeWidgetA -> show();
		} else {
			m_pChangeWidgetA -> hide ();
		}
		m_pChangeWidgetA->activate();
	}

	if( getChangeDataB() ) {
		ft = new FloatingText(m_pView, const_cast<FloatingTextData*>(getChangeDataB()));
		if(m_pChangeWidgetB) {
			m_pView->removeWidget(m_pChangeWidgetB);
		}
		m_pChangeWidgetB = ft;
		m_pChangeWidgetB->setAssoc(this);
		m_pChangeWidgetB->setRole( tr_ChangeB );
		if (FloatingText::isTextValid(m_pChangeWidgetB->getText()) ) {
			m_pChangeWidgetB -> show();
		} else {
			m_pChangeWidgetB -> hide ();
		}
		m_pChangeWidgetB->activate();
	}

	// CHECK
	// m_LinePath.setAssocType( m_AssocType );
	if(status) {
		m_bActivated = true;
	}
	return status;
}

/** This function calculates which role should be set for the m_pName FloatingText */
Uml::Text_Role AssociationWidget::CalculateNameType(Text_Role defaultRole) {

	Text_Role result = defaultRole;
	if( m_pView -> getType() == dt_Collaboration ) {
		if(m_pWidgetA == m_pWidgetB) {
			result = tr_Coll_Message;//for now same as other Coll_Message
		} else {
			result = tr_Coll_Message;
		}
	} else if( m_pView -> getType() == dt_Sequence ) {
		if(m_pWidgetA == m_pWidgetB) {
			result = tr_Seq_Message_Self;
		} else {
			result = tr_Seq_Message;
		}
	}

	return result;
}

UMLWidget* AssociationWidget::getWidgetA() {
	return m_pWidgetA;
}

UMLWidget* AssociationWidget::getWidgetB() {
	return m_pWidgetB;
}


bool AssociationWidget::setWidgets( UMLWidget* WidgetA,
				    Association_Type AssocType, UMLWidget* WidgetB) {
	//if the association already has a WidgetB or WidgetA associated, then
	//it cannot be changed to other widget, that would require a  deletion
	//of the association and the creation of a new one
	if((m_pWidgetA && (m_pWidgetA != WidgetA)) || (m_pWidgetB && (m_pWidgetB != WidgetB))) {
		return false;
	}
	setWidgetA(WidgetA);
	setAssocType(AssocType);
	setWidgetB(WidgetB);

	calculateEndingPoints();
	synchronizeData();
	return true;
}

// synchronize associationwidget floatingtext children with
// their floatingtextdata held by the associationwidgetdata child
// object
void AssociationWidget::synchronizeData() {

	if(m_pWidgetA) {
		if( m_pWidgetA -> getBaseType() == wt_Object )
			setWidgetAID( ( (ObjectWidget *) m_pWidgetA ) -> getLocalID() );
		else
			setWidgetAID( m_pWidgetA->getID() );
	} else {
		setWidgetAID(-1);
	}

	if(m_pWidgetB) {
		if( m_pWidgetB -> getBaseType() == wt_Object )
			setWidgetBID( ( (ObjectWidget *) m_pWidgetB ) -> getLocalID() );
		else
			setWidgetBID( m_pWidgetB->getID() );
  	} else {
  		setWidgetBID(-1);
  	}

 	if(m_pName) {
 		// Note: The FloatingText::getData() call already synchronizes
 		// the FloatingText with its FloatingTextData
 		// Addendum: NO this isnt true, the synchData() is commented out
 		// thus we should do the following below for each floattext widget (-b.t.)
 		setNameData(dynamic_cast<FloatingTextData*>(m_pName->getData()));
 	} else {
 		setNameData(0);
 	}

 	if(m_pRoleA) {
 		setRoleAData(dynamic_cast<FloatingTextData*>(m_pRoleA->getData()));
 	} else {
 		setRoleAData(0);
 	}

 	if(m_pRoleB) {
 		setRoleBData(dynamic_cast<FloatingTextData*>(m_pRoleB->getData()));
  	} else {
 		setRoleBData(0);
  	}

  	if(m_pMultiA) {
  		setMultiDataA(dynamic_cast<FloatingTextData*>(m_pMultiA->getData()));
  	} else {
  		setMultiDataA(0);
  	}

 	if(m_pChangeWidgetA) {
 		setChangeDataA(dynamic_cast<FloatingTextData*>(m_pChangeWidgetA->getData()));
 	} else {
 		setChangeDataA(0);
 	}

 	if(m_pChangeWidgetB) {
 		setChangeDataB(dynamic_cast<FloatingTextData*>(m_pChangeWidgetB->getData()));
 	} else {
 		setChangeDataB(0);
 	}

  	if(m_pMultiB) {
  		setMultiDataB(dynamic_cast<FloatingTextData*>(m_pMultiB->getData()));
  	} else {
  		setMultiDataB(0);
	}
}

/** Returns true if this association associates WidgetA to WidgetB, otherwise it returns
    false */
bool AssociationWidget::checkAssoc(UMLWidget * WidgetA, UMLWidget *WidgetB) {
	return (WidgetA == m_pWidgetA && WidgetB == m_pWidgetB);
}

/** CleansUp all the association's data in the related widgets  */
void AssociationWidget::cleanup() {

	//let any other associations know we are going so they can tidy their positions up
	if(m_nTotalCountA > 2)
		updateAssociations(m_nTotalCountA - 1, m_WidgetARegion, true);
	if(m_nTotalCountB > 2)
		updateAssociations(m_nTotalCountB - 1, m_WidgetBRegion, false);

	if(m_pWidgetA) {
		m_pWidgetA->removeAssoc(this);
		m_pWidgetA = 0;
	}
	if(m_pWidgetB) {
		m_pWidgetB->removeAssoc(this);
		m_pWidgetB = 0;
	}

	if(m_pName) {
		m_pView->removeWidget(m_pName);
		m_pName = 0;
	}

	if(m_pRoleA) {
		m_pView->removeWidget(m_pRoleA);
		m_pRoleA = 0;
	}
	if(m_pRoleB) {
		m_pView->removeWidget(m_pRoleB);
		m_pRoleB = 0;
	}
	if(m_pMultiA) {
		m_pView->removeWidget(m_pMultiA);
		m_pMultiA = 0;
	}
	if(m_pMultiB) {
		m_pView->removeWidget(m_pMultiB);
		m_pMultiB = 0;
	}
	if(m_pChangeWidgetA) {
		m_pView->removeWidget(m_pChangeWidgetA);
		m_pChangeWidgetA = 0;
	}
	if(m_pChangeWidgetB) {
		m_pView->removeWidget(m_pChangeWidgetB);
		m_pChangeWidgetB = 0;
	}

	// We should to remove the UMLAssociation from the document
	if (m_pAssociation) {
		m_pView->getDocument()->removeAssociation(m_pAssociation);
		// delete m_pAssociation; // no.. its really owned by umldoc
	        m_pAssociation = 0;
	}

	// Why bother syncing data you are about to delete?!? -b.t.
	// synchronizeData();

	AssociationWidgetData::cleanup();
}


/** Returns true if the Widget is either at the starting or ending side of the association */
bool AssociationWidget::contains(UMLWidget* Widget) {
	return (Widget == m_pWidgetA || Widget == m_pWidgetB);
}

/** Sets the association's type */
void AssociationWidget::setAssocType(Association_Type type) {
	//if the association new type is not supposed to have Multiplicity FloatingTexts and a Role
	//FloatingText then set the internl floating text pointers to null
	if( !AssocRules::allowMultiplicity( type, getWidgetA() -> getBaseType() ) ) {
		setMultiA( 0 ); setMultiA("");
		setMultiB( 0 ); setMultiB("");
	}
	if( !AssocRules::allowRole( type ) )
	{
		setRoleNameA("");
		setRoleADoc("");
		setRoleNameB("");
		setRoleBDoc("");
	}
	AssociationWidgetData::setAssocType(type);
	m_pAssociation->setAssocType(type);
}

/** Returns a QString Object representing this AssociationWidget */
QString AssociationWidget::toString() {
	QString string = "";

	if(m_pWidgetA) {
		string = m_pWidgetA -> getName();
	}
	string.append(":");

	if(m_pRoleA) {
		string += m_pRoleA -> getText();
	}
	string.append(":");
	switch(getAssocType()) {
	case at_Generalization:
		string.append(i18n("Generalization"));
		break;

	case at_Aggregation:
		string.append(i18n("Aggregation"));
		break;

	case at_Dependency:
		string.append(i18n("Dependency"));
		break;

	case at_Association:
		string.append(i18n("Association"));
		break;

	case at_Anchor:
		string.append(i18n("Anchor"));
		break;

	case at_Realization:
		string.append( i18n("Realization") );
		break;

	case at_Composition:
		string.append( i18n("Composition") );
		break;

	case at_UniAssociation:
		string.append( i18n("Uni Association") );
		break;

	case at_Implementation:
		string.append( i18n("Implementation") );
		break;

	case at_State:
		string.append( i18n("State Transition") );
		break;

	default:
		string.append(i18n("Other Type"));
		break;
	}
	;//end switch
	string.append(":");
	if(m_pWidgetB) {
		string += m_pWidgetB -> getName();
	}

	if(m_pRoleB) {
		string += m_pRoleB -> getText();
	}

	return string;
}

void AssociationWidget::mouseDoubleClickEvent(QMouseEvent * me) {
	if(me -> button() != RightButton && me->button() != LeftButton)
		return;
	int i = 0;
	if( ( i = m_LinePath.onLinePath( me -> pos() ) ) == -1 )
	{
		m_LinePath.setSelected(false);
		return;
	}
	if(me->button() == LeftButton) {
		/* if there is no point around the mouse pointer, we insert a new one */
		if (! m_LinePath.isPoint(i, me -> pos(), 5 ))
			m_LinePath.insertPoint( i, me -> pos() );
		else
			/* there was a point so we remove the point */
			m_LinePath.removePoint(i);

		m_LinePath.update();

		//calculateRoleTextSegment();
		calculateNameTextSegment();
	}
}

void AssociationWidget::moveEvent(QMoveEvent* me) {
	/*to be here a line segment has moved.
	  we need to see if the three text widgets needs to be moved.
	  there are a few things to check first though:

	  1) Do they exist
	  2) does it need to move:
	  2a) for the multi widgets only move if they changed region, otherwise they are close enough
	  2b) for role name move if the segment it is on moves.
	*/
	//first see if either the first or last segments moved, else no need to recalculate their point positions

	QPoint oldNamePoint = calculateTextPosition(tr_Name);
	QPoint oldMultiAPoint = calculateTextPosition(tr_MultiA);
	QPoint oldMultiBPoint = calculateTextPosition(tr_MultiB);
	QPoint oldChangeAPoint = calculateTextPosition(tr_ChangeA);
	QPoint oldChangeBPoint = calculateTextPosition(tr_ChangeB);
	QPoint oldRoleAPoint = calculateTextPosition(tr_RoleAName);
	QPoint oldRoleBPoint = calculateTextPosition(tr_RoleBName);

	m_LinePath.setPoint( m_nMovingPoint, me->pos() );
	int pos = m_LinePath.count() - 1;//set to last point for widget b

	if ( m_nMovingPoint == 1 || (m_nMovingPoint == pos-1) ) {
		calculateEndingPoints();
	}
	if (m_pChangeWidgetA && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_ChangeA, calculateTextPosition(tr_ChangeA), oldChangeAPoint);
	}
	if (m_pChangeWidgetB && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_ChangeB, calculateTextPosition(tr_ChangeB), oldChangeBPoint);
	}
	if (m_pMultiA && (m_nMovingPoint == 1)) {
		setTextPositionRelatively(tr_MultiA, calculateTextPosition(tr_MultiA), oldMultiAPoint);
	}
	if (m_pMultiB && (m_nMovingPoint == pos-1)) {
		setTextPositionRelatively(tr_MultiB, calculateTextPosition(tr_MultiB), oldMultiBPoint);
	}

	if (m_pName) {
		if(m_nMovingPoint == (int)m_unNameLineSegment ||
		   m_nMovingPoint - 1 == (int)m_unNameLineSegment) {
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name), oldNamePoint);
		}
	}

	if (m_pRoleA) {
		setTextPositionRelatively(tr_RoleAName, calculateTextPosition(tr_RoleAName), oldRoleAPoint);
	}
	if (m_pRoleB) {
		setTextPositionRelatively(tr_RoleBName, calculateTextPosition(tr_RoleBName), oldRoleBPoint);
	}
}


/** Calculates and sets the first and last point in the Association's LinePath
    Each point is a middle point of its respecting UMLWidget's Bounding rectangle
    or a corner of it
    This method picks which sides to use for the association */
void AssociationWidget::calculateEndingPoints() {
	/*
	 *For each UMLWidget the diagram is divided in four Regions by its diagonals
	 * as indicated below
	 *                                           Region 2
	 *   											 \              /
	 *											     	|--------|
	 *													|  \    /  |
	 *							Region 1		|   /  \   |    Region 3
	 *													|--------|
	 *   											 /              \
	 *                                           Region 4
	 *
	 *Each diagonal is defined by two corners of the bounding rectangle
	 *
	 *To calculate the first point in the LinePath we have to find out in which
	 * Region (defined by WidgetA's diagonals) is WidgetB's TopLeft corner (lets call it Region M) after that the first point will be
	 *the middle point of rectangle's side contained in Region M.

	 *
	 *To calculate the last point in the LinePath we repeate the above in but in the opposite direction
	 *(from widgetB to WidgetA)
	 */


	if(!m_pWidgetA || !m_pWidgetB)
		return;
	int xA =  (int)m_pWidgetA -> x();
	int yA = (int)m_pWidgetA -> y();
	int xB = (int)m_pWidgetB -> x();
	int yB = (int)m_pWidgetB -> y();
	m_OldCornerA.setX(xA);
	m_OldCornerA.setY(yA);
	m_OldCornerB.setX(xB);
	m_OldCornerB.setY(yB);
	uint size = m_LinePath.count();
	uint pos = size - 1;
	if(size < 2)
		m_LinePath.setStartEndPoints( m_OldCornerA, m_OldCornerB );

	QRect rc(xA, yA, m_pWidgetA->width(), m_pWidgetA->height());
	//see if an association to self
	//see if it needs to be set up before we continue
	//if self association/message and doesn't have the minimum
	//4 points then create it.  Make sure no points are out of bounds of viewing area
	//this only happends on first time through that we are worried about.
	if( m_pWidgetA == m_pWidgetB  && size < 4 ) {
		const int DISTANCE = 50;
		int x = (int)m_pWidgetA -> x();
		int y = (int)m_pWidgetA -> y();
		int h = (int)m_pWidgetA -> height();
		int w = (int)m_pWidgetA -> width();
		//see if above widget ok to start
		if( y - DISTANCE > 0 ) {
			m_LinePath.setStartEndPoints( QPoint( x + w / 4, y ) , QPoint( x + w * 3 / 4, y ) );
			m_LinePath.insertPoint( 1, QPoint( x + w / 4, y - DISTANCE ) );
			m_LinePath.insertPoint( 2 ,QPoint( x + w * 3 / 4, y - DISTANCE ) );
			m_WidgetARegion = m_WidgetBRegion = North;
		} else {
			m_LinePath.setStartEndPoints( QPoint( x + w / 4, y + h ), QPoint( x + w * 3 / 4, y + h ) );
			m_LinePath.insertPoint( 1, QPoint( x + w / 4, y + h + DISTANCE ) );
			m_LinePath.insertPoint( 2, QPoint( x + w * 3 / 4, y + h + DISTANCE ) );
			m_WidgetARegion = m_WidgetBRegion = South;
		}
		return;
	}//end a == b
	//if the line has more than one segment change the values to calculate
	//from widget to point 1
	size = m_LinePath.count();
	if( size > 2 ) {

		QPoint p = m_LinePath.getPoint( 1 );
		xB = p.x();
		yB = p.y();
	}
	//find widgetA region
	int oldCountA = m_nTotalCountA;
	Region oldRegionA = m_WidgetARegion;
	m_WidgetARegion = findPointRegion( rc, xB, yB );
	//move some regions to the standard ones
	switch( m_WidgetARegion ) {
	case NorthWest:
		m_WidgetARegion = North;
		break;
	case NorthEast:
		m_WidgetARegion = East;
		break;
	case SouthEast:
		m_WidgetARegion = South;
		break;
	case SouthWest:
	case Center:
		m_WidgetARegion = West;
		break;
	default:
		break;
	}

	int regionCountA = getRegionCount( m_WidgetARegion, true ) + 2;//+2 = (1 for this one and one to halve it)
	//updateRegionLineCount( regionCountA - 1, regionCountA, m_WidgetARegion, true );
	if( oldRegionA != m_WidgetARegion ) {
		updateRegionLineCount( regionCountA - 1, regionCountA, m_WidgetARegion, true );
		updateAssociations( oldCountA - 1, oldRegionA, true );//tell the old region this ones left

		updateAssociations( regionCountA, m_WidgetARegion, true );
	} else if( oldCountA != regionCountA ) {
		updateRegionLineCount( regionCountA - 1, regionCountA, m_WidgetARegion, true );

		updateAssociations( regionCountA, m_WidgetARegion, true );
	} else
		updateRegionLineCount( m_nIndexA, m_nTotalCountA, m_WidgetARegion, true );
	//now do the same for widgetB
	//if the line has more than one segment change the values to calculate
	//from widgetB to point the last point away from it
	xA =  (int)m_pWidgetA -> x();
	yA = (int)m_pWidgetA -> y();
	xB = (int)m_pWidgetB -> x();
	yB = (int)m_pWidgetB -> y();
	if (size > 2 ) {
		QPoint p = m_LinePath.getPoint( pos - 1 );
		xA = p.x();
		yA = p.y();
	}
	rc.setX( xB );
	rc.setY( yB );
	rc.setWidth( m_pWidgetB->width() );
	rc.setHeight( m_pWidgetB->height() );
	int oldCountB = m_nTotalCountB;
	Region oldRegionB = m_WidgetBRegion;
	m_WidgetBRegion = findPointRegion( rc, xA, yA );

	//move some regions to the standard ones
	switch( m_WidgetBRegion ) {
	case NorthWest:
		m_WidgetBRegion = North;
		break;
	case NorthEast:
		m_WidgetBRegion = East;

		break;
	case SouthEast:
		m_WidgetBRegion = South;
		break;
	case SouthWest:
	case Center:
		m_WidgetBRegion = West;
		break;
	default:
		break;
	}
	int regionCountB = getRegionCount( m_WidgetBRegion, false ) + 2;//+2 = (1 for this one and one to halve it)
	if( oldRegionB != m_WidgetBRegion ) {
		updateRegionLineCount( regionCountB - 1, regionCountB, m_WidgetBRegion, false );
		updateAssociations( oldCountB - 1, oldRegionB, false );
		updateAssociations( regionCountB, m_WidgetBRegion, false );
	} else if( oldCountB != regionCountB ) {
		updateRegionLineCount( regionCountB - 1, regionCountB, m_WidgetBRegion, false );
		updateAssociations( regionCountB, m_WidgetBRegion, false );
	} else
		updateRegionLineCount( m_nIndexB, m_nTotalCountB, m_WidgetBRegion, false );
}

/** Read property of bool m_bActivated. */
const bool AssociationWidget::isActivated() {
	return m_bActivated;
}

/** Set the m_bActivated flag of a widget but does not perform the Activate method */
void AssociationWidget::setActivated(bool Active /*=true*/) {
	m_bActivated = Active;
}

// this will synchronize UMLAssociation w/ this new WidgetData
void AssociationWidget::mergeAssociationDataIntoUMLRepresentation()
{
	UMLAssociation *uml = getAssociation();

	// would be desirable to do the following
	// so that we can be sure its back to initial state
	// in case we missed something here.
	uml->init();

	// all attributes should be synched based on data widget
	uml->setRoleAId(getWidgetAID());
	uml->setRoleBId(getWidgetBID());
	uml->setAssocType(getAssocType());
	uml->setChangeabilityA(getChangeabilityA());
	uml->setChangeabilityB(getChangeabilityB());

	uml->setDoc(getDoc());
	uml->setRoleADoc(getRoleADoc());
	uml->setRoleBDoc(getRoleBDoc());

	// dont believe this is needed here.
	if(getWidgetA() != 0) {
		UMLObject *oA = getWidgetA()->getUMLObject();
		if (oA)
			uml->setObjectA(oA);
		else {
			UMLObject *obj = m_pView->getDocument()->findUMLObject(getWidgetAID());
			if (obj)
				uml->setObjectA(obj);
		}
	}

	if(getWidgetB() != 0) {
		UMLObject *oB = getWidgetB()->getUMLObject();
		if (oB)
			uml->setObjectB(oB);
		else {
			UMLObject *obj = m_pView->getDocument()->findUMLObject(getWidgetBID());
			if (obj)
				uml->setObjectA(obj);
		}
	}

	// floating text widgets
	FloatingTextData *textData = getNameData();
	if (textData)
		uml->setName(textData->getText());

	textData = getRoleAData();
	if (textData)
	{
		uml->setRoleNameA(textData->getText());
		// it doesnt make sense to have visibility wi/o Rolename
		// so we only set it when its in here. Probably should have
		// error condition thrown when visb is set but rolename isnt.
		uml->setVisibilityA(getVisibilityA());
	}

	textData = getRoleBData();
	if (textData)
	{
		uml->setRoleNameB(textData->getText());
		uml->setVisibilityB(getVisibilityB());
	}

	textData = getMultiDataA();
	if (textData)
		uml->setMultiA(textData->getText());

	textData = getMultiDataB();
	if (textData)
		uml->setMultiB(textData->getText());

}

/** Adjusts the endding point of the association that connects to Widget */
void AssociationWidget::widgetMoved(UMLWidget* widget, int x, int y ) {
	QPoint oldNamePoint = calculateTextPosition(tr_Name);
	QPoint oldMultiAPoint = calculateTextPosition(tr_MultiA);
	QPoint oldMultiBPoint = calculateTextPosition(tr_MultiB);
	QPoint oldChangeAPoint = calculateTextPosition(tr_ChangeA);
	QPoint oldChangeBPoint = calculateTextPosition(tr_ChangeB);
	QPoint oldRoleAPoint = calculateTextPosition(tr_RoleAName);
	QPoint oldRoleBPoint = calculateTextPosition(tr_RoleBName);

	int dx = m_OldCornerA.x() - x;
	int dy = m_OldCornerA.y() - y;
	uint size = m_LinePath.count();
	uint pos = size - 1;
	calculateEndingPoints();

	if( m_pWidgetA == m_pWidgetB ) {
		for( int i=1 ; i < (int)pos ; i++ ) {
			QPoint p = m_LinePath.getPoint( i );
			int newX = p.x() - dx;
			int newY = p.y() - dy;
			if( m_pView -> getSnapToGrid() ) {
				int gridX = m_pView -> getSnapX();
				int gridY = m_pView -> getSnapY();
				int modX = newX % gridX;
				int modY = newY % gridY;
				newX -= modX;
				newY -= modY;
				if( modX >= ( gridX / 2 ) )
					newX += gridX;
				if( modY >= ( gridY / 2 ) )
					newY += gridY;
			}
			p.setX( newX );
			p.setY( newY );
			m_LinePath.setPoint( i, p );
		}

		if ( m_pName && !m_pName->getSelected() ) {
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name), oldNamePoint);
		}

	}//end if widgetA = widgetB
	else if( (m_pWidgetA==widget) && m_pName && (m_unNameLineSegment == 0)
		&& !m_pWidgetA->getSelected() ) {
		//only calculate position and move text if the segment it is on is moving
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name),
						  oldNamePoint);
	}//end if widgetA moved
	else if ( (m_pWidgetB==widget) && m_pName && (m_unNameLineSegment == pos-1)
		  && !m_pName->getSelected() ) {
		//only calculate position and move text if the segment it is on is moving
			setTextPositionRelatively(tr_Name, calculateTextPosition(tr_Name),
						  oldNamePoint);
	}//end if widgetB moved

	if ( m_pRoleA && !m_pRoleA->getSelected() ) {
		setTextPositionRelatively(tr_RoleAName, calculateTextPosition(tr_RoleAName), oldRoleAPoint);
	}
	if ( m_pRoleB && !m_pRoleB->getSelected() ) {
		setTextPositionRelatively(tr_RoleBName, calculateTextPosition(tr_RoleBName), oldRoleBPoint);
	}
	if ( m_pMultiA && !m_pMultiA->getSelected() ) {
		setTextPositionRelatively(tr_MultiA, calculateTextPosition(tr_MultiA), oldMultiAPoint);
	}
	if ( m_pMultiB && !m_pMultiB->getSelected() ) {
		setTextPositionRelatively(tr_MultiB, calculateTextPosition(tr_MultiB), oldMultiBPoint);
	}
	if ( m_pChangeWidgetA && !m_pChangeWidgetA->getSelected() ) {
		setTextPositionRelatively(tr_ChangeA, calculateTextPosition(tr_ChangeA), oldChangeAPoint);
	}
	if ( m_pChangeWidgetB && !m_pChangeWidgetB->getSelected() ) {
		setTextPositionRelatively(tr_ChangeB, calculateTextPosition(tr_ChangeB), oldChangeBPoint);
	}
}//end method widgetMoved

/** Finds out in which region of rectangle Rect contains the Point (PosX, PosY) and returns the region
    number:
    1 = Region 1
    2 = Region 2
    3 = Region 3
    4 = Region 4
    5 = On diagonal 2 between Region 1 and 2
    6 = On diagonal 1 between Region 2 and 3
    7 = On diagonal 2 between Region 3 and 4
    8	= On diagonal 1 between Region4 and 1
    9 = On diagonal 1 and On diagonal 2 (the center)
*/
AssociationWidget::Region AssociationWidget::findPointRegion(QRect Rect, int PosX, int PosY) {

	float w = (float)Rect.width();
	float h = (float)Rect.height();
	float x =  (float)Rect.x();
	float y = (float)Rect.y();
	float Slope2 = w / h;
	float Slope1 = Slope2*(float)(-1);
	float b1 = x + w - ( Slope1* y );
	float b2 = x - ( Slope2* y );

	float eval1 = Slope1 * (float)PosY + b1;
	float eval2 = Slope2  *(float)PosY + b2;

	Region result = Error;
	//if inside region 1
	if(eval1 > PosX && eval2 > PosX) {
		result = West;

	}
	//if inside region 2
	else if (eval1 > PosX && eval2 < PosX) {
		result = North;
	}
	//if inside region 3
	else if (eval1 < PosX && eval2 < PosX) {
		result = East;
	}
	//if inside region 4
	else if (eval1 < PosX && eval2 > PosX) {
		result = South;
	}
	//if inside region 5
	else if (eval1 == PosX && eval2 < PosX) {
		result = NorthWest;
	}
	//if inside region 6

	else if (eval1 < PosX && eval2 == PosX) {
		result = NorthEast;

	}
	//if inside region 7
	else if (eval1 == PosX && eval2 > PosX) {
		result = SouthEast;
	}
	//if inside region 8
	else if (eval1 > PosX && eval2 == PosX) {
		result = SouthWest;
	}
	//if inside region 9
	else if (eval1 == PosX && eval2 == PosX) {
		result = Center;
	}

	return result;
}

QPoint AssociationWidget::findRectIntersectionPoint(UMLWidget* pWidget, QPoint P1, QPoint P2) {
	QPoint result(-1, -1);

	int old_region = -1;
	if(!pWidget) {
		return result;
	}
	int X = -1, Y = -1;
	if(pWidget == m_pWidgetA)
	{
		X = (int)m_pWidgetB -> x();
		Y = (int)m_pWidgetB -> y();
		old_region = m_nCornerARegion;
	} else {
		X = (int)m_pWidgetA -> x();
		Y = (int)m_pWidgetA -> y();
		old_region = m_nCornerBRegion;
	}
	QRect rc((int)pWidget -> x(), (int)pWidget -> y(), pWidget->width(), pWidget->height());
	int region = findPointRegion(rc, X, Y);
	if(region == old_region) {
		return QPoint(-1, -1);
	}
	switch(region)
	{
	case West:
		result = findIntersection(rc.topLeft(), rc.bottomLeft(), P1, P2);
		break;
	case North:
		result = findIntersection(rc.topLeft(), rc.topRight(), P1, P2);
		break;
	case East:
		result = findIntersection(rc.topRight(), rc.bottomRight(), P1, P2);

		break;
	case South:
		result = findIntersection(rc.bottomLeft(), rc.bottomRight(), P1, P2);
		break;

	case NorthWest:
		result = rc.topLeft();

		break;
	case NorthEast:
		result  = rc.topRight();
		break;
	case SouthEast:
		result = rc.bottomRight();
		break;

	case SouthWest:
	case Center:
		result = rc.bottomLeft();
		break;
	}
	return result;
}

/** Returns the intersection point between lines P1P2 and P3P4, if the intersection
    point is not contained in the segment P1P2 then it returns (-1, -1)*/
QPoint AssociationWidget::findIntersection(QPoint P1, QPoint P2, QPoint P3, QPoint P4) {
	/*
	 * For the function's internal calculations remember:
	 * QT coordinates start with the point (0,0) as the topleft corner and x-values
	 * increase from left to right and y-values increase from top to bottom; it means
	 * the visible area is quadrant I in the regular XY coordinate system
	 *

	 *								|
	 *	Quadrant II		|   Quadrant I
	 *		-----------------|-----------------
	 *	Quadrant III		|   Quadrant IV
	 *								|
	 * In order for the linear function calculations to work in this method we must switch x and y values
	 * (x values become y values and viceversa)
	 */
	int x1 = P1.y();
	int y1 = P1.x();
	int x2 = P2.y();
	int y2 = P2.x();
	int x3 = P3.y();
	int y3 = P3.x();
	int x4 = P4.x();
	int y4 = P4.y();

	//Line 1 is the line between (x1,y1) and (x2,y2)
	//Line 2 is the line between (x3,y3) and (x4, y4)
	bool no_line1 = true; //it is false if Line 1 is a linear function
	bool no_line2 = true; //it is false if Line 2 is a linear function
	float slope1 = 0, slope2 = 0, b1 = 0, b2 = 0;
	if(x2 != x1) {
		slope1 = (y2 - y1) / (x2 - x1);
		b1 = y1 - (slope1*x1);
		no_line1 = false;
	}
	if(x4 != x3) {
		slope2 = (y4 - y3) / (x4 - x3);
		b2 = y3 - (slope2*x3);
		no_line2 = false;
	}
	QPoint pt;
	//if either line is not a function
	if(no_line2 && no_line1) {
		//if the lines are not the same one
		if(x1 != x3) {
			return QPoint(-1, -1);
		}
		//if the lines are the same one
		if(y3 <= y4) {
			if( y3 <= y1 && y1 <= y4) {
				return QPoint(y1, x1);
			} else {
				return QPoint(y2, x2);
			}
		} else {
			if( y4 <= y1 && y1 <= y3) {
				return QPoint(y1, x1);
			} else {
				return QPoint(y2, x2);
			}
		}

	} else if( no_line1) {
		pt.setX(static_cast<int>((slope2*x1) + b2));
		pt.setY(x1);
		if(y1 >= y2) {
			if( !(y2 <= pt.x() && pt.x() <= y1)) {
				pt.setX(-1);
				pt.setY(-1);
			}
		}
		else {
			if( !(y1 <= pt.x() && pt.x() <= y2)) {
				pt.setX(-1);
				pt.setY(-1);
			}
		}
		return pt;
	} else if( no_line2) {
		pt.setX(static_cast<int>((slope1*x3) + b1));
		pt.setY(x3);
		if(y3 >= y4) {

			if( !(y4 <= pt.x() && pt.x() <= y3)) {

				pt.setX(-1);
				pt.setY(-1);
			}
		} else {
			if( !(y3 <= pt.x() && pt.x() <= y4)) {
				pt.setX(-1);
				pt.setY(-1);
			}
		}
		return pt;
	}
	pt.setY(static_cast<int>((b2 - b1) / (slope1 - slope2)));
	pt.setX(static_cast<int>((slope1*pt.y()) + b1));
	//the intersection point must be inside the segment (x1, y1) (x2, y2)
	if(x2 >= x1&& y2 >= y1) {
		if(! ((x1 <= pt.x() && pt.x() <= x2)	&& (y1 <= pt.y() && pt.y() <= y2)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	} else if (x2 < x1 && y2 >= y1)
	{

		if(! ((x2 <= pt.x() && pt.x() <= x1)	&& (y1 <= pt.y() && pt.y() <= y2)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	} else if (x2 >= x1 && y2 < y1)
	{
		if(! ((x1 <= pt.x() && pt.x() <= x2)	&& (y2 <= pt.y() && pt.y() <= y1)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	} else {
		if(! ((x2 <= pt.x() && pt.x() <= x1)	&& (y2 <= pt.y() && pt.y() <= y1)) ) {
			pt.setX(-1);
			pt.setY(-1);
		}
	}


	return pt;
}

/** Returns the total length of the assocition's LinePath:

result = segment_1_length + segment_2_length + ..... + segment_n_length  */
float AssociationWidget::totalLength() {
	uint size = m_LinePath.count();

	//Open space in the LinePath point array to insert the new point
	//move all the points from j position one space ahead
	int xi = 0, xj = 0, yi = 0, yj = 0;

	float total_length = 0;
	for(uint i = 0; i < size - 1; i++) {
		QPoint pi = m_LinePath.getPoint( i );
		QPoint pj = m_LinePath.getPoint( i+1 );
		xi = pi.y();
		xj = pj.y();
		yi = pi.x();
		yj = pj.x();

		total_length +=  sqrt( double(((xj - xi)*(xj - xi)) + ((yj - yi)*(yj - yi))) );
	}

	return total_length;
}


/** Calculates which point of segment P1P2 has a distance equal to Distance from P1,
    Lets say such point is P3,  the distance from P1 to P3 must be equal to Distance
    and if P3 is not a point of the segment P1P2 then the function returns (-1,-1)
*/
QPoint AssociationWidget::calculatePointAtDistance(QPoint P1, QPoint P2, float Distance) {
	/*
	  the distance D between points (x1, y1) and (x3, y3) has the following formula:
	  D = ----      --------------------------------
	  \     /               2                2
	  \ /    (x3 - x1) + (y3 - y1)

	  D, x1 and y1 are known and the point (x3, y3) is inside line (x1,y1)(x2,y2), so if the
	  that line has the formula y = mx + b
	  then y3 = m*x3 + b

	  2                  2                 2
	  D   = (x3 - x1)   + (y3 - y1)

	  2         2                         2        2                         2

	  D    = x3    - 2*x3*x1 + x1   + y3   - 2*y3*y1  + y1

	  2         2       2         2                          2
	  D    - x1    - y1    = x3    - 2*x3*x1  + y3   - 2*y3*y1

	  2         2       2         2                                      2

	  D    - x1    - y1    = x3    - 2*x3*x1  + (m*x3 + b)  - 2*(m*x3 + b)*y1

	  2         2         2                      2         2           2
	  D    - x1    - y1     + 2*b*y1 - b   =  (m  + 1)*x3     + (-2*x1 + 2*m*b -2*m*y1)*x3

	  2         2         2                      2
	  C = - D    + x1    + y1   - 2*b*y1 + b


	  2
	  A = (m    + 1)

	  B = (-2*x1 + 2*m*b -2*m*y1)

	  and we have
	  2
	  A*x3 + Bx3 - C = 0


	  (                ---------------  )
	  sol_1  =(  -B + ---   /   2                )
	  (	 		    \/   B   - 4*A*C  )

	  --------------------------------
	  2*A


	  (                ---------------  )
	  sol_2  =(  -B -   ---   /   2               )
	  (	 		    \/   B   - 4*A*C)
	  --------------------------------
	  2*A



	  then in the distance formula we have only one variable x3 and that is easy
	  to calculate
	*/
	int x1 = P1.y();
	int y1 = P1.x();
	int x2 = P2.y();
	int y2 = P2.x();

	if(x2 == x1) {
		return QPoint(x1, static_cast<int>(y1 + Distance));
	}
	float slope = (static_cast<float>(y2) - static_cast<float>(y1)) / (static_cast<float>(x2) - static_cast<float>(x1));
	float b = (y1 - slope*x1);
	float A = (slope * slope) + 1;
	float B = (2*slope*b) - (2*x1)  - (2*slope*y1);
	float C = (b*b) - (Distance*Distance) + (x1*x1) + (y1*y1) - (2*b*y1);
	float t = B*B - 4*A*C;

	if(t < 0) {
		return QPoint(-1, -1);

	}
	float sol_1 = ((-1* B) + sqrt(t) ) / (2*A);
	float sol_2 = ((-1*B) - sqrt(t) ) / (2*A);

	if(sol_1 < 0 && sol_2 < 0) {

		return QPoint(-1, -1);

	}
	if(sol_1 < 0 && sol_2 >=0) {
		if(x2 > x1) {
			if(x1 <= sol_2 && sol_2 <= x2) {
				return QPoint(static_cast<int>(slope*sol_2 + b), static_cast<int>(sol_2));
			}
			else {

				return QPoint(-1, -1);
			}
		} else {
			if(x2 <= sol_2 && sol_2 <= x1) {
				return QPoint(static_cast<int>(slope*sol_2 + b), static_cast<int>(sol_2));
			} else {
				return QPoint(-1, -1);
			}
		}
	} else if(sol_1 >= 0 && sol_2 < 0) {
		if(x2 > x1) {
			if(x1 <= sol_1 && sol_1 <= x2) {
				return QPoint(static_cast<int>(slope*sol_1 + b), static_cast<int>(sol_1));
			} else {
				return QPoint(-1, -1);
			}
		} else {
			if(x2 <= sol_1 && sol_1 <= x1) {
				return QPoint(static_cast<int>(slope*sol_1 + b), static_cast<int>(sol_1));
			} else {
				return QPoint(-1, -1);
			}

		}
	} else {
		if(x2 > x1) {
			if(x1 <= sol_1 && sol_1 <= x2) {
				return QPoint(static_cast<int>(slope*sol_1 + b), static_cast<int>(sol_1));
			} else {
				if(x1 <= sol_2 && sol_2 <= x2) {
					return QPoint(static_cast<int>(slope*sol_2 + b), static_cast<int>(sol_2));
				} else {

					return QPoint(-1, -1);
				}
			}
		} else {
			if(x2 <= sol_1 && sol_1 <= x1) {
				return QPoint(static_cast<int>(slope*sol_1 + b), static_cast<int>(sol_1));
			} else {
				if(x2 <= sol_2 && sol_2 <= x1) {
					return QPoint(static_cast<int>(slope*sol_2 + b), static_cast<int>(sol_2));
				} else {
					return QPoint(-1, -1);
				}
			}
		}
	}

	return QPoint(-1, -1);
}

/** Calculates which point of a perpendicular line to segment P1P2 that contains P2
    has a distance equal to Distance from P2,
    Lets say such point is P3,  the distance from P2 to P3 must be equal to Distance
*/
QPoint AssociationWidget::calculatePointAtDistanceOnPerpendicular(QPoint P1, QPoint P2, float Distance) {
	/*
	  the distance D between points (x2, y2) and (x3, y3) has the following formula:

	  D = ----      --------------------------------
	  \     /               2                2
	  \ /    (x3 - x2) + (y3 - y2)

	  D, x2 and y2 are known and line P2P3 is perpendicular to line (x1,y1)(x2,y2), so if the
	  line P1P2 has the formula y = m*x + b,
	  then              (x1 - x2)
	  m=  -----------    , because it is perpendicular to line P1P2
	  (y2 - y1)

	  also y2 = m*x2 + b
	  => b = y2 - m*x2

	  then P3 = (x3, m*x3 + b

	  2                  2                 2
	  D   = (x3 - x2)   + (y3 - y2)

	  2         2                         2        2                         2
	  D    = x3    - 2*x3*x2 + x2   + y3   - 2*y3*y2  + y2

	  2        2       2         2                          2
	  D    - x2    - y2    = x3    - 2*x3*x2  + y3   - 2*y3*y2



	  2         2       2         2                                      2
	  D    - x2    - y2    = x3    - 2*x3*x2  + (m*x3 + b)  - 2*(m*x3 + b)*y2

	  2         2       2                      2         2           2
	  D    - x2    - y2     + 2*b*y2 - b   =  (m  + 1)*x3     + (-2*x2 + 2*m*b -2*m*y2)*x3

	  2         2         2                      2
	  C = - D    + x2    + y2   - 2*b*y2 + b

	  2
	  A = (m    + 1)

	  B = (-2*x2 + 2*m*b -2*m*y2)

	  and we have
	  2

	  A*x3 + Bx3 - C = 0


	  (                ---------------  )
	  sol_1  =(  -B + ---   /   2                )
	  (	 		    \/   B   - 4*A*C  )
	  --------------------------------
	  2*A

	  (                ---------------  )
	  sol_2  =(  -B -   ---   /   2               )
	  (	 		    \/   B   - 4*A*C)

	  --------------------------------
	  2*A


	  then in the distance formula we have only one variable x3 and that is easy
	  to calculate
	*/
	int x1 = P1.y();
	int y1 = P1.x();
	int x2 = P2.y();

	int y2 = P2.x();

	if(x2 == x1) {
		return QPoint(static_cast<int>(x2+ Distance), y2);
	}
	float slope = (static_cast<float>(x1) - static_cast<float>(x2)) / (static_cast<float>(y2) - static_cast<float>(y1));
	float b = (y2 - slope*x2);
	float A = (slope * slope) + 1;
	float B = (2*slope*b) - (2*x2)  - (2*slope*y2);
	float C = (b*b) - (Distance*Distance) + (x2*x2) + (y2*y2) - (2*b*y2);
	float t = B*B - 4*A*C;
	if(t < 0) {
		return QPoint(-1, -1);
	}
	float sol_1 = ((-1* B) + sqrt(t) ) / (2*A);

	float sol_2 = ((-1*B) - sqrt(t) ) / (2*A);

	if(sol_1 < 0 && sol_2 < 0) {
		return QPoint(-1, -1);
	}
	if(sol_1 < 0 && sol_2 >=0) {
		return QPoint(static_cast<int>(slope*sol_2 + b), static_cast<int>(sol_2));
	} else if(sol_1 >= 0 && sol_2 < 0) {
		return QPoint(static_cast<int>(slope*sol_1 + b), static_cast<int>(sol_1));
	} else //Choose one solution , either will work fine
	{
		if(slope >= 0) {
			if(sol_1 <= sol_2)
			{
				return QPoint(static_cast<int>(slope*sol_2 + b), static_cast<int>(sol_2));
			} else {
				return QPoint(static_cast<int>(slope*sol_1 + b), static_cast<int>(sol_1));
			}
		} else {
			if(sol_1 <= sol_2)
			{
				return QPoint(static_cast<int>(slope*sol_1 + b), static_cast<int>(sol_1));
			} else {
				return QPoint(static_cast<int>(slope*sol_2 + b), static_cast<int>(sol_2));
			}
		}

	}
	return QPoint(-1, -1);

}

/** Calculates the intersection (PS) between line P1P2 and a perpendicular line containing
    P3, the result is returned in ResultingPoint. and result value represents the distance
    between ResultingPoint and P3; if this value is negative an error ocurred. */
float AssociationWidget::perpendicularProjection(QPoint P1, QPoint P2, QPoint P3, QPoint ResultingPoint) {
	//line P1P2 is Line 1 = y=slope1*x + b1


	//line P3PS is Line 1 = y=slope2*x + b2

	float slope2 = 0;
	float slope1 = 0;
	float sx = 0, sy = 0;
	int y2 = P2.x();
	int y1 = P1.x();
	int x2 = P2.y();
	int x1 = P1.y();
	int y3 = P3.x();
	int x3 = P3.y();
	float distance = 0;
	float b1 = 0;

	float b2 = 0;

	if(x2 == x1) {
		sx = x2;
		sy = y3;
	} else if(y2 == y1) {

		sy = y2;
		sx = x3;
	} else {
		slope1 = (y2 - y1)/ (x2 - x1);
		slope2 = (x1 - x2)/ (y2 - y1);
		b1 = y2 - (slope1 * x2);
		b2 = y3 - (slope2 * x3);
		sx = (b2 - b1) / (slope1 - slope2);
		sy = slope1*sx + b1;
	}
	distance = static_cast<int>( sqrt( ((x3 - sx)*(x3 - sx)) + ((y3 - sy)*(y3 - sy)) ) );

	ResultingPoint.setX(static_cast<int>(sy));
	ResultingPoint.setY(static_cast<int>(sx));

	return distance;
}

QPoint AssociationWidget::calculateTextPosition(Text_Role role) {
	QPoint p( -1, -1 ), q( -1, -1 );
	uint size = m_LinePath.count();
	uint pos = size - 1;
	int x = 0, y = 0;
	int textW = 0, textH = 0;
	int slope = 0, divisor = 1;
	const int SPACE = 2;
	FloatingText const * text = 0;

	if(role == tr_MultiA) {
		text = getMultiAWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( 0 );
		q = m_LinePath.getPoint( 1 );
		divisor = (p.x()-q.x());
		if (divisor != 0)
			slope = (p.y()-q.y())/divisor;
		else
			slope = 10000;


		if( p.y() > q.y() )
			if(slope == 0)
				y = p.y() + SPACE;
			else
				y = p.y() - SPACE - textH;
		else
			if(slope == 0)
				y = p.y() - SPACE - textH;
			else
				y = p.y() + SPACE;

		if( p.x() < q.x() )
			if(slope == 0)
				x = p.x() + SPACE;
			else
				x = p.x() - SPACE - textW;
		else
			if(slope == 0)
				x = p.x() - SPACE - textW;
			else
				x = p.x() + SPACE;

	} else if(role == tr_MultiB) {
		text = getMultiBWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( pos );
		q = m_LinePath.getPoint( pos - 1 );
		divisor = (p.x()-q.x());
		if (divisor != 0)
			slope = (p.y()-q.y())/divisor;
		else
			slope = 10000000;

		if( p.y() > q.y() )
		{
			if(slope == 0)
				y = p.y() - SPACE;
			else
				y = p.y() - textH - SPACE;
		} else
			if(slope == 0)
				y = p.y() - textH - SPACE;
			else
				y = p.y() + SPACE;


		if( p.x() < q.x() )
			if(slope == 0)
				x = p.x() + SPACE;
			else
				x = p.x() - textW + SPACE;
		else
			if(slope == 0)
				x = p.x() - textW + SPACE;
			else
				x = p.x() + SPACE;

	} else if(role == tr_Name) {

		x = (int)( ( m_LinePath.getPoint(m_unNameLineSegment).x() +
		             m_LinePath.getPoint(m_unNameLineSegment + 1).x() ) / 2 );

		y = (int)( ( m_LinePath.getPoint(m_unNameLineSegment).y() +
		             m_LinePath.getPoint(m_unNameLineSegment + 1).y() ) / 2 );

	} else if(role == tr_ChangeA) {

	 	text = getChangeWidgetA();

		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( 0 );
		q = m_LinePath.getPoint( 1 );

		if( p.y() > q.y() )
			y = p.y() - SPACE - (textH *2);
		else
			y = p.y() + SPACE + textH;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - SPACE - textW;

	} else if(role == tr_ChangeB) {

		text = getChangeWidgetB();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}

		p = m_LinePath.getPoint( pos );
		q = m_LinePath.getPoint( pos - 1 );

		if( p.y() > q.y() )
			y = p.y() - (textH*2) - SPACE;
		else
			y = p.y() + textH + SPACE;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - textW - SPACE;

	} else if(role == tr_RoleAName) {

	 	text = getRoleAWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}
		p = m_LinePath.getPoint( 0 );
		q = m_LinePath.getPoint( 1 );

		if( p.y() > q.y() )
			y = p.y() - SPACE - textH;
		else
			y = p.y() + SPACE;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - SPACE - textW;
	}
	else if(role == tr_RoleBName)
	{
		text = getRoleBWidget();
		if( text ) {
			textW = text -> width();
			textH = text -> height();
		}

		p = m_LinePath.getPoint( pos );
		q = m_LinePath.getPoint( pos - 1 );
		if( p.y() > q.y() )
			y = p.y() - textH - SPACE;
		else
			y = p.y() + SPACE;

		if( p.x() < q.x() )
			x = p.x() + SPACE;
		else
			x = p.x() - textW - SPACE;
	}
	p = QPoint( x, y );
	return p;
}

void AssociationWidget::calculateNameTextSegment() {
	if(!m_pName) {
		return;
	}
	//changed to use the middle of the text
	//i think this will give a better result.
	//never know what sort of lines people come up with
	//and text could be long to give a false reading
	int xt = (int)m_pName -> x();
	int yt = (int)m_pName -> y();
	xt += m_pName -> width() / 2;
	yt += m_pName -> height() / 2;
	uint size = m_LinePath.count();
	int xi = 0, xj = 0, yi = 0, yj = 0;
	//sum of length(PTP1) and length(PTP2)
	float total_length = 0;
	float smallest_length = 0;
	for(uint i = 0; i < size - 1; i++) {
		QPoint pi = m_LinePath.getPoint( i );
		QPoint pj = m_LinePath.getPoint( i+1 );
		xi = pi.x();
		xj = pj.x();
		yi = pi.y();
		yj = pj.y();
		total_length =  sqrt( pow( double(xt - xi), 2.0 ) + pow( double(yt - yi), 2. ) ) + sqrt( pow( double(xt - xj), 2.0) + pow( double(yt - yj), 2.0) );
		//this gives the closest point
		if( total_length < smallest_length || i == 0) {
			smallest_length = total_length;
			m_unNameLineSegment = i;
		}
	}
}

void AssociationWidget::setTextPosition(Text_Role role, QPoint pos) {
	bool startMove = false;
	if( m_pMultiA && m_pMultiA->getStartMove() )
		startMove = true;
	else if( m_pMultiB && m_pMultiB->getStartMove() )
		startMove = true;
	else if( m_pChangeWidgetA && m_pChangeWidgetA->getStartMove() )
		startMove = true;
	else if( m_pChangeWidgetB && m_pChangeWidgetB->getStartMove() )
		startMove = true;
	else if( m_pRoleA  && m_pRoleA->getStartMove() )
		startMove = true;
	else if( m_pRoleB  && m_pRoleB->getStartMove() )
		startMove = true;
	else if( m_pName && m_pName->getStartMove() )
		startMove = true;

	if (startMove) {
		return;
	}

	switch(role) {
	case tr_MultiA:
			m_pMultiA->setLinePos( pos.x(), pos.y() );
		break;


	case tr_MultiB:
			m_pMultiB->setLinePos( pos.x(), pos.y() );
		break;

	case tr_Name:
	case tr_Coll_Message:
			m_pName->setLinePos( pos.x(), pos.y() );
		break;
	case tr_RoleAName:
			m_pRoleA->setLinePos( pos.x(), pos.y() );
		break;
	case tr_RoleBName:
			m_pRoleB->setLinePos( pos.x(), pos.y() );
		break;
	case tr_ChangeA:
			m_pChangeWidgetA->setLinePos( pos.x(), pos.y() );
		break;
	case tr_ChangeB:
			m_pChangeWidgetB->setLinePos( pos.x(), pos.y() );
		break;
	default:
		break;
	}
}

void AssociationWidget::setTextPositionRelatively(Text_Role role, QPoint pos, QPoint oldPosition) {
	bool startMove = false;
	if( m_pMultiA && m_pMultiA->getStartMove() )
		startMove = true;
	else if( m_pMultiB && m_pMultiB->getStartMove() )
		startMove = true;
	else if( m_pChangeWidgetA && m_pChangeWidgetA->getStartMove() )
		startMove = true;
	else if( m_pChangeWidgetB && m_pChangeWidgetB->getStartMove() )
		startMove = true;
	else if( m_pRoleA  && m_pRoleA->getStartMove() )
		startMove = true;
	else if( m_pRoleB  && m_pRoleB->getStartMove() )
		startMove = true;
	else if( m_pName && m_pName->getStartMove() )
		startMove = true;

	if (startMove) {
		return;
	}

	switch(role) {
	case tr_MultiA:
			m_pMultiA->setLinePositionRelatively( pos.x(), pos.y(),
							      oldPosition.x(), oldPosition.y() );
		break;


	case tr_MultiB:
			m_pMultiB->setLinePositionRelatively( pos.x(), pos.y(),
							      oldPosition.x(), oldPosition.y() );
		break;

	case tr_Name:
	case tr_Coll_Message:
			m_pName->setLinePositionRelatively( pos.x(), pos.y(),
							    oldPosition.x(), oldPosition.y() );
		break;
	case tr_RoleAName:
			m_pRoleA->setLinePositionRelatively( pos.x(), pos.y(),
							     oldPosition.x(), oldPosition.y() );
		break;
	case tr_RoleBName:
			m_pRoleB->setLinePositionRelatively( pos.x(), pos.y(),
							     oldPosition.x(), oldPosition.y() );
		break;
	case tr_ChangeA:
			m_pChangeWidgetA->setLinePositionRelatively( pos.x(), pos.y(),
								     oldPosition.x(), oldPosition.y() );
		break;
	case tr_ChangeB:
			m_pChangeWidgetB->setLinePositionRelatively( pos.x(), pos.y(),
								     oldPosition.x(), oldPosition.y() );
		break;
	default:
		break;
	}
}

void AssociationWidget::mousePressEvent(QMouseEvent * me) {
	m_nMovingPoint = -1;
	//make sure we should be here depending on the button
	if(me -> button() != RightButton && me->button() != LeftButton)
		return;
	QPoint mep = me->pos();
	//See is the user has clicked on a point to start moving od line from that point
	checkPoints(mep);
	bool _select = m_bSelected?false:true;
	if( me -> state() != ShiftButton )
		m_pView -> clearSelected();
	m_bSelected = _select;
	setSelected( _select );
}

void AssociationWidget::mouseReleaseEvent(QMouseEvent * me) {
	if(me -> button() != RightButton && me->button() != LeftButton) {
		setSelected( (m_bSelected = false) );
		return;
	}
	m_nMovingPoint = -1;
	if(me->button() == RightButton) {
		//work out the type of menu we want
		//work out if the association allows rolenames, multiplicity, etc
		//also must be within a certain ditance to be a multiplicity menu
		QPoint p = me -> pos();
		ListPopupMenu::Menu_Type menuType = ListPopupMenu::mt_Undefined;
		int pos = m_LinePath.count() - 1;
		int DISTANCE = 40;//must be within this many pixels for it to be a multi menu
		float lengthMAP = sqrt( pow( double(m_LinePath.getPoint(0).x() - p.x()), 2.0) +
		                        pow( double(m_LinePath.getPoint(0).y() - p.y()), 2.0) );
		float lengthMBP = sqrt( pow( double(m_LinePath.getPoint(pos).x() - p.x()), 2.0) +
		                        pow( double(m_LinePath.getPoint(pos).y() - p.y()), 2.0) );
		Association_Type type = m_AssocType;
		//allow multiplicity
		if( AssocRules::allowMultiplicity( type, getWidgetA() -> getBaseType() ) ) {
			if(lengthMAP < DISTANCE)
				menuType =  ListPopupMenu::mt_MultiA;
			else if(lengthMBP < DISTANCE)
				menuType = ListPopupMenu::mt_MultiB;
		}
		if( menuType == ListPopupMenu::mt_Undefined ) {
			if( type == at_Anchor )
				menuType = ListPopupMenu::mt_Anchor;
			else if( AssocRules::allowRole( type ) )
				menuType = ListPopupMenu::mt_RoleName;
			else
				menuType = ListPopupMenu::mt_Association_Selected;
			if(type == at_Coll_Message) {
				menuType = ListPopupMenu::mt_Collaboration_Message;
			}
		}//end else
		m_pMenu = new ListPopupMenu(m_pView, menuType);
		m_pMenu->popup(me -> globalPos());
		connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
		setSelected( m_bSelected = true  );
	}//end if RightButton
	if(me->button() == LeftButton) {}//end LeftButton
}//end method mouseReleaseEvent

void AssociationWidget::slotMenuSelection(int sel) {
	QString oldText, newText;
	int result;
	bool done = false, ok;
	KLineEditDlg* dlg = 0;
	QFont font;

	//if it's a collaboration message we now just use the code in floatingtextwidget
	//this means there's some redundant code below but that's better than duplicated code
	if (m_AssocType == at_Coll_Message && sel != ListPopupMenu::mt_Delete) {
		m_pName->slotMenuSelection(sel);
		return;
	}

	switch(sel) {
	case ListPopupMenu::mt_Properties:
		if(m_AssocType == at_Seq_Message
		   || m_AssocType == at_Seq_Message_Self
			) {
			// show op dlg for seq. diagram here
			// don't worry about here, I don't think it can get here as
			// line is widget on seq. diagram
			// here just in case - remove later after testing

		} else {  //standard assoc dialog
			m_pView -> updateDocumentation( false );
			AssocPropDlg dlg(static_cast<QWidget*>(m_pView), this );
			int result = dlg.exec();
			QString name = dlg.getName();
			QString doc = dlg.getDoc(), roleADoc = dlg.getRoleADoc(), roleBDoc = dlg.getRoleBDoc();
			QString rnA = dlg.getRoleAName(), rnB = dlg.getRoleBName(),
				 ma = dlg.getMultiA(), mb = dlg.getMultiB();
			Scope vA = dlg.getVisibilityA(), vB = dlg.getVisibilityB();
			Changeability_Type cA = dlg.getChangeabilityA(), cB = dlg.getChangeabilityB();
			if(result) {
				//rules built into these functions to stop updating incorrect values
				setName(name);
				setRoleNameA(rnA);
				setRoleNameB(rnB);
				setDoc(doc);
				setRoleADoc(roleADoc);
				setRoleBDoc(roleBDoc);
				setMultiA(ma);
				setMultiB(mb);
				setVisibilityA(vA);
				setVisibilityB(vB);
				setChangeabilityA(cA);
				setChangeabilityB(cB);
				m_pView -> showDocumentation( this, true );
			}
		}
		done = true;
		break;

	case ListPopupMenu::mt_Delete://for anchor
	case ListPopupMenu::mt_Delete_Association:
	case ListPopupMenu::mt_Delete_Message:
		m_pView->removeAssoc(this);
		done = true;
		break;

	case ListPopupMenu::mt_Operation:

		m_pView->getDocument() -> createUMLObject( (UMLClassifier *)m_pWidgetB -> getUMLObject(), ListPopupMenu::convert_MT_OT((ListPopupMenu::Menu_Type)sel));
		done = true;
		break;

	case ListPopupMenu::mt_Sequence_Number:
		if(!m_pName) {
			setName("");
			// m_pName-> setUMLObject(m_pName -> getUMLObject()); // WHAT?? This CANT be right
		}
		newText = KLineEditDlg::getText(i18n("Enter sequence number"), m_pName -> getSeqNum(), &ok, m_pView);
		if(ok) {
			//Lets make sure the name text is there to set values
			//if not, set it up with default values and then put the
			//real values in.
			m_pName-> setSeqNum( newText );
			m_pName-> calculateSize();
			m_pName-> setVisible( m_pName-> getText().length() > 0 );
		}
		done = true;

		break;

	case ListPopupMenu::mt_Select_Operation:
		showOpDlg();
		done = true;
		break;

	case ListPopupMenu::mt_Rename_MultiA:
		if(m_pMultiA)
			oldText = m_pMultiA -> getText();
		else
			oldText = "";
		dlg = new KLineEditDlg( i18n("Enter multiplicity:") , oldText, m_pView);
		result = dlg ->exec();
		newText = dlg -> text();
		delete dlg;
		if( result && newText != oldText && FloatingText::isTextValid(newText) ) {
			setMultiA(newText);
		}
		break;


	case ListPopupMenu::mt_Rename_MultiB:
		if(m_pMultiB)
			oldText = m_pMultiB -> getText();
		else
			oldText = "";
		dlg = new KLineEditDlg( i18n("Enter multiplicity:") , oldText, m_pView);
		result = dlg ->exec();

		newText = dlg -> text();
		delete dlg;
		if( result && newText != oldText && FloatingText::isTextValid(newText) ) {
			setMultiB(newText);
		}
		break;

	case ListPopupMenu::mt_Rename_Name:
		if(m_pName)
			oldText = m_pName-> getText();
		else
			oldText = "";

		dlg = new KLineEditDlg( i18n("Enter association name") , oldText, m_pView);
		result = dlg ->exec();
		newText = dlg -> text();
		delete dlg;
		if( result && newText != oldText && FloatingText::isTextValid(newText) )
			setName(newText);

		break;

	case ListPopupMenu::mt_Rename_RoleAName:
		if(m_pRoleA)
			oldText = m_pRoleA -> getText();
		else
			oldText = "";
		dlg = new KLineEditDlg( i18n("Enter role name:") , oldText, m_pView);
		result = dlg ->exec();
		newText = dlg -> text();
		delete dlg;
		if( result && newText != oldText && FloatingText::isTextValid(newText) ) {
			setRoleNameA(newText);
		}
		break;

	case ListPopupMenu::mt_Rename_RoleBName:
		if(m_pRoleB)
			oldText = m_pRoleB -> getText();
		else
			oldText = "";
		dlg = new KLineEditDlg( i18n("Enter role name") , oldText, m_pView);
		result = dlg ->exec();
		newText = dlg -> text();
		delete dlg;
		if( result && newText != oldText && FloatingText::isTextValid(newText) ) {
			setRoleNameB(newText);
		}
		break;

	case ListPopupMenu::mt_Change_Font:
		font = getFont();
		if( KFontDialog::getFont( font, false, m_pView ) )
			setFont(font);
		break;

	case ListPopupMenu::mt_Change_Font_Selection:
		font = getFont();
		if( KFontDialog::getFont( font, false, m_pView ) )
			m_pView -> selectionSetFont( font );
		break;

	case ListPopupMenu::mt_Cut:
		m_pView->setStartedCut();
		m_pView->getDocument()->editCut();
		break;

	case ListPopupMenu::mt_Copy:
		m_pView->getDocument()->editCopy();
		break;

	case ListPopupMenu::mt_Paste:
		m_pView->getDocument()->editPaste();
		break;

	case ListPopupMenu::mt_Reset_Label_Positions:
		resetTextPositions();
		break;
	}//end switch
}


// utility method
void AssociationWidget::setFont (QFont font) { changeFont(font); }

// find a general font for the association
QFont AssociationWidget::getFont() {
	QFont font;

	if( m_pRoleA )
		font = m_pRoleA -> getFont( );
	else    if( m_pRoleB)
		font = m_pRoleB -> getFont( );
	else    if( m_pMultiA )
		font = m_pMultiA -> getFont( );
	else    if( m_pMultiB )
		font = m_pMultiB -> getFont( );
	else    if( m_pChangeWidgetA)
		font = m_pChangeWidgetA-> getFont( );
	else    if( m_pChangeWidgetB)
		font = m_pChangeWidgetB-> getFont( );
	else    if( m_pName)
		font = m_pName-> getFont( );
	else
		font = m_pWidgetA -> getFont();

	return font;
}

void AssociationWidget::showOpDlg() {
	if(!m_pName) {
		setName("");
		// m_pName-> setUMLObject(m_pName-> getUMLObject()); // bah, this cant be right
	}

	// CHECK THIS, WHY WigetB?? !!!
	// (I suppose because we dont have a WidgetA when in seq diagram?!?)
	UMLClassifier * c = (UMLClassifier *) m_pWidgetB -> getUMLObject();
	SelectOpDlg selectDlg(m_pView, c);

	selectDlg.setSeqNumber(m_pName-> getSeqNum());
	selectDlg.setCustomOp(m_pName-> getOperation());

	int result = selectDlg.exec();
	if( result ) {
		m_pName-> setSeqNum( selectDlg.getSeqNumber() );
		m_pName-> setOperation( selectDlg.getOpText() );
		m_pName-> calculateSize();

	}
	m_pName-> setVisible( m_pName-> getText().length() > 0 );
}

void AssociationWidget::checkPoints(QPoint p) {
	m_nMovingPoint = -1;
	//only check if more than the two endpoints
	int size = m_LinePath.count();
	if( size <= 2 )
		return;
	//check all points except the end points to se if we clicked on one of them
	QPoint tempPoint;
	int x, y;
	const int BOUNDRY = 4; //echeck for pixels around the point
	for(int i=1;i<size-1;i++) {
		tempPoint = m_LinePath.getPoint( i );
		x = tempPoint.x();
		y = tempPoint.y();
		if( x - BOUNDRY <= p.x() && x + BOUNDRY >= p.x() &&
		    y - BOUNDRY <= p.y() && y + BOUNDRY >= p.y() ) {
			m_nMovingPoint = i;
			i = size; //no need to check the rest
		}//end if
	}//end for
}

void AssociationWidget::mouseMoveEvent(QMouseEvent* me) {
	//make sure we have a moving point
	//I don't think there is another reason for being here
	if( m_nMovingPoint == -1 || me->state() != LeftButton) {
		return;
	}
	setSelected( m_bSelected = true );
	//new position for point
	QPoint p = me->pos();
	QPoint oldp = m_LinePath.getPoint(m_nMovingPoint);

	if( m_pView -> getSnapToGrid() ) {
		int newX = p.x();
		int newY = p.y();
		int gridX = m_pView -> getSnapX();
		int gridY = m_pView -> getSnapY();
		int modX = newX % gridX;
		int modY = newY % gridY;
		newX -= modX;
		newY -= modY;
		if( modX >= ( gridX / 2 ) ) {
			newX += gridX;
		}
		if( modY >= ( gridY / 2 ) ) {
			newY += gridY;
		}
		p.setX(newX);
		p.setY(newY);
	}

	//move event called now
	QMoveEvent m(p, oldp);
	moveEvent(&m);
	m_pView->resizeCanvasToItems();
}
AssociationWidget::Region AssociationWidget::getWidgetRegion(AssociationWidget * widget) {
	if(widget  -> getWidgetA()== m_pWidgetA)

		return m_WidgetARegion;


	if(widget -> getWidgetB() == m_pWidgetB)
		return m_WidgetBRegion;
	return Error;
}

int AssociationWidget::getRegionCount(AssociationWidget::Region region, bool widgetA) {
	if(region == Error)
		return 0;
	int widgetCount = 0;
	AssociationWidgetList * list = m_pView -> getAssociationWidgetList();
	AssociationWidgetListIt assoc_it(*list);
	AssociationWidget* assocwidget = 0;
	while((assocwidget = assoc_it.current())) {
		UMLWidget * a = 0, * b = 0;
		//don't count this association
		if(assocwidget != this) {
			a = assocwidget -> getWidgetA();
			b = assocwidget -> getWidgetB();

			//don't count Associations to self if both of there end points are on the same region
			//they are different and placement won't interfere with them
			bool count = true;
			if( a == b && assocwidget -> getWidgetARegion() == assocwidget -> getWidgetBRegion() )
				;
			{
				//count = false;
			}

			if( count ) {
				//if counting for widgetA
				if(widgetA) {
					if(m_pWidgetA == a) {
						if( region == assocwidget -> getWidgetARegion() )
							widgetCount++;
					} else if(m_pWidgetA == b) {
						if( region == assocwidget -> getWidgetBRegion() )
							widgetCount++;
					}
				}//end widgetA
				else {
					if(m_pWidgetB == a)
					{
						if( region == assocwidget -> getWidgetARegion() )
							widgetCount++;
					} else if(m_pWidgetB == b) {
						if( region == assocwidget -> getWidgetBRegion() )

							widgetCount++;
					}
				}
			}//end count
		}//end assocwidget != this
		++assoc_it;
	}//end while
	return widgetCount;
}

void AssociationWidget::updateAssociations(int totalCount,
					   AssociationWidget::Region region, bool widgetA)
{
	if( region == Error )
		return;
	AssociationWidgetList * list = m_pView -> getAssociationWidgetList();
	AssociationWidgetListIt assoc_it(*list);
	AssociationWidget* assocwidget = 0;
	AssociationWidget* assocwidget2 = 0;

	AssociationWidgetList ordered;
	uint counter;
	bool out;

	// we order the AssociationWidget list by region and x/y value
	while ( (assocwidget = assoc_it.current()) )
	{
		// widgetA is given as function parameter!
		if (widgetA)
		{
			// so we have to look at m_pWidgetA
			// now we must find out with which end assocwidget connects to
			// m_pWidgetA
			if ( (m_pWidgetA == assocwidget -> getWidgetA() &&
			      region == assocwidget -> getWidgetARegion()))
			{
				counter = 0;
				out = false;

				// now we go through all allready known associations and insert
				// assocwidget at the right position, so that the lines don't cross
				for (assocwidget2 = ordered.first(); assocwidget2;
				     assocwidget2 = ordered.next())
				{
					switch (region)
					{
					case North:
					case South:
						if (assocwidget2->getWidgetB()->x() >
						    assocwidget->getWidgetB()->x())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					case East:
					case West:
						if (assocwidget2->getWidgetB()->y() >
						    assocwidget->getWidgetB()->y())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					default:
						break;
					} // switch (region)
					if (out == true)
						break;
					counter++;
				} // for (assocwidget2 = ordered.first(); assocwidget2; ...
				if (out == false)
					ordered.append(assocwidget);
			} else if (m_pWidgetA == assocwidget -> getWidgetB() &&
				   region == assocwidget -> getWidgetBRegion()) {
				counter = 0;
				out = false;

				// now we go through all allready known associations and insert
				// assocwidget at the right position, so that the lines don't cross
				for (assocwidget2 = ordered.first(); assocwidget2;
				     assocwidget2 = ordered.next())
				{
					switch (region)
					{
					case North:
					case South:
						if (assocwidget2->getWidgetB()->x() >
						    assocwidget->getWidgetA()->x())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					case East:
					case West:
						if (assocwidget2->getWidgetB()->y() >
						    assocwidget->getWidgetA()->y())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					default:
						break;
					} // switch (region)
					if (out == true)
						break;
					counter++;
				} // for (assocwidget2 = ordered.first(); assocwidget2; ...
				if (out == false)
					ordered.append(assocwidget);
			} // if (m_pWidgetA == assocwidget-> getWidgetA() && region == ...
		} else {
			// so we have to look at m_pWidgetB
			// now we must find out with which end assocwidget connects to
			// m_pWidgetB
			if (m_pWidgetB == assocwidget -> getWidgetA() &&
			    region == assocwidget -> getWidgetARegion())
			{
				counter = 0;
				out = false;

				// now we go through all allready known associations and insert
				// assocwidget at the right position, so that the lines don't cross
				for (assocwidget2 = ordered.first(); assocwidget2;
				     assocwidget2 = ordered.next())
				{
					switch (region)
					{
					case North:
					case South:
						if (assocwidget2->getWidgetA()->x() >
						    assocwidget->getWidgetB()->x())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					case East:
					case West:
						if (assocwidget2->getWidgetA()->y() >
						    assocwidget->getWidgetB()->y())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					default:
						break;
					} // switch (region)
					if (out == true)
						break;
					counter++;
				} // for (assocwidget2 = ordered.first(); assocwidget2; ...
				if (out == false)
					ordered.append(assocwidget);
			} else if (m_pWidgetB == assocwidget -> getWidgetB() &&
				   region == assocwidget -> getWidgetBRegion()) {
				counter = 0;
				out = false;

				// now we go through all allready known associations and insert
				// assocwidget at the right position, so that the lines don't cross
				for (assocwidget2 = ordered.first(); assocwidget2;
				     assocwidget2 = ordered.next())
				{
					switch (region)
					{
					case North:
					case South:
						if (assocwidget2->getWidgetA()->x() >
						    assocwidget->getWidgetA()->x())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					case East:
					case West:
						if (assocwidget2->getWidgetA()->y() >
						    assocwidget->getWidgetA()->y())
						{
							ordered.insert(counter, assocwidget);
							out = true;
						}
						break;
					default:
						break;
					} // switch (region)
					if (out == true)
						break;
					counter++;
				} // for (assocwidget2 = ordered.first(); assocwidget2; ...
				if (out == false)
					ordered.append(assocwidget);
			} // if (m_pWidgetB == assocwidget -> getWidgetA() && region == ...
		} // if (widgetA)
		++assoc_it;
	} // while ( (assocwidget = assoc_it.current()) )

	// we now have an ordered list and we only have to call updateRegionLineCount
	int index = 1;
	for (assocwidget = ordered.first(); assocwidget; assocwidget = ordered.next())
	{
		if(widgetA)
		{
			if( m_pWidgetA == assocwidget -> getWidgetA() )
			{
				assocwidget -> updateRegionLineCount(index++, totalCount, region, true);
			} else if( m_pWidgetA == assocwidget -> getWidgetB() ) {
				assocwidget -> updateRegionLineCount(index++, totalCount, region, false);
			}
		} else { //end widgetA
			if(m_pWidgetB == assocwidget -> getWidgetA() )
			{
				assocwidget -> updateRegionLineCount(index++, totalCount, region, true);
			} else if(m_pWidgetB == assocwidget -> getWidgetB() ) {
				assocwidget -> updateRegionLineCount(index++, totalCount, region, false);
			}
		}
	} // for (assocwidget = ordered.first(); ...)
}

void AssociationWidget::updateRegionLineCount(int index, int totalCount, AssociationWidget::Region region , bool widgetA) {
	if( region == Error )
		return;
	QPoint pt;
	int x, y, cw, ch, ww, wh;
	UMLWidget * pWidget = m_pWidgetA;
	if( !widgetA )
		pWidget = m_pWidgetB;
	//if a fork (ActivityWidget) for widget B then all associations should meet in the middle
	if( pWidget -> getBaseType() == Uml::wt_Activity) {
		if( static_cast<ActivityWidget*>(pWidget)->getActivityType() == ActivityWidget::Fork ) {
			totalCount = 2;
			index = 1;
		}
	}
	//if the association is to self and the line ends are on the same region  use a different calculation
	if(m_pWidgetA == m_pWidgetB && m_WidgetARegion == m_WidgetBRegion) {
		x = (int)m_pWidgetA -> x();
		y = (int)m_pWidgetA -> y();
		wh = m_pWidgetA -> height();
		ww = m_pWidgetA -> width();
		int size = m_LinePath.count();
		//see if above widget ok to place assoc.
		switch( m_WidgetARegion ) {
		case North:
			m_LinePath.setPoint( 0, QPoint( x + ( ww / 4 ), y ) );
			m_LinePath.setPoint( size - 1, QPoint(x + ( ww * 3 / 4 ), y ) );
			break;

		case South:
			m_LinePath.setPoint( 0, QPoint( x + ( ww / 4 ), y + wh ) );
			m_LinePath.setPoint( size - 1, QPoint( x + ( ww * 3 / 4 ), y + wh ) );
			break;

		case East:
			m_LinePath.setPoint( 0, QPoint( x + ww, y + ( wh / 4 ) ) );
			m_LinePath.setPoint( size - 1, QPoint( x + ww, y + ( wh * 3 / 4 ) ) );
			break;

		case West:
			m_LinePath.setPoint( 0, QPoint( x, y + ( wh / 4 ) ) );
			m_LinePath.setPoint( size - 1, QPoint( x, y + ( wh * 3 / 4 ) ) );
			break;
		default:
			break;
		}//end switch
		m_OldCornerA.setX( x );
		m_OldCornerA.setY( y );
		m_OldCornerB.setX( x );
		m_OldCornerB.setY( y );

		return;
	}
	else if(widgetA) {
		m_nIndexA = index;
		m_nTotalCountA = totalCount;
		x =  (int)m_pWidgetA -> x();
		y = (int)m_pWidgetA -> y();
		ww = m_pWidgetA->width();
		wh = m_pWidgetA->height();
		ch = static_cast<int>(( wh * index / totalCount));
		cw = static_cast<int>(( ww * index / totalCount));
		m_OldCornerA.setX(x);
		m_OldCornerA.setY(y);
	} else {
		m_nIndexB = index;
		m_nTotalCountB = totalCount;
		x =  (int)m_pWidgetB -> x();
		y = (int)m_pWidgetB -> y();
		ww = m_pWidgetB->width();
		wh = m_pWidgetB->height();
		ch = static_cast<int>(( wh * index / totalCount));
		cw = static_cast<int>(( ww * index / totalCount));
		m_OldCornerB.setX(x);
		m_OldCornerB.setY(y);
	}

	switch(region) {
	case West:
		pt.setX(x);
		pt.setY(y + ch);

		break;
	case North:
		pt.setX(x + cw);
		pt.setY(y);
		break;
	case East:
		pt.setX(x + ww);
		pt.setY(y + ch);
		break;
	case South:
	case Center:

		pt.setX(x + cw);
		pt.setY(y + wh);
		break;
	default:
		break;
	}
	if(widgetA)
		m_LinePath.setPoint( 0, pt );
	else
		m_LinePath.setPoint( m_LinePath.count() - 1, pt );
}

void AssociationWidget::setSelected(bool _select) {
	if( _select ) {
		if( m_pView -> getSelectCount() == 0 )
			m_pView -> showDocumentation( this, false );
	} else
		m_pView -> updateDocumentation( true );
	m_bSelected = _select;
	m_LinePath.setSelected( _select );
	if( m_pName)
		m_pName-> setSelected( _select );
	if( m_pRoleA )
		m_pRoleA -> setSelected( _select );
	if( m_pRoleB )
		m_pRoleB -> setSelected( _select );
	if( m_pMultiA )
		m_pMultiA -> setSelected( _select );
	if( m_pMultiB )
		m_pMultiB -> setSelected( _select );
	if( m_pChangeWidgetA)
		m_pChangeWidgetA-> setSelected( _select );
	if( m_pChangeWidgetB)
		m_pChangeWidgetB-> setSelected( _select );
}

bool AssociationWidget::onAssociation(const QPoint & point) {
	return ( m_LinePath.onLinePath( point ) != -1 );
}

void AssociationWidget::slotRemovePopupMenu()
{
	if(m_pMenu) {
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}

void AssociationWidget::slotClearAllSelected() {
	setSelected( false );
}

void AssociationWidget::moveMidPointsBy( int x, int y ) {
	int pos = m_LinePath.count() - 1;
	for( int i=1 ; i < (int)pos ; i++ ) {
		QPoint p = m_LinePath.getPoint( i );
		int newX = p.x() + x;
		int newY = p.y() + y;
		if( m_pView -> getSnapToGrid() ) {
			int gridX = m_pView -> getSnapX();
			int gridY = m_pView -> getSnapY();
			int modX = newX % gridX;
			int modY = newY % gridY;
			newX -= modX;
			newY -= modY;
			if( modX >= ( gridX / 2 ) )
				newX += gridX;
			if( modY >= ( gridY / 2 ) )
				newY += gridY;
		}
		p.setX( newX );
		p.setY( newY );
		m_LinePath.setPoint( i, p );
	}
}

void AssociationWidget::moveEntireAssoc( int x, int y ) {
	//TODO: ADD SUPPORT FOR ASSOC. ON SEQ. DIAGRAMS WHEN NOTES BACK IN.
	moveMidPointsBy( x, y );
	calculateEndingPoints();
	calculateNameTextSegment();
	resetTextPositions();
}

void AssociationWidget::changeFont(QFont font) {
	if( m_pName) {
		m_pName->setFont( font );
	}
	if( m_pRoleA ) {
		m_pRoleA->setFont( font );
	}
	if( m_pRoleB ) {
		m_pRoleB->setFont( font );
	}
	if( m_pMultiA ) {
		m_pMultiA->setFont( font );
	}
	if( m_pMultiB ) {
		m_pMultiB->setFont( font );
	}
	if( m_pChangeWidgetA)
		m_pChangeWidgetA->setFont( font );
	if( m_pChangeWidgetB)
		m_pChangeWidgetB->setFont( font );
}

QRect AssociationWidget::getAssocLineRectangle()
{
	QRect rectangle;
	QPoint p;
	uint pen_width;

	/* we also want the end points connected to the other widget */
	int pos = m_LinePath.count();

	/* go through all points on the linepath */
	for( int i=0 ; i < (int) pos; i++ )
	{
		p = m_LinePath.getPoint( i );

		/* the first point is our starting point */
		if (i == 0)
		{
			rectangle.setRect(p.x(), p.y(), 0, 0);
		} else {

			/* the lines have the width of the pen */
			pen_width = m_LinePath.getPen().width();
			if (pen_width == 0)
				pen_width = 1; // width must be at least 1

			if (p.x() < rectangle.x())
				rectangle.setX(p.x());
			if (p.y() < rectangle.y())
				rectangle.setY(p.y());
			if (p.x() > rectangle.x() + rectangle.width())
				rectangle.setWidth(abs(static_cast<int>(p.x() - rectangle.x() + pen_width)));
			if (p.y() > rectangle.y() + rectangle.height())
				rectangle.setHeight(abs(static_cast<int>(p.y() - rectangle.y() + pen_width)));
		}
	}
	return rectangle;
}


void AssociationWidget::init (QWidget *parent)
{

 	// pointer to parent viewwidget object
 	m_pView = (UMLView *)parent;

 	// objects owned by this association
 	m_LinePath.setAssociation( this );
 	m_pAssociation = new UMLAssociation(m_pView->getDocument());

 	// pointers to floating text widgets objects owned by this association
 	m_pName = 0;
 	m_pChangeWidgetA = 0;
 	m_pChangeWidgetB = 0;
 	m_pMultiA = 0;
 	m_pMultiB = 0;
 	m_pRoleA = 0;
 	m_pRoleB = 0;
 	m_pWidgetA = 0;
 	m_pWidgetB = 0;

 	// associationwidget attributes
 	m_nCornerARegion = -1;
 	m_nCornerBRegion = -1;
 	m_WidgetARegion = Error;
 	m_WidgetBRegion = Error;
 	m_bActivated = false;
 	m_unNameLineSegment = 0;
 	m_pMenu = 0;
 	m_bSelected = false;
 	m_nMovingPoint = -1;

}

void AssociationWidget::resetTextPositions() {
	if (m_pMultiA) {
		setTextPosition( tr_MultiA, calculateTextPosition(tr_MultiA) );
	}
	if (m_pMultiB) {
		setTextPosition( tr_MultiB, calculateTextPosition(tr_MultiB) );
	}
	if (m_pChangeWidgetA) {
		setTextPosition( tr_ChangeA, calculateTextPosition(tr_ChangeA) );
	}
	if (m_pChangeWidgetB) {
		setTextPosition( tr_ChangeB, calculateTextPosition(tr_ChangeB) );
	}
	if (m_pName) {
		setTextPosition( tr_Name, calculateTextPosition(tr_Name) );
	}
	if (m_pRoleA) {
		setTextPosition( tr_RoleAName, calculateTextPosition(tr_RoleAName) );
	}
	if (m_pRoleB) {
		setTextPosition( tr_RoleBName, calculateTextPosition(tr_RoleBName) );
	}
}

void AssociationWidget::setWidgetA( UMLWidget* WidgetA) {
	if (!WidgetA)
		m_pWidgetA = 0;
	else {
		m_pWidgetA = WidgetA;
		m_pWidgetA->addAssoc(this);
		if(m_pAssociation)
			m_pAssociation->setObjectA(m_pWidgetA->getUMLObject());
	}
}

void AssociationWidget::setWidgetB( UMLWidget* WidgetB) {
	if (!WidgetB)
		m_pWidgetB = 0;
	else {
		m_pWidgetB = WidgetB;
		m_pWidgetB->addAssoc(this);
		if(m_pAssociation)
			m_pAssociation->setObjectB(m_pWidgetB->getUMLObject());
	}
}

#include "associationwidget.moc"
