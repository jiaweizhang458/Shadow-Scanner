#ifndef SERIAL_PORT_COMMAND_LINE_DIALOG_H
#define SERIAL_PORT_COMMAND_LINE_DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>

#include "ui_SerialPortCommandLine.h"

class TurntableScanningPanel;

class SerialPortCommandLine : public QDialog, protected Ui_SerialPortCommandLine {
  Q_OBJECT

public:
  SerialPortCommandLine(TurntableScanningPanel* parent = 0);
  ~SerialPortCommandLine();

signals:

public slots:

private slots:
  void on_serialPortCommand_returnPressed();
  void on_sendCommandButton_clicked();
  void on_clearCommandButton_clicked();
  void on_closeButton_clicked();

private:

  TurntableScanningPanel* _turntableScanningPanel;

};

#endif // SERIAL_PORT_COMMAND_LINE_DIALOG_H
