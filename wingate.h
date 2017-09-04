#ifndef WINGATE_H
#define WINGATE_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include "newathlete.h"
#include "editathlete.h"
#include "athlete.h"
#include "monitorsensor.h"
#include <QSoundEffect>

namespace Ui {
class WinGate;
}

class WinGate : public QMainWindow
{
    Q_OBJECT

public:
    explicit WinGate(QWidget *parent = 0);
    ~WinGate();

private slots:
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void on_pushButton_start_clicked();
    void on_actionConnect_to_sensor_triggered();
    void on_actionDisconnect_triggered();
    void on_pushButton_newAthlete_clicked();
    void on_pushButton_deleteAthlete_clicked();
    void on_pushButton_editAthlete_clicked();
    void on_comboBox_athlete_currentIndexChanged(int index);
    void on_pushButton_4_clicked();
    void on_actionNew_session_triggered();
    void on_actionSave_session_triggered();
    void on_actionLoad_sesseion_triggered();
    void on_comboBox_data_attempt_currentIndexChanged(int index);
    void on_comboBox_data_athlete_currentIndexChanged(int index);
    void on_pushButton_data_deleteselected_clicked();
    void on_pushButton_data_deleteall_clicked();
    void on_actionExport_to_CSV_triggered();
    void on_actionMonitor_sensor_triggered();

    void on_actionExport_to_CSV_RPM_only_triggered();

    void on_radioButton_power_clicked();

    void on_radioButton_rpm_clicked();

public slots:
    int getRPMfromSerial();
    void monitorRPM();
    void testTick();

private:
    Ui::WinGate *ui;
    QSerialPortInfo *serialInfo;
    QSerialPort *serial;
    QTimer *timer;
    bool connectToSensor();
    NewAthlete *newAthleteWindow;
    EditAthlete *editAthleteWindow;
    monitorsensor *monitorSensor;
    QVector<Athlete> *athletes;
    void updateAthletes();
    QVector<double> currentData;
    QVector<double> timeData;
    int testSecsRemaining;
    int countdownSecsRemaining;
    bool testStarted = false;
    void disableControls();
    void enableControls();
    QSoundEffect sound;
    void countdownSound();
    void startSound();
    int calcPower(double revs, double resistance);
    int rpmMax(QVector<double> vec);
    int rpmMinEnd(QVector<double> vec);
    int averagePower(QVector<double> vec, double resistance);
    double fatigueIndex(QVector<double> vec);
    int peakPower(QVector<double> vec, double resistance);
};

#endif // WINGATE_H
