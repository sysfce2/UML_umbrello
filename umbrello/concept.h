/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONCEPT_H
#define CONCEPT_H

#include "umlcanvasobject.h"
#include <qptrlist.h>

class IDChangeLog;
class UMLAssociation;
class UMLAttribute;
class UMLOperation;
class UMLTemplate;

/**
 *	This class contains the non-graphical information required for a UML Concept (ie a class).
 *	This class inherits from @ref UMLCanvasObject which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Concepts will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short	Information for a non-graphical Concept/Class.
 *	@author Paul Hensgen	<phensgen@techie.com>
 */

class UMLConcept : public UMLCanvasObject {
public:
	/**
	 *	Sets up a Concept.
	 *
	 *	@param	parent	The parent to this Concept.
	 *	@param	name	The name of the Concept.
	 *	@param	id	The unique id of the Concept.
	 */
	UMLConcept(QObject* parent, QString Name, int id);

	/**
	 *	Sets up a Concept.
	 *
	 *	@param	parent		The parent to this Concept.
	 */
	UMLConcept(QObject* parent);

	/**
	 *	Standard deconstructor.
	 */
	virtual ~UMLConcept();

	/**
	 * 		Overloaded '==' operator
	 */
  	bool operator==( UMLConcept & rhs );
  
 	/**
  	 *	Adds an attribute to the Concept.
  	 *
  	 *	@param	name	The name of the Attribute.
	 *	@param	id			The id of the Attribute.
	 */
	UMLObject* addAttribute(QString name, int id);

	/**
	 * Adds an already created attribute, the attribute object must not belong to any other
	 *	concept
	 */
	bool addAttribute(UMLAttribute* Att, IDChangeLog* Log = 0);

	/**
	 *	Removes an attribute from the Concept.
	 *
	 *	@param	a	The attribute to remove.
	 */
	int removeAttribute(UMLObject *a);

 	/**
  	 *	Adds a template to the Concept.
  	 *
  	 *	@param	name	The name of the template.
	 *	@param	id	The id of the template.
	 */
	UMLObject* addTemplate(QString name, int id);

	/**
	 * Adds an already created template, the template object must not belong to any other
	 *	concept
	 */
	bool addTemplate(UMLTemplate* newTemplate, IDChangeLog* log = 0);

	/**
	 *	Removes a template from the Concept.
	 *
	 *	@param	a	The template to remove.
	 */
	int removeTemplate(UMLTemplate* a);

	/**
	 *	Adds an operation to the Concept.
	 *
	 *	@param	name	The name of the operation.
	 *	@param	id			The id of the operation.
	 */
	UMLObject* addOperation(QString name, int id);

	/**
	 *	Adds an already created Operation. The Operation must not belong to any other
	 *	concept
	 */
	bool addOperation(UMLOperation* Op);

	/**
	 *	Adds an already created Operation and checks for
	 *	operations with the same name. The Operation must not
	 *	belong to any other concept.  Used by the clipboard
	 *	when pasteing.
	 */
	bool addOperation(UMLOperation* Op, IDChangeLog* Log);

	/**
	 *	Remove an operation from the Concept.
	 *
	 *	@param	o	The operation to remove.
	 */
	int removeOperation(UMLObject *o);

	/**
	 *	Returns the number of attributes for the Concept.
	 *
	 *	@return	The number of attributes for the Concept.
	 */
	int attributes();

	/**
	 *	Returns the number of templates for the Concept.
	 *
	 *	@return	The number of templates for the Concept.
	 */
	int templates();

	/**
	 *	Returns the number of operations for the Concept.
	 *
	 *	@return	The number of operations for the Concept.
	 */
	int operations() ;

	/**
	 *	Return the list of attributes for the Concept.
	 *
	 *	@return The list of attributes for the Concept.
	 */
	QPtrList<UMLAttribute>* getAttList();

	/**
	 *	Return the list of templates for the Concept.
	 *
	 *	@return The list of templates for the Concept.
	 */
	QPtrList<UMLTemplate>* getTemplateList();

	/**
	 *	Return the list of operations for the Concept.
	 *
	 *	@return The list of operation for the Concept.
	 */
	QPtrList<UMLOperation>* getOpList();

	/**
	 * Returns a name for the new association, operation, template
	 * or attribute appended with a number if the default name is
	 * taken e.g. new_association, new_association_1 etc
	 */
	virtual QString uniqChildName(const UMLObject_Type type);

	/**
	 *	Find a list of attributes, operations, associations or
	 *	templates with the given name.
	 *
	 *	@param	t	The type to find.
	 *	@param	n	The name of the object to find.
	 *
	 *	@return	The object found.  Will return 0 if none found.
	 */
	QPtrList<UMLObject> findChildObject(UMLObject_Type t, QString n);

	/**
	 *	Find an attribute, operation, association or template.
	 *
	 *	@param	id	The id of the object to find.
	 *
	 *	@return	The object found.  Will return 0 if none found.
	 */
	UMLObject* findChildObject(int id);

	/**
	 * Use to save or load this classes information
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 * load the information.
	 *	@param	fileversion	the version of the serialize format
	 *
	 *	@return	Returns the result of the operation.
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 * Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	/**
	 *	Initializes key variables of the class.
	 */
	void init();

	/**
	 * Creates the UML:Class element including it's operations,
	 * attributes and templates
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the UML:Class element including it's operations,
	 * attributes and templates
	 */
	bool loadFromXMI( QDomElement & element );
	
	/** reimplemented from UMLObject*/
	virtual bool acceptAssociationType(Uml::Association_Type);

private:

	/**
	 * 	List of all the operations in this class.
	 */
	QPtrList<UMLOperation> m_OpsList;

	/**
	 * 	List of all the attributes in this class.
	 */
	QPtrList<UMLAttribute> m_AttsList;

	/**
	 * 	List of all the templates in this class.
	 */
	QPtrList<UMLTemplate> m_TemplateList;
};

#endif
