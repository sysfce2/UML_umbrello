/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef STEREOTYPESWINDOW_H
#define STEREOTYPESWINDOW_H

#include <QDockWidget>
#include <QModelIndex>

class QTableView;

class StereotypesWindow : public QDockWidget
{
    Q_OBJECT
public:
    explicit StereotypesWindow(const QString &title, QWidget *parent = nullptr);
    ~StereotypesWindow();

Q_SIGNALS:

public Q_SLOTS:
    void modified();

protected Q_SLOTS:
    void slotStereotypesDoubleClicked(QModelIndex index);

protected:
    QTableView *m_stereotypesTree;
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // STEREOTYPESWINDOW_H
