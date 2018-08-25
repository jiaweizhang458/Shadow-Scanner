/****************************************************************************
 **
 ** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
 ** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
 ** Contact: http://www.qt.io/licensing/
 **
 ** This file is part of the QtSerialPort module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL21$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see http://www.qt.io/terms-conditions. For further
 ** information use the contact form at http://www.qt.io/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 or version 3 as published by the Free
 ** Software Foundation and appearing in the file LICENSE.LGPLv21 and
 ** LICENSE.LGPLv3 included in the packaging of this file. Please review the
 ** following information to ensure the GNU Lesser General Public License
 ** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
 ** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** As a special exception, The Qt Company gives you certain additional
 ** rights. These rights are described in The Qt Company LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "SerialPortSettings.hpp"
#include "ui_SerialPortSettings.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SerialPortSettings", "N/A");

SerialPortSettings::SerialPortSettings(QWidget *parent) :
QDialog(parent) {
  setupUi(this);

  intValidator = new QIntValidator(0, 4000000, this);

  baudRateBox->setInsertPolicy(QComboBox::NoInsert);

  connect(applyButton, SIGNAL(clicked()),
          this, SLOT(apply()));
  connect(serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(showPortInfo(int)));
  connect(baudRateBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(checkCustomBaudRatePolicy(int)));
  connect(serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(checkCustomDevicePathPolicy(int)));

  fillPortsParameters();
  fillPortsInfo();

  updateSettings();
}

SerialPortSettings::~SerialPortSettings() {
}

SerialPortSettings::Settings SerialPortSettings::settings() const {
  return currentSettings;
}

void SerialPortSettings::showPortInfo(int idx) {
  if (idx == -1) return;

  QStringList list = serialPortInfoListBox->itemData(idx).toStringList();
  descriptionLabel->setText
    (tr("Description: %1").arg(list.count() > 1?list.at(1):tr(blankString)));
  manufacturerLabel->setText
    (tr("Manufacturer: %1").arg(list.count() > 2?list.at(2):tr(blankString)));
  serialNumberLabel->setText
    (tr("Serial number: %1").arg(list.count() > 3?list.at(3):tr(blankString)));
  locationLabel->setText
    (tr("Location: %1").arg(list.count() > 4?list.at(4):tr(blankString)));
  vidLabel->setText
    (tr("Vendor Identifier: %1").arg(list.count() > 5?list.at(5):tr(blankString)));
  pidLabel->setText
    (tr("Product Identifier: %1").arg(list.count() > 6?list.at(6):tr(blankString)));
}

void SerialPortSettings::apply() {
  updateSettings();
  hide();
  emit applyPressed();
}

void SerialPortSettings::checkCustomBaudRatePolicy(int idx) {
  bool isCustomBaudRate = !baudRateBox->itemData(idx).isValid();
  baudRateBox->setEditable(isCustomBaudRate);
  if (isCustomBaudRate) {
    baudRateBox->clearEditText();
    QLineEdit *edit = baudRateBox->lineEdit();
    edit->setValidator(intValidator);
  }
}

void SerialPortSettings::checkCustomDevicePathPolicy(int idx) {
  bool isCustomPath = !serialPortInfoListBox->itemData(idx).isValid();
  serialPortInfoListBox->setEditable(isCustomPath);
  if (isCustomPath)
    serialPortInfoListBox->clearEditText();
}

void SerialPortSettings::fillPortsParameters() {
  baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
  baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
  baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
  baudRateBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
  baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
  baudRateBox->addItem(tr("Custom"));

  dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
  dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
  dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
  dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
  dataBitsBox->setCurrentIndex(3);

  parityBox->addItem(tr("None"), QSerialPort::NoParity);
  parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
  parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
  parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
  parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

  stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
  stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
  stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

  flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
  flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
  flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

void SerialPortSettings::fillPortsInfo() {
  serialPortInfoListBox->clear();
  QString description;
  QString manufacturer;
  QString serialNumber;
  foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
    QStringList list;
    description = info.description();
    manufacturer = info.manufacturer();
    serialNumber = info.serialNumber();
    list << info.portName()
         << (!description.isEmpty()?description:blankString)
         << (!manufacturer.isEmpty()?manufacturer:blankString)
         << (!serialNumber.isEmpty()?serialNumber:blankString)
         << info.systemLocation()
         << (info.vendorIdentifier()?
             QString::number(info.vendorIdentifier(), 16):blankString)
         << (info.productIdentifier()?
             QString::number(info.productIdentifier(), 16):blankString);
    serialPortInfoListBox->addItem(list.first(), list);
  }

  serialPortInfoListBox->addItem(tr("Custom"));
}

void SerialPortSettings::updateSettings() {
  currentSettings.name = serialPortInfoListBox->currentText();

  if (baudRateBox->currentIndex() == 4) {
    currentSettings.baudRate = baudRateBox->currentText().toInt();
  } else {
    currentSettings.baudRate = static_cast<QSerialPort::BaudRate>
      (baudRateBox->itemData(baudRateBox->currentIndex()).toInt());
  }
  currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

  currentSettings.dataBits = static_cast<QSerialPort::DataBits>
    (dataBitsBox->itemData(dataBitsBox->currentIndex()).toInt());
  currentSettings.stringDataBits = dataBitsBox->currentText();

  currentSettings.parity = static_cast<QSerialPort::Parity>
    (parityBox->itemData(parityBox->currentIndex()).toInt());
  currentSettings.stringParity = parityBox->currentText();

  currentSettings.stopBits = static_cast<QSerialPort::StopBits>
    (stopBitsBox->itemData(stopBitsBox->currentIndex()).toInt());
  currentSettings.stringStopBits = stopBitsBox->currentText();

  currentSettings.flowControl = static_cast<QSerialPort::FlowControl>
    (flowControlBox->itemData(flowControlBox->currentIndex()).toInt());
  currentSettings.stringFlowControl = flowControlBox->currentText();

  // currentSettings.localEchoEnabled = localEchoCheckBox->isChecked();
}

void SerialPortSettings::selectName(const QString& name) {
  int indx = serialPortInfoListBox->findText(name,Qt::MatchExactly);
  if(indx>=0) serialPortInfoListBox->setCurrentIndex(indx);
}

void SerialPortSettings::selectBaudRate(const QString& baudRate) {
  int indx = baudRateBox->findText(baudRate,Qt::MatchExactly);
  if(indx>=0) baudRateBox->setCurrentIndex(indx);
}

void SerialPortSettings::selectDataBits(const QString& dataBits) {
  int indx = dataBitsBox->findText(dataBits,Qt::MatchExactly);
  if(indx>=0) dataBitsBox->setCurrentIndex(indx);
}

void SerialPortSettings::selectParity(const QString& parity) {
  int indx = parityBox->findText(parity,Qt::MatchExactly);
  if(indx>=0) parityBox->setCurrentIndex(indx);
}

void SerialPortSettings::selectStopBits(const QString& stopBits) {
  int indx = stopBitsBox->findText(stopBits,Qt::MatchExactly);
  if(indx>=0) stopBitsBox->setCurrentIndex(indx);
}

void SerialPortSettings::selectFlowControl(const QString& flowControl) {
  int indx = flowControlBox->findText(flowControl,Qt::MatchExactly);
  if(indx>=0) flowControlBox->setCurrentIndex(indx);
}
