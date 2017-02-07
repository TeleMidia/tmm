/******************************************************************************
TeleMídia Multiplexer
Copyright (C) 2016 TeleMídia Lab/PUC-Rio
https://github.com/TeleMidia/tmm-mpeg2ts/graphs/contributors

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version. This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
for more details. You should have received a copy of the GNU Affero General
Public License along with this program. If not, see http://www.gnu.org/licenses/.

*******************************************************************************/

#ifndef TMMWINDOW_H
#define TMMWINDOW_H

#include <QFile>
#include <QDomDocument>
#include <QXmlStreamReader>

#include <QVector>
#include <QMap>
#include <QProcess>
#include <QMainWindow>

#include <QStandardItemModel>
#include <QItemSelectionModel>

// #include <TreeModel.h>

#ifdef Q_WS_WIN
#define TM_MUXER_PROGRAM "tm-muxer.exe"
#else
#define TM_MUXER_PROGRAM "tm-muxer.exe"
#endif

namespace Ui {
  class TMMWindow;
}

class TMMWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TMMWindow(QWidget *parent = 0);
  ~TMMWindow();

private:
  Ui::TMMWindow *ui;
  QFile currentFile;
  bool loading;

  QProcess tmmProcess; // the process that will call tm-muxer
  QDomDocument doc;    // a pointer to current document
  QString currentSelectedUuid;
  int currentSelectedIndex;
  QStandardItemModel itemsModel, avModel;
  // TreeModel *treeModel; // \todo This could be implemented in a cleanner way using QT MVC framework

  void makeConnections();

  bool parseXML(const QString &file);

  QVector < QMap <QString, QString> > items;
  QMap < QString, QVector <QMap <QString, QString> > > avs; // mainAVs indexed by item uuid
  QMap < QString, QVector <QMap <QString, QString> > > carousels; // carossels indexed by item uuid
  QMap <QString, QString> outputParams;

  QVector <QDomElement> searchElements( const QDomElement &parent,
                                        const QString &tagname,
                                        QMap <QString, QString> &attrFilter );

  void loadGUI();
  void loadPlaylistItemsGUI();
  void closeEvent(QCloseEvent *);

public slots:
  bool selectItem(const QString &newSelectedUuid);

private slots:
  void openFile();
  void newFile();

  void loadFile(const QString& currentFile);
  bool saveModel(const QString &strFile);

  void changeCurrentItem(const QItemSelection &, const QItemSelection &);

  void createNewItem();
  void removeCurrentItem();

  void createNewAV();
  void removeCurrentAV();

  void avChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );

  void startTmm();
  void stopTmm();
  void transmissionStarted();
  void transmissionStopped();

  void itemNameChanged ( QString name );
  void itemDurChanged ( QString name );
  void carouselPathChanged ( QString name );
  void carouselBitrateChanged ( QString name );
};

#endif // TMMWindow_H
