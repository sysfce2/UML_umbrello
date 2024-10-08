/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLAPPPRIVATE_H
#define UMLAPPPRIVATE_H

// app includes
#include "cmds.h"
#include "finddialog.h"
#include "findresults.h"
#include "uml.h"
#include "umldoc.h"
#include "diagramswindow.h"
#include "objectswindow.h"
#include "stereotypeswindow.h"

// kde includes
#include <KActionCollection>
#include <KToggleAction>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

// qt includes
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QListWidget>
#include <QObject>
#ifdef WEBKIT_WELCOMEPAGE
#include <QWebView>
#else
#include <QTextBrowser>
#endif

class QWidget;


/**
 * Class UMLAppPrivate holds private class members/methods
 * to reduce the size of the public class and to speed up
 * recompiling.
 * The migration to this class is not complete yet.
 **/
class UMLAppPrivate : public QObject
{
    Q_OBJECT
public:
    UMLApp *parent;
    FindDialog findDialog;
    FindResults findResults;
    QListWidget *logWindow;         ///< Logging window.
    KToggleAction *viewDiagramsWindow;
    KToggleAction *viewObjectsWindow;
    KToggleAction *viewStereotypesWindow;
    KToggleAction *viewWelcomeWindow;
    DiagramsWindow *diagramsWindow;
    ObjectsWindow *objectsWindow;
    StereotypesWindow *stereotypesWindow;
    QDockWidget *welcomeWindow;
    QDockWidget *editorWindow;

    KTextEditor::Editor *editor;
    KTextEditor::View *view;
    KTextEditor::Document *document;

    explicit UMLAppPrivate(UMLApp *_parent)
      : parent(_parent),
        findDialog(_parent),
        viewDiagramsWindow(nullptr),
        viewObjectsWindow(nullptr),
        viewStereotypesWindow(nullptr),
        viewWelcomeWindow(nullptr),
        diagramsWindow(nullptr),
        objectsWindow(nullptr),
        stereotypesWindow(nullptr),
        welcomeWindow(nullptr),
        editorWindow(nullptr),
        view(nullptr),
        document(nullptr)
    {
        /* TODO: On the call to KTextEditor::Editor::instance() Valgrind reports
           "Conditional jump or move depends on uninitialised value(s)".
         */
        editor = KTextEditor::Editor::instance();
        logWindow = new QListWidget;
        QFont mono;
        mono.setFamily(QStringLiteral("Monospace"));
        logWindow->setFont(mono);
        connect(logWindow, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(slotLogWindowItemDoubleClicked(QListWidgetItem *)));
    }

    ~UMLAppPrivate()
    {
        delete diagramsWindow;
        delete editorWindow;
        delete logWindow;
        delete objectsWindow;
        delete stereotypesWindow;
        delete welcomeWindow;
    }

    bool openFileInEditor(const QUrl &file, int startCursor=0, int endCursor=0);

public Q_SLOTS:
    void slotLogWindowItemDoubleClicked(QListWidgetItem *item)
    {
        QStringList columns = item->text().split(QChar::fromLatin1(':'));

        openFileInEditor(QUrl::fromLocalFile(columns[0]), columns[1].toInt()-1, columns[2].toInt());
    }

    void createDiagramsWindow()
    {
        // create the tree viewer
        diagramsWindow = new DiagramsWindow(i18n("&Diagrams"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, diagramsWindow);

        viewDiagramsWindow = parent->actionCollection()->add<KToggleAction>(QStringLiteral("view_show_diagrams"));
        connect(viewDiagramsWindow, SIGNAL(triggered(bool)), diagramsWindow, SLOT(setVisible(bool)));
    }

    void createObjectsWindow()
    {
        // create the object window
        objectsWindow = new ObjectsWindow(i18n("&UML Objects"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, objectsWindow);

        viewObjectsWindow = parent->actionCollection()->add<KToggleAction>(QStringLiteral("view_show_objects"));
        connect(viewObjectsWindow, SIGNAL(triggered(bool)), objectsWindow, SLOT(setVisible(bool)));
    }

    void createStereotypesWindow()
    {
        // create the tree viewer
        stereotypesWindow = new StereotypesWindow(i18n("&Stereotypes"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, stereotypesWindow);

        viewStereotypesWindow = parent->actionCollection()->add<KToggleAction>(QStringLiteral("view_show_stereotypes"));
        connect(viewStereotypesWindow, SIGNAL(triggered(bool)), stereotypesWindow, SLOT(setVisible(bool)));
    }

    void createWelcomeWindow()
    {
        QString file = findWelcomeFile();
        if (file.isEmpty())
            return;
        QString html = readWelcomeFile(file);
        if (html.isEmpty())
            return;
        // qDebug() << html;
        welcomeWindow = new QDockWidget(i18n("Welcome"), parent);
        welcomeWindow->setObjectName(QStringLiteral("WelcomeDock"));
#ifdef WEBKIT_WELCOMEPAGE
        QWebView *view = new QWebView;
        view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
        view->setContextMenuPolicy(Qt::NoContextMenu);
        connect(view, SIGNAL(linkClicked(const QUrl)), this, SLOT(slotWelcomeWindowLinkClicked(const QUrl)));
        view->setHtml(html);
        view->show();
        welcomeWindow->setWidget(view);
#else
        QTextBrowser *tb = new QTextBrowser(dynamic_cast<QWidget*>(this));
        tb->setOpenExternalLinks(true);
        tb->setOpenLinks(false);
        tb->setHtml(html);
        connect(tb, SIGNAL(anchorClicked(const QUrl)), this, SLOT(slotWelcomeWindowLinkClicked(const QUrl)));
        welcomeWindow->setWidget(tb);
#endif
        parent->addDockWidget(Qt::RightDockWidgetArea, welcomeWindow);

        viewWelcomeWindow = parent->actionCollection()->add<KToggleAction>(QStringLiteral("view_show_welcome"));
        connect(viewWelcomeWindow, SIGNAL(triggered(bool)), welcomeWindow, SLOT(setVisible(bool)));
    }

    void slotWelcomeWindowLinkClicked(const QUrl &url)
    {
        //qDebug() << url;
        if (url.scheme() == QStringLiteral("mailto") || url.scheme().startsWith(QStringLiteral("http"))) {
            QDesktopServices::openUrl(url);
            return;
        }
        QStringList list = url.toString().split(QLatin1Char('-'));
        list.removeLast();
        QString key;
        for(const QString &s: list) {
            QString a = s;
            a[0] = a[0].toUpper();
            key.append(a);
        }
        Uml::DiagramType::Enum type = Uml::DiagramType::fromString(key);
        if (type == Uml::DiagramType::Undefined)
            return;
        QString diagramName = UMLApp::app()->document()->createDiagramName(type);
        if (!diagramName.isEmpty())
            UMLApp::app()->executeCommand(new Uml::CmdCreateDiagram(UMLApp::app()->document(), type, diagramName));
    }

private:
    QString findWelcomeFile();
    QString readWelcomeFile(const QString &file);
};

#endif
