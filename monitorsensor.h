#ifndef MONITORSENSOR_H
#define MONITORSENSOR_H

#include <QDialog>
#include <QTimer>
#include <QSerialPort>

namespace Ui {
class monitorsensor;
}

class monitorsensor : public QDialog
{
    Q_OBJECT

public:
    explicit monitorsensor(QWidget *parent = 0);
    explicit monitorsensor(QWidget *parent, QSerialPort *serialPort);
    ~monitorsensor();

private:
    Ui::monitorsensor *ui;
    QSerialPort *serial;
    QTimer *monitorTimer;
    int sensorStatus();

private slots:
    void updateSensorLabel();
};

#endif // MONITORSENSOR_H
