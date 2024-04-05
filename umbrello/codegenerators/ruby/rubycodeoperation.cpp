/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "rubycodeoperation.h"

// local includes
#include "debug_utils.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodedocumentation.h"
#include "rubycodegenerator.h"
#include "uml.h"

// qt includes
#include <QRegularExpression>

RubyCodeOperation::RubyCodeOperation (RubyClassifierCodeDocument * doc, UMLOperation *parent, const QString & body, const QString & comment)
        : CodeOperation (doc, parent, body, comment)
{
    // lets not go with the default comment and instead use
    // full-blown ruby documentation object instead
    setComment(new RubyCodeDocumentation(doc));

    // these things never change..
    setOverallIndentationLevel(1);
}

RubyCodeOperation::~RubyCodeOperation()
{
}

// we basically want to update the doc and start text of this method
void RubyCodeOperation::updateMethodDeclaration()
{
    CodeDocument * doc = getParentDocument();
    RubyClassifierCodeDocument * rubydoc = dynamic_cast<RubyClassifierCodeDocument*>(doc);
    Q_ASSERT(rubydoc);
    UMLClassifier *c = rubydoc->getParentClassifier();
    UMLOperation * o = getParentOperation();
    bool isInterface = rubydoc->getParentClassifier()->isInterface();
    QString endLine = getNewLineEndingChars();

    // now, the starting text.
    //:UNUSED: QString strVis = o->visibility().toString();
    // no return type for constructors
    QString fixedReturn = RubyCodeGenerator::cppToRubyType(o->getTypeName());
    QString returnType = o->isConstructorOperation() ? QString() : (fixedReturn + QString(QStringLiteral(" ")));
    QString methodName = o->name();

    QString RubyClassName = rubydoc->getRubyClassName(c->name());

    // Skip destructors, and operator methods which
    // can't be defined in ruby
    if (methodName.startsWith(QLatin1Char('~'))
         || QRegularExpression(QStringLiteral("operator\\s*(=|--|\\+\\+|!=)$")).exactMatch(methodName))
    {
        getComment()->setText(QString());
        return;
    }

    if (RubyClassName == methodName) {
        methodName = QStringLiteral("initialize");
    }

    methodName.remove(QRegularExpression(QStringLiteral("operator\\s*")));
    methodName = methodName.mid(0, 1).toLower() + methodName.mid(1);

    QString paramStr;
    QStringList commentedParams;

    // assemble parameters
    UMLAttributeList list = getParentOperation()->getParmList();
    int nrofParam = list.count();
    int paramNum = 0;
    for(UMLAttribute *parm : list) {
        QString paramName = RubyCodeGenerator::cppToRubyName(parm->name());
        paramStr += paramName;
        if (! parm->getInitialValue().isEmpty()) {
            paramStr += QString(QStringLiteral(" = ")) + RubyCodeGenerator::cppToRubyType(parm->getInitialValue());
        }
        paramNum++;

        if (paramNum != nrofParam)
            paramStr  += QStringLiteral(", ");
    }

    QString startText;
    if (isInterface) {
        // Assume 'isInterface' means a module in Ruby, so
        // generate module methods
        startText = QStringLiteral("def ") + RubyClassName + QLatin1Char('.') + methodName + QLatin1Char('(') + paramStr + QLatin1Char(')');
    } else {
        startText = QStringLiteral("def ")+ methodName + QLatin1Char('(') + paramStr + QLatin1Char(')');
    }

    // startText += "";  ???
    setEndMethodText(QStringLiteral("end"));

    setStartMethodText(startText);

    // Lastly, for text content generation, we fix the comment on the
    // operation, IF the codeop is autogenerated & currently empty
    QString comment = o->doc();

    if (comment.isEmpty()) {
        if (contentType() == CodeBlock::AutoGenerated) {
            UMLAttributeList parameters = o->getParmList();
            for(UMLAttribute *currentAtt : parameters) {
                comment += endLine + QStringLiteral("* _") + currentAtt->name() + QStringLiteral("_ ");
                comment += (QLatin1Char(' ') + currentAtt->doc().replace(QRegularExpression(QStringLiteral("[\\n\\r]+[\\t ]*")), endLine + QStringLiteral("  ")));
            }
            // add a returns statement too
            if (!returnType.isEmpty() && !QRegularExpression(QStringLiteral("^void\\s*$")).exactMatch(returnType))
                comment += endLine + QStringLiteral("* _returns_ ") + returnType + QLatin1Char(' ');
            getComment()->setText(comment);
        }
    } else {
        comment.replace(QRegularExpression(QStringLiteral("[\\n\\r]+ *")), endLine);
        comment.replace(QRegularExpression(QStringLiteral("[\\n\\r]+\\t*")), endLine);

        comment.replace(QStringLiteral(" m_"), QStringLiteral(" "));
        comment.replace(QRegularExpression(QStringLiteral("\\s[npb](?=[A-Z])")), QStringLiteral(" "));
        QRegularExpression re_params(QStringLiteral("@param (\\w)(\\w*)"));
        int pos = re_params.indexIn(comment);
        while (pos != -1) {
            comment.replace(re_params.cap(0),
                            QString(QStringLiteral("@param _")) + re_params.cap(1).toLower() + re_params.cap(2) + QLatin1Char('_'));
            commentedParams.append(re_params.cap(1).toLower() + re_params.cap(2));

            pos += re_params.matchedLength() + 3;
            pos = re_params.indexIn(comment, pos);
        }

        UMLAttributeList parameters = o->getParmList();
        for(UMLAttribute *currentAtt : parameters) {
            // Only write an individual @param entry if one hasn't been found already
            // in the main doc comment
            if (commentedParams.contains(RubyCodeGenerator::cppToRubyName(currentAtt->name())) == 0) {
                comment += (endLine + QStringLiteral("@param _") + RubyCodeGenerator::cppToRubyName(currentAtt->name()) + QLatin1Char('_'));
                if (currentAtt->doc().isEmpty()) {
                    comment += (QLatin1Char(' ') + RubyCodeGenerator::cppToRubyType(currentAtt->getTypeName()));
                } else {
                    comment += (QLatin1Char(' ') + currentAtt->doc().replace(QRegularExpression(QStringLiteral("[\\n\\r]+[\\t ]*")), endLine + QStringLiteral("  ")));
                }
            }
        }

        comment.remove(QStringLiteral("@ref "));
        comment.replace(QStringLiteral("@param"), QStringLiteral("*"));
        comment.replace(QStringLiteral("@return"), QStringLiteral("* _returns_"));

        // All lines after the first one starting with '*' in the doc comment
        // must be indented correctly. If they aren't a list
        // item starting with '*', then indent the text with
        // two spaces, '  ', to line up with the list item.
        pos = comment.indexOf(endLine + QLatin1Char('*'));
        if (pos != -1) {
            pos += endLine.length() + 1;
            pos = comment.indexOf(endLine, pos);
        }

        while (pos > 0) {
            pos += endLine.length();
            if (comment[pos] != QLatin1Char('*')) {
                comment.insert(pos, QStringLiteral("  "));
                pos += 2;
            }

            pos = comment.indexOf(endLine, pos);
        }

        QString typeStr = RubyCodeGenerator::cppToRubyType(o->getTypeName());
        if (!typeStr.isEmpty()
                && !QRegularExpression(QStringLiteral("^void\\s*$")).exactMatch(typeStr)
                && comment.contains(QStringLiteral("_returns_")) == 0)
        {
            comment += endLine + QStringLiteral("* _returns_ ") + typeStr;
        }

        getComment()->setText(comment);
    }

    // In Java, for interfaces..we DON'T write out non-public
    // method declarations. And for Ruby modules?
    if (isInterface) {
        UMLOperation * o = getParentOperation();
        if (o->visibility() != Uml::Visibility::Public)
            setWriteOutText(false);
    }

}

int RubyCodeOperation::lastEditableLine()
{
    ClassifierCodeDocument * doc = dynamic_cast<ClassifierCodeDocument*>(getParentDocument());

    // Check for dynamic casting failure
    if (doc == 0)
    {
        logError0("RubyCodeOperation::lastEditableLine doc: invalid dynamic cast");
        return -1;
    }

    if (doc->parentIsInterface())
        return -1; // very last line is NOT editable as its a one-line declaration w/ no body in
    // an interface.
    return 0;
}

