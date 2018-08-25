// Software developed for the Spring 2016 Brown University course
// ENGN 2502 3D Photography
// Copyright (c) 2016, Daniel Moreno and Gabriel Taubin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Brown University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL DANIEL MORENO NOR GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gui_util.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QListView>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDir>

#include "Application.hpp"

int gui_util::fillCombo
(QComboBox *combo, const QStringList &list, const QString &defaultSelection)
{
  //save current value
  auto currentText = combo->currentText();

  //block signals
  combo->blockSignals(true);

  //clear 
  combo->clear();

  //update
  combo->addItems(list);

  //set current
  bool useDefault = currentText.isEmpty();
  if (!useDefault)
  { //search for the previous value
    auto index = combo->findText(currentText);
    useDefault = (index < 0);
    if (!useDefault)
    {
      combo->setCurrentIndex(index);
    }
  }
  if (useDefault)
  { //search for the default value
    auto index = combo->findText(defaultSelection);
    combo->setCurrentIndex((index<0 ? 0 : index));
  }

  //unblock
  combo->blockSignals(false);

  //notify if changed
  if (currentText != combo->currentText())
  { //changed
    emit combo->currentIndexChanged(combo->currentIndex());
  }

  return combo->count();
}

bool gui_util::validateFloat
(QLineEdit * line, float minVal, float maxVal, float defaultValue)
{
  bool ok = false;
  if (line)
  {
    float val = line->text().toFloat(&ok);
    if (!ok) { val = defaultValue; }
    if (val < minVal) { val = minVal; }
    if (val > maxVal) { val = maxVal; }
    line->setText(QString::number(val));
  }
  return ok;
}

void gui_util::wait(int msecs)
{
  QElapsedTimer timer;
  timer.start();
  while (timer.elapsed()<msecs)
  {
    QCoreApplication::processEvents();
  }
}

QStandardItem *gui_util::getCurrentItem
(QListView * listView, QModelIndex * selectedIndex) {
  if (!listView) { //null pointer
    return nullptr;
  }
  auto model = dynamic_cast<QStandardItemModel*>(listView->model());
  if (!model) { //unsupported model (or null)
    return nullptr;
  }

  //get selected item
  auto selectionModel = listView->selectionModel();
  if (!selectionModel) { //empty model
    return nullptr;
  }
  auto modelIndex = selectionModel->currentIndex();
  auto item = model->itemFromIndex(modelIndex);
  
  if (selectedIndex) {
    *selectedIndex = modelIndex;
  }

  return item;
}

QImage gui_util::loadCurrentImage
(QListView * listView, bool displayError, QString subdir)
{
  QModelIndex modelIndex;
  auto item = getCurrentItem(listView, &modelIndex);
  if (!item)
  { //no item selected
    if (displayError) {
      QMessageBox::critical(getApp()->getMainWindow(),
                            "Error", "No item is selected.");
    }
    return QImage();
  }

  QFileInfo info(item->data().toString());
  auto imageName = info.fileName();
  auto imagePath = info.absolutePath();

  QDir dir(imagePath);
  if (!subdir.isEmpty())
  {
    dir.cd(subdir);
  }
  QString filename = dir.absoluteFilePath(imageName);
  QImage qImg;
  if (!qImg.load(filename))
  {
    if (displayError) {
      QMessageBox::critical(getApp()->getMainWindow(),
                            "Error", QString("Image load failed: %1").arg(filename));
    }
    return QImage();
  }
  qImg.setText("filename", filename);
  qImg.setText("index", QString::number(modelIndex.row()));
  return qImg;
}

void gui_util::updateFilesItems(QStandardItemModel * model, QStringList fileList)
{
  if (!model)
  { //unsupported model (or null)
    return;
  }

  //update the model
  model->clear();
  auto parent = model->invisibleRootItem();
  foreach(auto filename, fileList)
  {
    QFileInfo info(filename);
    QStandardItem *item = new QStandardItem(info.fileName());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                   Qt::ItemNeverHasChildren);
    item->setData(info.absoluteFilePath());
    parent->appendRow(item);
  }
}
