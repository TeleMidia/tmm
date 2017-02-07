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

#include "TMMWindow.h"
#include "ui_TMMWindow.h"

#include <QDebug>
#include <QProcess>
#include <QFileDialog>
#include <QTextStream>
#include <QSplitter>

#include <QFile>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QStringList>

#include <QUuid>
#include <limits.h>

#include <assert.h>

QMap <QString, QString> fromAttributesToMap(QDomElement el)
{
  QMap <QString, QString> ret;
  for(int i = 0; i < el.attributes().size(); i++)
  {
    ret.insert( el.attributes().item(i).toAttr().name(),
                el.attributes().item(i).toAttr().value() );
  }

  return ret;
}

TMMWindow::TMMWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TMMWindow)
{
  ui->setupUi(this);

  QStringList headers;
  headers << tr("Title") << tr("Description");

  itemsModel.setColumnCount(2);
  itemsModel.setHorizontalHeaderLabels( QStringList() << tr("name") << tr("uuid") );
  ui->treeView_Playlist->setModel(&this->itemsModel);
  ui->treeView_Playlist->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->treeView_Playlist->setColumnHidden(1, true);

  avModel.setHorizontalHeaderLabels( QStringList() << tr("id") << tr("pid") << tr("src") );
  avModel.setColumnCount(3);
  ui->treeView_AVs->setModel( &this->avModel );
  makeConnections();

  currentSelectedIndex = -1;
  loading = false;

  ui->spinBox_Carousel_Bitrate->setRange(0,  INT_MAX);
  ui->spinBox_Duration->setRange(0, INT_MAX);
  ui->spinBox_Output_Bitrate->setRange(0, INT_MAX);
}

TMMWindow::~TMMWindow()
{
  delete ui;
}

void TMMWindow::makeConnections()
{
  connect( ui->action_Open, SIGNAL(triggered()),
           this, SLOT(openFile()) );

  connect(ui->action_New, SIGNAL(triggered()),
          this, SLOT(newFile()));

  connect( ui->action_Quit, SIGNAL(triggered()),
           this, SLOT(close()) );

  connect( ui->action_Run, SIGNAL(triggered()),
           this, SLOT(startTmm()) );

  connect( ui->action_Stop, SIGNAL(triggered()),
           this, SLOT(stopTmm()) );

  connect( &tmmProcess, SIGNAL(started()),
           this, SLOT(transmissionStarted()) );

  connect( &tmmProcess, SIGNAL(finished(int)),
           this, SLOT(transmissionStopped()) );

  /* Item Buttons */
  connect( ui->pushButton_Add_Item, SIGNAL(pressed()),
           this, SLOT(createNewItem()) );

  connect( ui->pushButton_Remove_Item, SIGNAL(pressed()),
           this, SLOT(removeCurrentItem()) );

  /* AV Buttons */
  connect( ui->pushButton_Add_AV, SIGNAL(pressed()),
           this, SLOT(createNewAV()) );

  connect( ui->pushButton_Remove_AV, SIGNAL(pressed()),
           this, SLOT(removeCurrentAV()) );

  connect( ui->treeView_Playlist->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ),
           this, SLOT(changeCurrentItem( const QItemSelection &, const QItemSelection & )) );


  connect( ui->treeView_AVs->model(),
           SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           this,
           SLOT(avChanged(QModelIndex,QModelIndex)) );

  connect( ui->lineEdit_Item_Name,
           SIGNAL(textChanged(QString)),
           this,
           SLOT(itemNameChanged(QString)) );

  connect( ui->spinBox_Duration,
           SIGNAL(valueChanged(QString)),
           this,
           SLOT(itemDurChanged(QString)) );

  connect( ui->lineEdit_Carousel_Path,
           SIGNAL(textChanged(QString)),
           this,
           SLOT(carouselPathChanged(QString)) );

  connect( ui->spinBox_Carousel_Bitrate,
           SIGNAL(valueChanged(QString)),
           this,
           SLOT(carouselBitrateChanged(QString)) );
}

void TMMWindow::openFile()
{
  QFileDialog dialog;
  QString nextFile = dialog.getOpenFileName(this);
  loadFile(nextFile);
}

