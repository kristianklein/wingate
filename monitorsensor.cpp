#include "monitorsensor.h"
#include "ui_monitorsensor.h"
#include <QDebug>

monitorsensor::monitorsensor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::monitorsensor)
{
    ui->setupUi(this);
}

monitorsensor::monitorsensor(QWidget *parent, QSerialPort *serialPort) :
    QDialog(parent),
    ui(new Ui::monitorsensor)
{
    ui->setupUi(this);
    serial = serialPort;

    monitorTimer = new QTimer;
    connect(monitorTimer, SIGNAL(timeout()), this, SLOT(updateSensorLabel()));
    monitorTimer->setTimerType(Qt::PreciseTimer);
    monitorTimer->start(100);
}

monitorsensor::~monitorsensor()
{
    delete ui;

    monitorTimer->stop();
    delete monitorTimer;
}

int monitorsensor::sensorStatus()
{
    if (serial->isOpen())
    {
        QString incomingData;

        serial->flush();
        serial->write("2"); // request RPM data
        serial->waitForReadyRead(50);
        incomingData = serial->readAll();
        int incomingDataAsInt = incomingData.toInt();

        return incomingDataAsInt;
    }
    return -1;
}

void monitorsensor::updateSensorLabel(){
    if (serial->isOpen()){
        if (sensorStatus())
        {
            ui->label_sensorstatus->setStyleSheet("QLabel { color: green }");
        } else {
            ui->label_sensorstatus->setStyleSheet("QLabel { color: red }");
        }
    } else {
        ui->label_sensorstatus->setStyleSheet("QLabel { color: black }");
    }
}
