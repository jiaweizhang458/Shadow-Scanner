#include "TurntableScanningPanel.hpp"
#include "SerialPortCommandLine.hpp"

QT_USE_NAMESPACE

SerialPortCommandLine::SerialPortCommandLine(TurntableScanningPanel* parent) :
  QDialog((QWidget*)0),
  _turntableScanningPanel(parent) {
  setupUi(this);
}

SerialPortCommandLine::~SerialPortCommandLine() {
}

void SerialPortCommandLine::on_serialPortCommand_returnPressed() {
  on_sendCommandButton_clicked();
}

void SerialPortCommandLine::on_sendCommandButton_clicked() {
  QString command = serialPortCommand->text()+" ";
  QByteArray response = _turntableScanningPanel->sendTurntableCommand(command,30000);
  serialPortResponse->setText(QString(response));
}

void SerialPortCommandLine::on_clearCommandButton_clicked() {
  serialPortCommand->clear();
  serialPortResponse->clear();
}

void SerialPortCommandLine::on_closeButton_clicked() {
  hide();
}