void TMMWindow::newFile()
{
  QFileDialog dialog;
  QString newFile = dialog.getSaveFileName(this);

  loadFile(newFile);
}

void TMMWindow::loadFile(const QString &currentFile)
{
  items.clear();
  avs.clear();
  carousels.clear();
  if(this->currentFile.isOpen())
    this->currentFile.close();

  this->currentFile.setFileName(currentFile);

  // Ok! I will parse the XML file.
  parseXML(currentFile);
}

void TMMWindow::startTmm()
{
  QString program = qApp->applicationDirPath().append(QDir::separator()).append(TM_MUXER_PROGRAM);
  qDebug() << program;
  QStringList args;
  args << currentFile.fileName();
  tmmProcess.startDetached(program, args);

  transmissionStarted();
  saveModel("tmp.tmm");
}

void TMMWindow::stopTmm()
{
  tmmProcess.kill();
  // qDebug() << "tmmProcess killed!";
  transmissionStopped();
}

void TMMWindow::transmissionStarted()
{
  ui->centralWidget->setEnabled(false);
  ui->action_Run->setEnabled(false);
  ui->action_Stop->setEnabled(true);
}

void TMMWindow::transmissionStopped()
{
  ui->centralWidget->setEnabled(true);
  ui->action_Run->setEnabled(true);
  ui->action_Stop->setEnabled(false);
}

bool TMMWindow::parseXML(const QString &strFile)
{
  doc.clear();

  /* We'll parse the example.xml */
  QFile file(strFile);
  if(!doc.setContent(&file))
  {
    qWarning() << this << "TMMWindow::parseXML" << "Couldn't open " << strFile;
    return false;
  }

  // get the output properties
  QDomElement output = doc.elementsByTagName("output").at(0).toElement(); // get all items
  for (int i = 0; i < output.attributes().size(); i++)
  {
    this->outputParams.insert( output.attributes().item(i).toAttr().name(),
                               output.attributes().item(i).toAttr().value() );
  }

  // Load items to maps
  QDomNodeList items = doc.elementsByTagName("item"); // get all items
  for(int i = 0; i < items.size(); i++)
  {
    QDomElement el = items.at(i).toElement();
    if(!el.isNull() && el.tagName() == "item")
    {
      QMap <QString, QString> item;
      item["uuid"] = QUuid::createUuid().toString();
      // \fixme change this for copying all attributes
      item["dur"] = el.attribute("dur");
      item["name"] = el.attribute("name");

      this->items.append(item);
      // Load pmts
      QDomNodeList children = el.childNodes();
      for(int i = 0; i < children.size(); i++)
      {
        QDomElement el = children.at(i).toElement();
        if(!el.isNull() && el.tagName() == "pmtref")
        {
          QMap <QString, QString> attrs;
          attrs.insert("id", el.attribute("pmtid"));

          // Ok! I have found a pmtref here! Let's get the associated PMT element!
          QVector <QDomElement> pmts = searchElements( doc.documentElement(),
                                                       "pmt",
                                                       attrs );
          if(pmts.size())
          {
            attrs.clear();
            // Now, let's search for the referred elementary streams!
            QVector <QDomElement> elementaryStreamsRef = searchElements( pmts.at(0),
                                                                         "es",
                                                                         attrs );
            for(int i = 0; i < elementaryStreamsRef.size(); i++)
            {
              // We have found references to elementaryStreams. Now, let's get them!
              attrs.clear();
              attrs.insert("id", elementaryStreamsRef.at(i).attribute("refid"));

              // First the audio and videos
              QVector <QDomElement> elementaryStreams = searchElements( doc.documentElement(),
                                                                        "av",
                                                                        attrs );
              if(elementaryStreams.size())
              {
                QMap <QString, QString> av = fromAttributesToMap(elementaryStreams.at(0));
                avs[item["uuid"]].append(av);
              }
            }

            // Now, it's time to search for carousels
            QVector <QDomElement> carousels = searchElements( doc.documentElement(),
                                                         "carousel",
                                                         attrs );
            QMap <QString, QString> carouselMap;
            if (carousels.size())
            {
              QDomElement carousel = carousels.at(0); // \fixme For now we only support one carousel
              carouselMap = fromAttributesToMap(carousel);
            }
            this->carousels[item["uuid"]].append(carouselMap);

            break; // \todo for now, we only support one PMT for each ITEM!
          }
        }
      }
    }
  }

  loadGUI();
  return true;
}

