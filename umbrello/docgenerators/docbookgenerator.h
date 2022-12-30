/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2006 Gael de Chalendar (aka Kleag) kleag@free.fr
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DOCBOOKGENERATOR_H
#define DOCBOOKGENERATOR_H

#include <QtGlobal>

#include <QObject>
#include <QUrl>

class UMLDoc;
class DocbookGeneratorJob;

/**
 * class DocbookGenerator is a documentation generator for UML documents.
 * It uses libxslt to convert the XMI generated by UMLDoc::saveToXMI1 through
 * the XSLT file stored in resources.
 *
 * @todo Add configure checks for libxml2 and libxslt and use conditional
 * compilation of this library and its callers
 * @todo allow to specify the destination and ensure that it works with distant
 * ones
 */
class DocbookGenerator : public QObject
{
    Q_OBJECT
  public:

    DocbookGenerator();
    virtual ~DocbookGenerator();

    bool generateDocbookForProject();
    void generateDocbookForProjectInto(const QUrl& destDir);
    static QString customXslFile();

signals:
    void finished(bool status);

  private slots:

    void slotDocbookGenerationFinished(const QString&);

    void threadFinished();

  private:

    DocbookGeneratorJob* docbookGeneratorJob;

    bool m_pStatus;
    bool m_pThreadFinished;
    QUrl m_destDir;
    UMLDoc* umlDoc;
};

#endif // DOCBOOKGENERATOR_H
