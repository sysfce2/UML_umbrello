/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "associationwidget.h"
#include "docwindow.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlview.h"
#include "umlwidget.h"
#include <klocale.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qmultilineedit.h>

DocWindow::DocWindow( UMLDoc * doc, QWidget *parent, const char *name ) : QWidget( parent, name ) {
	//setup visual display
	QHBoxLayout * mainLayout = new QHBoxLayout( this );

	m_pDocGB = new QGroupBox( i18n( "Documentation" ), this );
	mainLayout -> addWidget( m_pDocGB );

	QHBoxLayout * docLayout = new QHBoxLayout( m_pDocGB );
	m_pDocMLE = new QMultiLineEdit( m_pDocGB );
	m_pDocMLE -> setText( "" );
	docLayout -> setMargin( fontMetrics().height() );
	docLayout -> addWidget( m_pDocMLE);
	m_pDocMLE -> setWordWrap(QMultiLineEdit::WidgetWidth);

	//setup the documentation variables
	//show projects documentation to start
	m_pUMLDoc = doc;
	m_Showing = st_Project;
	m_pUMLObject = 0;
	m_pUMLView = 0;
	m_pUMLWidget = 0;
	m_pAssocWidget = 0;
	updateDocumentation( true );
}

DocWindow::~DocWindow() {}

void DocWindow::showDocumentation( UMLObject * object, bool overwrite ) {
	if( object == m_pUMLObject && !overwrite )
		return;
	if( object != m_pUMLObject )
		updateDocumentation( true );
	m_Showing = st_UMLObject;
	if( !object ) {
		m_pDocMLE -> setText( m_pUMLDoc -> getDoc() );
		m_pUMLObject = 0;
		return;
	}
	m_pUMLObject = object;
	m_pDocMLE -> setText( m_pUMLObject -> getDoc() );
}

void DocWindow::updateDocumentation( bool clear ) {
	if( m_pUMLObject )
		m_pUMLObject -> setDoc( m_pDocMLE -> text() );
	else if( m_pUMLView )
		m_pUMLView -> setDoc( m_pDocMLE -> text() );
	else if ( m_pUMLWidget )
		m_pUMLWidget -> setDoc( m_pDocMLE -> text() );
	else if( m_pAssocWidget )
		m_pAssocWidget -> setDoc( m_pDocMLE -> text() );
	else
		m_pUMLDoc -> setDoc( m_pDocMLE -> text() );

	if( clear ) {
		m_pDocMLE -> setText( m_pUMLDoc -> getDoc() );
		m_pUMLObject = 0;
		m_pUMLView = 0;
		m_pUMLWidget = 0;
		m_pAssocWidget = 0;
		m_Showing = st_Project;
	}
}

void DocWindow::showDocumentation( UMLView * view, bool overwrite ) {
	if( view == m_pUMLView && !overwrite )
		return;
	if( view != m_pUMLView )
		updateDocumentation( true );
	m_Showing = st_UMLView;
	if( !view ) {
		m_pDocMLE -> setText( m_pUMLDoc -> getDoc() );
		m_pUMLView = 0;
		return;
	}
	m_pUMLView = view;
	m_pDocMLE -> setText( m_pUMLView -> getDoc() );
}

void DocWindow::showDocumentation( UMLWidget * widget, bool overwrite ) {
	if( widget == m_pUMLWidget && !overwrite )
		return;
	if( widget != m_pUMLWidget )
		updateDocumentation( true );
	m_Showing = st_UMLWidget;
	if( !widget ) {
		m_pDocMLE -> setText( m_pUMLDoc -> getDoc() );
		m_pUMLWidget = 0;
		return;
	}
	m_pUMLWidget = widget;
	m_pDocMLE -> setText( m_pUMLWidget -> getDoc() );
}

void DocWindow::showDocumentation( AssociationWidget * widget, bool overwrite ) {
	if( widget == m_pAssocWidget && !overwrite )
		return;
	if( widget != m_pAssocWidget )
		updateDocumentation( true );
	m_Showing = st_Association;
	if( !widget ) {
		m_pDocMLE -> setText( m_pUMLDoc -> getDoc() );
		m_pAssocWidget = 0;
		return;
	}
	m_pAssocWidget = widget;
	m_pDocMLE -> setText( m_pAssocWidget -> getDoc() );
}

void DocWindow::newDocumentation( ) {
	m_pUMLView = 0;
	m_pUMLObject = 0;
	m_pUMLWidget = 0;
	m_pAssocWidget = 0;
	m_Showing = st_Project;
	m_pDocMLE -> setText( m_pUMLDoc -> getDoc() );
}

bool DocWindow::isTyping()
{
	if (m_pDocMLE->hasFocus())
		return true;
	else
		return false;
}

#include "docwindow.moc"