void TMMWindow::loadGUI()
{
  QDomElement docElem = doc.documentElement(); // root element tmm
  if(docElem.tagName() == "tmm") // Ok! Where are loading a correct file
  {
    QString newTitle = "TeleMidia Multiplexer GUI - ";
    newTitle += this->currentFile.fileName();
    setWindowTitle(newTitle);
  }

  // get output
  ui->lineEdit_Dest_Address->setText(outputParams.value("dest"));
  ui->spinBox_Output_Bitrate->setValue(outputParams.value("bitrate").toInt());

  loadPlaylistItemsGUI();
}

void TMMWindow::loadPlaylistItemsGUI()
{
  itemsModel.clear();
  itemsModel.setHorizontalHeaderLabels( QStringList() << tr("name") << tr("uuid") );
  ui->treeView_Playlist->setColumnHidden(1, true);

  //get items
  for(int i = 0; i < this->items.size(); i++)
  {
    itemsModel.appendRow( new QStandardItem(items.at(i).value("name")) );
    itemsModel.setItem(itemsModel.rowCount() - 1, 1, new QStandardItem( items.at(i).value("uuid")) );
  }
}

void TMMWindow::createNewItem()
{
  QMap <QString, QString> item;
  item["uuid"] = QUuid::createUuid().toString();

  // \fixme change this for copying all attributes
  item["dur"] = "5";
  item["name"] = "New item ...";

  items.append(item);
  QMap <QString, QString> carousel;
  carousels[item["uuid"]].append(carousel);

  // \fixme This must be incremental!!!
  loadGUI();
  selectItem(item["uuid"]);
}

