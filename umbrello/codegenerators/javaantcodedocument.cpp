
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  This code generated by: 
 *      Author : thomas
 *      Date   : Tue Jun 24 2003
 */

#include <qregexp.h>

#include "javaantcodedocument.h"
#include "javacodegenerator.h"

#include "xmlcodecomment.h"
#include "xmlelementcodeblock.h"


// Constructors/Destructors
//  

JavaANTCodeDocument::JavaANTCodeDocument ( JavaCodeGenerator * gen ) 
    : CodeDocument ( (CodeGenerator *)gen ) 
{
	setFileName("build"); // default name
	setFileExtension(".xml");
	setID("ANTDOC"); // default id tag for this type of document
}

JavaANTCodeDocument::~JavaANTCodeDocument ( ) { }

//  
// Methods
//  

// Other methods
//  

/**
 * create a new CodeBlockWithComments object belonging to this CodeDocument.
 * @return      CodeBlockWithComments
 */
CodeBlockWithComments * JavaANTCodeDocument::newCodeBlockWithComments ( ) {
        return new XMLElementCodeBlock(this,"empty");
}

/**
 * create a new CodeBlockWithComments object belonging to this CodeDocument.
 * @return      CodeBlockWithComments
 */
CodeComment * JavaANTCodeDocument::newCodeComment ( ) {
        return new XMLCodeComment(this);
}

HierarchicalCodeBlock * JavaANTCodeDocument::newHierarchicalCodeBlock ( ) {
        return new XMLElementCodeBlock(this,"empty");
}

/** set the class attributes of this object from
 * the passed element node.
 */
void JavaANTCodeDocument::setAttributesFromNode ( QDomElement & root)
{

        // superclass save
        CodeDocument::setAttributesFromNode(root);

        // now set local attributes
// setPackage(root.attribute("package",""));

}

/**
 * load params from the appropriate XMI element node.
 */
void JavaANTCodeDocument::loadFromXMI ( QDomElement & root ) {
        setAttributesFromNode(root);
}

/** set attributes of the node that represents this class
 * in the XMI document.
 */
void JavaANTCodeDocument::setAttributesOnNode ( QDomDocument & doc, QDomElement & docElement)
{

        // superclass call
        CodeDocument::setAttributesOnNode(doc,docElement);

        // now set local attributes/fields
//FIX 
}

/**
 * Save the XMI representation of this object
 * @return      bool    status of save
 */
bool JavaANTCodeDocument::saveToXMI ( QDomDocument & doc, QDomElement & root ) {
        bool status = true;

        QDomElement docElement = doc.createElement( "codedocument" );

        setAttributesOnNode(doc, docElement);

        root.appendChild( docElement );

        return status;
}

// we add in our code blocks that describe how to generate
// the project here...
void JavaANTCodeDocument::updateContent( ) {
   	// FIX : fill in more content based on classes
	// which exist
	CodeBlockWithComments * xmlDecl = getCodeBlockWithComments("xmlDecl","",0);
	xmlDecl->setText("<?xml version=\"1.0\"?>");

	XMLElementCodeBlock * rootNode = (XMLElementCodeBlock*) getHierarchicalCodeBlock("projectDecl", "Java ANT build document", 1);
	rootNode->setNodeName("project");

// <project name="XDF" default="help" basedir=".">
	//HierarchicalCodeBlock * projDecl = xmlDecl->getHierarchicalCodeBlock("projectDecl", "Java ANT build document", 1);

	// set some global properties for the build
/*
  <!-- set global properties for this build -->
  <!-- paths -->
  <property name="docApi.dir"  value="docs/api"/>
  <property name="path" value="gov/nasa/gsfc/adc/xdf"/>
  <property name="src" value="src/${path}/"/>
  <property name="top" value="."/>
  <property name="build" value="${top}/gov"/>
  <property name="buildDir" value="${path}"/>
  <!-- compiler directives -->
  <property name="build.compiler" value="modern"/>
  <property name="useDeprecation" value="no"/>
  <property name="jarname" value="${project}.jar"/>
*/

}

// We overwritten by Java language implementation to get lowercase path
QString JavaANTCodeDocument::getPath ( )
{

        QString path = getPackage();

        // Replace all white spaces with blanks
        path.simplifyWhiteSpace();

        // Replace all blanks with underscore
        path.replace(QRegExp(" "), "_");

        path.replace(QRegExp("\\."),"/");

        path.lower();

        return path;

}


#include "javaantcodedocument.moc"
