
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
 *      Date   : Thu Jun 19 2003
 */

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qregexp.h>

#include "javacodegenerator.h"
#include "javacodecomment.h"
#include "javacodeclassfield.h"
#include "javacodeclassfielddeclarationblock.h"
#include "codeviewerdialog.h"

const bool JavaCodeGenerator::DEFAULT_BUILD_ANT_DOC = false;

// Constructors/Destructors
//

JavaCodeGenerator::JavaCodeGenerator ( UMLDoc * parentDoc , const char * name, QDomElement & elem )
    : CodeGenerator ( parentDoc , name)
{
	initFields();
	loadFromXMI(elem);
}

JavaCodeGenerator::JavaCodeGenerator ( UMLDoc * parentDoc, const char * name )
    : CodeGenerator ( parentDoc , name)
{
	initFields();
}

JavaCodeGenerator::~JavaCodeGenerator ( ) { }

//
// Methods
//

// Accessor methods
//

// return our language
QString JavaCodeGenerator::getLanguage() {
	return "Java";
}

/**
 * Set the value of m_createANTBuildFile
 * @param new_var the new value of m_createANTBuildFile
 */
void JavaCodeGenerator::setCreateANTBuildFile ( bool buildIt) {
        m_createANTBuildFile = buildIt;
        CodeDocument * antDoc = findCodeDocumentByID("ANTDOC");
	if (antDoc)
		antDoc->setWriteOutCode(buildIt);
}

/**
 * Get the value of m_createANTBuildFile
 * @return the value of m_createANTBuildFile
 */
bool JavaCodeGenerator::getCreateANTBuildFile ( ) {
        return m_createANTBuildFile;
}

// In the Java version, we make the ANT build file also available.
CodeViewerDialog * JavaCodeGenerator::getCodeViewerDialog ( QWidget* parent, CodeDocument *doc,
                                                            Settings::CodeViewerState state)
{
	CodeViewerDialog *dialog = new CodeViewerDialog(parent, doc, state);
	if(getCreateANTBuildFile())
		dialog->addCodeDocument(findCodeDocumentByID("ANTDOC"));
	return dialog;
}


void JavaCodeGenerator::setPolicy ( CodeGenerationPolicy* policy )
{
	JavaCodeGenerationPolicy * jpolicy = dynamic_cast<JavaCodeGenerationPolicy*>(policy);
	CodeGenerator::setPolicy(policy);
	setJavaPolicy(jpolicy);
}

void JavaCodeGenerator::setJavaPolicy( JavaCodeGenerationPolicy * policy) {
        m_javacodegenerationpolicy = policy;
}

JavaCodeGenerationPolicy * JavaCodeGenerator::getJavaPolicy() {
        return m_javacodegenerationpolicy;
}

JavaCodeGenerationPolicy::JavaCommentStyle JavaCodeGenerator::getCommentStyle ( )
{
	return getJavaPolicy()->getCommentStyle();
}

bool JavaCodeGenerator::getAutoGenerateConstructors ( )
{
	return getJavaPolicy()->getAutoGenerateConstructors();
}

bool JavaCodeGenerator::getAutoGenerateAttribAccessors ( )
{
        return getJavaPolicy()->getAutoGenerateAttribAccessors ();
}

bool JavaCodeGenerator::getAutoGenerateAssocAccessors ( )
{
        return getJavaPolicy()->getAutoGenerateAssocAccessors ();
}

QString JavaCodeGenerator::getListFieldClassName () {
        return QString("Vector");
}

// Other methods
//

QString JavaCodeGenerator::capitalizeFirstLetter(QString string)
{
        // we could lowercase everything tostart and then capitalize? Nah, it would
        // screw up formatting like getMyRadicalVariable() to getMyradicalvariable(). Bah.
        QChar firstChar = string.at(0);
        string.replace( 0, 1, firstChar.upper());
        return string;
}

// IF the type is "string" we need to declare it as
// the Java Object "String" (there is no string primative in Java).
// Same thing again for "bool" to "boolean"
QString JavaCodeGenerator::fixTypeName(QString string)
{
        string.replace(QRegExp("^string$"),"String");
        string.replace(QRegExp("^bool$"),"boolean");
        return cleanName(string);
}

/**
 * @return      JavaANTCodeDocument
 */
JavaANTCodeDocument * JavaCodeGenerator::newANTCodeDocument ( ) {
	return new JavaANTCodeDocument(this);
}

CodeGenerationPolicy * JavaCodeGenerator::newCodeGenerationPolicy( KConfig * config)
{
	CodeGenerationPolicy * myPolicy = new JavaCodeGenerationPolicy(this,config);
	return myPolicy;
}

/**
 * @return      ClassifierCodeDocument
 * @param       classifier
 */
CodeDocument * JavaCodeGenerator::newClassifierCodeDocument ( UMLClassifier * c)
{
        JavaClassifierCodeDocument * doc = new JavaClassifierCodeDocument(c,this);
        return doc;
}

CodeComment * JavaCodeGenerator::newCodeComment ( CodeDocument * doc) {
        return new JavaCodeComment(doc);
}

void JavaCodeGenerator::initFields ( ) {

	setPolicy ( new JavaCodeGenerationPolicy(this, getPolicy()) );

	// load Classifier documents from parent document
	initFromParentDocument();

	// add in an ANT document
        JavaANTCodeDocument * buildDoc = newANTCodeDocument( );
        addCodeDocument(buildDoc);

	// set our 'writeout' policy for that code document
        setCreateANTBuildFile(DEFAULT_BUILD_ANT_DOC);

}

void JavaCodeGenerator::createDefaultDatatypes() {
	m_document->createDatatype("int");
	m_document->createDatatype("char");
	m_document->createDatatype("boolean");
	m_document->createDatatype("float");
	m_document->createDatatype("double");
	m_document->createDatatype("byte");
	m_document->createDatatype("short");
	m_document->createDatatype("string");
}

#include "javacodegenerator.moc"