void TMMWindow::removeCurrentItem()
{
  int ret = QMessageBox::warning( this,
                                  tr("Confirm item deletion?"),
                                  tr("Are you sure you want to delete the selected items?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No );

  if(ret == QMessageBox::Yes)
  {
    // Get all selections
    QModelIndexList indexes = ui->treeView_Playlist->selectionModel()->selection().indexes();
    if(indexes.size())
    {
      QModelIndex index = indexes.at(0);

      // Remove from Map
      items.remove( index.row() );

      if(!items.size()) selectItem("");

      // \fixme This must be incremental
      loadGUI();
    }
  }
}

void TMMWindow::changeCurrentItem( const QItemSelection &selected,
                                   const QItemSelection &deselected )
{
  Q_UNUSED(deselected);
  QString itemName = selected.indexes().at(1).data().toString();
  qDebug() << "Selecting item = " << itemName;

  selectItem(itemName);
}

bool TMMWindow::selectItem(const QString &newSelectedUuid)
{
  // get current item
  int itemIndex = -1;
  for (int i = 0; i < items.size(); i++)
  {
    if(items.at(i)["uuid"] == newSelectedUuid)
    {
      itemIndex = i;
      break;
    }
  }

  if(itemIndex < 0)
  {
    qWarning() << "item uuid " << newSelectedUuid << " not found!";
    currentSelectedUuid = "";

    loading = true;
    avModel.clear();
    ui->lineEdit_Item_Name->clear();
    ui->spinBox_Duration->clear();
    ui->lineEdit_Carousel_Path->clear();
    ui->spinBox_Carousel_Bitrate->clear();
    ui->groupBox_ItemDescription->setEnabled(false);
    loading = false;
    return false;
  }

  loading = true;
  currentSelectedUuid = newSelectedUuid;
  currentSelectedIndex = itemIndex;

  ui->groupBox_ItemDescription->setEnabled(true);
  ui->lineEdit_Item_Name->setText(items.at(itemIndex).value("name"));
  ui->spinBox_Duration->setValue(items.at(itemIndex).value("dur").toInt());

  //load avs
  QVector < QMap <QString, QString> > avsFromItem = avs.value(newSelectedUuid);
  qDebug() << "Selected avs" << avsFromItem;
  avModel.clear();
  avModel.setHorizontalHeaderLabels( QStringList() << tr("id") << tr("pid") << tr("src") );

  for(int i = 0; i < avsFromItem.size(); i++)
  {
    avModel.appendRow(new QStandardItem( avsFromItem.at(i).value("id") ) );
    avModel.setItem(avModel.rowCount() - 1, 1, new QStandardItem( avsFromItem.at(i).value("pid") ) );
    avModel.setItem(avModel.rowCount() - 1, 2, new QStandardItem( avsFromItem.at(i).value("src") ) );
  }

  // \fime For now we only support one carousel.
  if(carousels[currentSelectedUuid].size())
  {
    ui->lineEdit_Carousel_Path->setText( carousels[currentSelectedUuid].at(0).value("src") );
    ui->spinBox_Carousel_Bitrate->setValue( carousels[currentSelectedUuid].at(0).value("bitrate").toInt() );
  }
  else
  {
    ui->lineEdit_Carousel_Path->setText("");
    ui->spinBox_Output_Bitrate->clear();
  }

  loading = false;
  return true;
}

QVector <QDomElement> TMMWindow::searchElements( const QDomElement &parent,
                                                 const QString &tagname,
                                                 QMap <QString, QString> &attrFilter )
{
  QVector <QDomElement> ret;
  QDomNodeList nodeList = parent.elementsByTagName(tagname);

  for(int i = 0; i < nodeList.size(); i++)
  {
    QDomElement el = nodeList.at(i).toElement();
    if(!el.isNull())
    {
      bool matchedElement = true;
      foreach(QString attr, attrFilter.keys())
      {
        if(el.attribute(attr) == attrFilter.value(attr))
          continue;
        else
        {
          matchedElement = false;
          break;
        }
      }

      if(matchedElement)
        ret.append(el);
    }
  }

  return ret;
}

void TMMWindow::removeCurrentAV()
{
  int ret = QMessageBox::warning( this,
                                  tr("Confirm AV deletion?"),
                                  tr("Are you sure you want to delete the selected audio/video?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No );

  if(ret == QMessageBox::Yes)
  {
    // Get all selections
    QModelIndexList indexes = ui->treeView_AVs->selectionModel()->selection().indexes();
    if(indexes.size())
    {
      QModelIndex index = indexes.at(0);
      // Remove info from maps
      // \todo use only the models, without the maps!
      QVector <QMap <QString, QString> > avsFromItem = this->avs.value(currentSelectedUuid);
      this->avs[currentSelectedUuid].clear();

      for(int i = 0; i < avsFromItem.size(); i++)
      {
        if(i == index.row() ) continue;
        this->avs[currentSelectedUuid].append(avsFromItem.at(i));
      }
      avModel.removeRow ( index.row() );
    }
  }
}

void TMMWindow::createNewAV()
{
  QMap <QString, QString> av;
  av["id"] = "new id";
  av["src"] = "...";
  av["pid"] = "1001";

  avs[currentSelectedUuid].append(av);

  // \fixme This must be incremental!
  avModel.appendRow(new QStandardItem( av["id"] ));
  avModel.setItem(avModel.rowCount() - 1, 1, new QStandardItem( av["pid"] ) );
  avModel.setItem(avModel.rowCount() - 1, 2, new QStandardItem( av["src"] ) );
}

void TMMWindow::itemNameChanged(QString name)
{
  if(loading) return;
  items[currentSelectedIndex]["name"] = name;

  // \todo This must be incremental
  loadGUI(); // We need this to update the item list
}

void TMMWindow::itemDurChanged(QString dur)
{
  if(loading) return;
  items[currentSelectedIndex]["dur"] = dur;
  // \todo This must be incremental
  loadGUI(); // We need this to update the item list
}

void TMMWindow::carouselPathChanged(QString path)
{
  if(loading) return;
  carousels[currentSelectedUuid][0]["src"] = path;
}

void TMMWindow::carouselBitrateChanged(QString bitrate)
{
  if(loading) return;
  carousels[currentSelectedUuid][0]["bitrate"] = bitrate;
}

void TMMWindow::avChanged ( const QModelIndex & topLeft,
                            const QModelIndex & bottomRight )
{
  Q_UNUSED(topLeft);
  QString value = bottomRight.model()->data(bottomRight).toString();
  if(this->avs.contains(currentSelectedUuid))
  {
    switch (bottomRight.column())
    {
      case 0:
        this->avs[currentSelectedUuid][ bottomRight.row() ]["id"] = value;
        break;
      case 1:
        this->avs[currentSelectedUuid][ bottomRight.row() ]["pid"] = value;
        break;
      case 2:
        this->avs[currentSelectedUuid][ bottomRight.row() ]["src"] = value;
        break;
    }
  }
}

bool TMMWindow::saveModel(const QString &strFile)
{
  QDomDocument doc;
  QDomElement root = doc.createElement("tmm");
  doc.appendChild(root);

  QDomElement head = doc.createElement("head");
  root.appendChild(head);

  QDomElement body = doc.createElement("body");
  root.appendChild(body);

  QDomElement inputs = doc.createElement("inputs");
  body.appendChild(inputs);

  QDomElement output = doc.createElement("output");
  output.setAttribute("dest", ui->lineEdit_Dest_Address->text());
  output.setAttribute("bitrate", ui->spinBox_Output_Bitrate->text());
  // \todo Other parameters
  body.appendChild(output);

  // Add items to output
  for(int i = 0; i < items.size(); i++)
  {
    QDomElement item = doc.createElement("item");
    item.setAttribute("name", items.at(i).value("name"));
    item.setAttribute("dur", items.at(i).value("dur"));
    output.appendChild(item);

    // Create AVS
    // \fixme For now we only support one PMT for each ITEM.
    QDomElement pmtref = doc.createElement("pmtref");
    pmtref.setAttribute("pmtid", "pmt" + i);
    item.appendChild(pmtref);

    QDomElement pmt = doc.createElement("pmt");
    pmt.setAttribute("id", "pmt" + i);
    inputs.appendChild(pmt);

    for(int j = 0; j < avs[items[i]["uuid"]].size(); j++)
    {
      QDomElement es = doc.createElement("es");
      es.setAttribute("refid", avs[items[i]["uuid"]].at(j)["id"]);
      pmt.appendChild(es);

      QDomElement av = doc.createElement("av");
      av.setAttribute("id", avs[items[i]["uuid"]].at(j)["id"]);
      av.setAttribute("src", avs[items[i]["uuid"]].at(j)["src"]);
      av.setAttribute("pid", avs[items[i]["uuid"]].at(j)["pid"]);
      inputs.appendChild(av);
    }

    for(int j = 0; j < carousels[items[i]["uuid"]].size(); j++)
    {
      QDomElement es = doc.createElement("es");
      es.setAttribute("refid", "carousel" + i); // \fixme This work only beacause we support only one carousel for each pmt
      pmt.appendChild(es);

      QDomElement carousel = doc.createElement("carousel");
      carousel.setAttribute("id", "carousel" + i);
      carousel.setAttribute("src", carousels[items[i]["uuid"]].at(j)["src"]);
      carousel.setAttribute("bitrate", carousels[items[i]["uuid"]].at(j)["bitrate"]);
      inputs.appendChild(carousel);
    }
  }

  QFile file(strFile);
  if(file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QTextStream out(&file);
    out << doc.toString();
    file.close();
    return true;
  }
  else
    return false;
}

void TMMWindow::closeEvent(QCloseEvent *event)
{
  if(currentFile.fileName() != "")
  {
    int ret = QMessageBox::warning( this,
                                    tr("Do you want to save your changes?"),
                                    tr("Do you want to save your changes to %1 file?").arg(currentFile.fileName()),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No );

    if(ret == QMessageBox::Yes)
    {
      saveModel(currentFile.fileName());
      QWidget::closeEvent(event);
    }
   }
}
