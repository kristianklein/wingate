#include "wingate.h"
#include "ui_wingate.h"
#include <QMessageBox>
#include <QSerialPort>
#include <cmath>
#include <QFileInfo>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>

WinGate::WinGate(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WinGate)
{
    // Setup UI
    ui->setupUi(this);

    // Setup QCustomPlot axes
    ui->widget_plot->xAxis->setLabel("Time (s)");
    ui->widget_plot->yAxis->setLabel("Pedal frequency (rpm)");
    ui->widget_plot->xAxis->setRange(0, 30);
    ui->widget_plot->yAxis->setRange(0, 100);
    ui->widget_plot->replot();

    ui->widget_2->xAxis->setLabel("Time (s)");
    ui->widget_2->yAxis->setLabel("Pedal frequency (rpm)");
    ui->widget_2->xAxis->setRange(0, 30);
    ui->widget_2->yAxis->setRange(0, 100);
    ui->widget_2->replot();

    // Create serial objects
    serialInfo = new QSerialPortInfo;
    serial = new QSerialPort;

    // Start timer
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(monitorRPM()));
    timer->setTimerType(Qt::PreciseTimer);
    timer->start(1000);

    // Create athlete vector
    athletes = new QVector<Athlete>;
}

WinGate::~WinGate()
{
    if (serial->isOpen())
    {
        serial->close();
    }

    delete serialInfo;
    delete serial;
    delete athletes;
    delete timer;
    delete ui;
}

void WinGate::on_actionAbout_triggered()
{
    // TODO: Make an "about" page (perhaps with an image?)
    QMessageBox::about(this,"About", "This software was created by Kristian Klein Jacobsen at the Institute of Sports Science and Health, University of Southern Denmark.\n\n"
                                             "Source code can be found at http://github.com/kristianklein/wingate");
}

void WinGate::on_actionExit_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"Exit", "Are you sure you want to exit?", QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        QApplication::quit();
    }
}

void WinGate::on_pushButton_start_clicked()
{
    if (!serial->isOpen())
    {
        QMessageBox::critical(this, "Sensor not connected", "The sensor is not connected. Please connect the sensor via USB and establish the connection from the 'Sensor'-menu.");
    }
    else if (athletes->size() == 0)
    {
        QMessageBox::critical(this, "No athlete selected", "No athletes have been created. Please create and choose athlete before starting the test.");
    }
    else
    {
        testStarted = true;

        // Set test time and countdown time
        testSecsRemaining = ui->spinBox_testtime->value();
        countdownSecsRemaining = ui->spinBox_countdown->value();
        currentData.clear();
        timeData.clear();
        for (int i = 0; i == testSecsRemaining; i++)
        {
            timeData.push_back(i);
        }

        // Connect timer to test-function
        disconnect(timer, SIGNAL(timeout()), this, SLOT(monitorRPM()));
        connect(timer, SIGNAL(timeout()), this, SLOT(testTick()));

        // Restart timer
        timer->start(1000);

        // Set statusbar
        QString countdownMessage = "Test starting in " + QString::number(ui->spinBox_countdown->value()) + " seconds...";
        ui->statusBar->showMessage(countdownMessage);

        // Reset graph
        if (ui->widget_plot->graphCount() > 0)
        {
            ui->widget_plot->graph(0)->data()->clear();
        }
        ui->widget_plot->xAxis->setRange(0, 30);
        ui->widget_plot->yAxis->setRange(0, 100);
        ui->widget_plot->replot();

        // Disable controls
        disableControls();

        // Play sound
        countdownSound();
    }
}

bool WinGate::connectToSensor()
{
    quint16 arduinoIdentifier = 29987;
    QString sensorPort;

    QList<QSerialPortInfo> portsAvailable = serialInfo->availablePorts();

    qDebug() << "Number of serial connections detected: " << portsAvailable.size();

    for (int i = 0; i < portsAvailable.size(); i++)
    {
        if (portsAvailable.at(i).productIdentifier() == arduinoIdentifier)
        {
            sensorPort = portsAvailable.at(i).portName();
            qDebug() << "Sensor detected at " << sensorPort;

            // Reconnect if serial is already open
            if (serial->isOpen())
            {
                serial->close();
            }

            serial->setPortName(sensorPort);
            serial->setBaudRate(QSerialPort::Baud9600);
            serial->setDataBits(QSerialPort::Data8);
            serial->setParity(QSerialPort::NoParity);
            serial->setStopBits(QSerialPort::OneStop);
            serial->setFlowControl(QSerialPort::NoFlowControl);
            serial->open(QIODevice::ReadWrite);
            return true;
        }
    }
    qDebug() << "Sensor not found.";
    return false;
}

void WinGate::updateAthletes()
{
    qDebug() << "updateAthletes() was called!";

    // Clear all
    ui->comboBox_athlete->clear();
    ui->comboBox_data_athlete->clear();
    ui->comboBox_data_attempt->clear();
    ui->tableWidget->clear();


    // Return if no athletes
    if (athletes->size() == 0)
    {
        return;
    }

    // Add athletes by full name (both in "Test" and "Data" tab)
    for (int i = 0; i < athletes->size(); i++)
    {
        QString currentFirstName = athletes->at(i).getFirstName();
        QString currentLastName = athletes->at(i).getLastName();
        QString fullname = currentFirstName + " " + currentLastName;
        ui->comboBox_athlete->addItem(fullname);
        ui->comboBox_data_athlete->addItem(fullname);
    }
}

void WinGate::on_actionConnect_to_sensor_triggered()
{
    if (connectToSensor())
    {
        ui->statusBar->showMessage("Succesfully connected to sensor!");
    }
    else
    {
        ui->statusBar->showMessage("Sensor not found.");
    }
}

void WinGate::on_actionDisconnect_triggered()
{
    if (serial->isOpen())
    {
        serial->close();
    }
    ui->statusBar->showMessage("Serial connection closed.");
}

int WinGate::getRPMfromSerial()
{
    if (serial->isOpen())
    {
        QString incomingData;

        serial->flush();
        serial->write("1"); // request RPM data
        serial->waitForReadyRead(200);
        incomingData = serial->readAll();
        int incomingDataAsInt = incomingData.toInt();

        return incomingDataAsInt;
    }
    return 0;
}

void WinGate::monitorRPM()
{
    int revs = getRPMfromSerial();

    ui->lcdNumber->display(revs);
}

void WinGate::on_pushButton_newAthlete_clicked()
{
    newAthleteWindow = new NewAthlete;

    if (newAthleteWindow->exec() == QDialog::Accepted)
    {
        Athlete createdAthlete = newAthleteWindow->getAthleteObject();
        athletes->append(createdAthlete);

        updateAthletes();
    }
}

void WinGate::on_pushButton_deleteAthlete_clicked()
{
    if (athletes->size() > 0)
    {
        int athleteIndex = ui->comboBox_athlete->currentIndex();
        QString athleteDeleteString = "Are you sure you want to delete " + athletes->at(athleteIndex).getFirstName() + " " + athletes->at(athleteIndex).getLastName() + "?";

        QMessageBox::StandardButton reply = QMessageBox::question(this,"Delete athlete", athleteDeleteString, QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            athletes->remove(athleteIndex);
            updateAthletes();
        }
    }
}



void WinGate::on_pushButton_editAthlete_clicked()
{
    if (athletes->size() == 0)
    {
        return;
    }

    int athleteIndex = ui->comboBox_athlete->currentIndex();
    int numAttempts = athletes->at(athleteIndex).getNumAttempts();

    if (numAttempts > 0) {
        // Display warning
        // Current athletes has attempts which will be deleted if the athlete info is edited
        QString athleteName = athletes->at(athleteIndex).getFirstName() + " " + athletes->at(athleteIndex).getLastName();
        QString editMsg = athleteName + " currently has " + QString::number(numAttempts) + " registered attempt(s). Editing the athlete info will DELETE ALL ATTEMPTS.\nAre you sure you wish to edit this athlete?";

        // Dialog box to accept editing
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(this,"Edit athlete warning", editMsg, QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
        {
            return;
        }

    }


    // Edit athlete
    editAthleteWindow = new EditAthlete(this,
                                        athletes->at(athleteIndex).getFirstName(),
                                        athletes->at(athleteIndex).getLastName(),
                                        athletes->at(athleteIndex).getGender(),
                                        athletes->at(athleteIndex).getAge(),
                                        athletes->at(athleteIndex).getHeight(),
                                        athletes->at(athleteIndex).getWeight(),
                                        athletes->at(athleteIndex).getResistance());

    if (editAthleteWindow->exec() == QDialog::Accepted)
    {
        // Edit athlete information
        Athlete editedAthlete = editAthleteWindow->getAthleteObject();
        athletes->insert(athleteIndex, editedAthlete);
        athletes->remove(athleteIndex+1);

        updateAthletes();
    }
}

void WinGate::on_comboBox_athlete_currentIndexChanged(int index)
{
    if (index == -1)
    {
        return;
    }

    ui->doubleSpinBox_resistance->setValue(athletes->at(index).getResistance());
}

void WinGate::testTick()
{ 
    if (countdownSecsRemaining > 1) // Go to countdown on the "0"-tick
    {
        countdownSecsRemaining--;
        QString countdownMessage = "Test starting in " + QString::number(countdownSecsRemaining) + " seconds...";
        ui->statusBar->showMessage(countdownMessage);

        // Play sound effect
        sound.setSource(QUrl::fromLocalFile("beep.wav"));
        sound.setLoopCount(1);
        sound.setVolume(0.25f);
        sound.play();

    }
    else if (testSecsRemaining >= 0)
    {
        // Play sound if first tick
        if (timeData.size() == 0)
        {
            startSound();
        }


        // Update time vector
        timeData.push_back(currentData.size());

        // Get RPM
        int revs = getRPMfromSerial();
        currentData.push_back(revs);

        // Find max value in vector
        int maxValue = 0;
        for (int i = 0; i < currentData.size(); i++)
        {
            if (currentData.at(i) > maxValue)
            {
                maxValue = currentData.at(i);
            }
        }

        // Replot data
        if (ui->widget_plot->graphCount() < 1)
        {
            ui->widget_plot->addGraph();
        }
        ui->widget_plot->graph(0)->setData(timeData, currentData);

        if (maxValue > 20)
        {
            ui->widget_plot->yAxis->setRange(0, maxValue);
        }
        else
        {
            ui->widget_plot->yAxis->setRange(0, 20);
        }


        ui->widget_plot->replot();

        // Set progress bar
        ui->progressBar->setValue(testSecsRemaining);
        testSecsRemaining--;

        // Set statusbar and lcd
        ui->statusBar->showMessage("Test running...");
        ui->lcdNumber->display(revs);

    }
    else
    {
        // Stop test and reconnect the timer
        disconnect(timer, SIGNAL(timeout()), this, SLOT(testTick()));
        connect(timer, SIGNAL(timeout()), this, SLOT(monitorRPM()));

        // Push data to athlete object
        int athleteIndex = ui->comboBox_athlete->currentIndex();
        Athlete thisAthlete = athletes->at(athleteIndex);
        thisAthlete.pushData(currentData);
        athletes->insert(athleteIndex, thisAthlete);
        athletes->remove(athleteIndex+1);

        // Print all data to console
        for (int i = 0; i < currentData.size(); i++)
        {
            qDebug() << i << "," << currentData.at(i);
        }

        // Set statusbar
        ui->statusBar->showMessage("Test finished!");

        // Enable controls
        enableControls();

        // Update attempt list
        this->on_comboBox_data_athlete_currentIndexChanged(ui->comboBox_data_athlete->currentIndex());
    }
}

void WinGate::on_pushButton_4_clicked()
{
    if (testStarted)
    {
        // Stop test and reconnect the timer
        disconnect(timer, SIGNAL(timeout()), this, SLOT(testTick()));
        connect(timer, SIGNAL(timeout()), this, SLOT(monitorRPM()));

        ui->statusBar->showMessage("Test stopped manually!");
        testStarted = false;

        // Re-enable controls
        enableControls();
    }
    else
    {
        ui->statusBar->showMessage("Test is not running.");
    }
}

void WinGate::disableControls()
{
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_newAthlete->setEnabled(false);
    ui->pushButton_editAthlete->setEnabled(false);
    ui->pushButton_deleteAthlete->setEnabled(false);
    ui->comboBox_athlete->setEnabled(false);
    ui->spinBox_testtime->setEnabled(false);
    ui->spinBox_countdown->setEnabled(false);
    ui->doubleSpinBox_resistance->setEnabled(false);
    ui->tabWidget->setTabEnabled(1, false);
    ui->menuBar->setEnabled(false);
}

void WinGate::enableControls()
{
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_newAthlete->setEnabled(true);
    ui->pushButton_editAthlete->setEnabled(true);
    ui->pushButton_deleteAthlete->setEnabled(true);
    ui->comboBox_athlete->setEnabled(true);
    ui->spinBox_testtime->setEnabled(true);
    ui->spinBox_countdown->setEnabled(true);
    ui->doubleSpinBox_resistance->setEnabled(true);
    ui->tabWidget->setTabEnabled(1, true);
    ui->menuBar->setEnabled(true);
}

void WinGate::countdownSound()
{
    QFileInfo checkfile("beep.wav");
    if(checkfile.exists())
    {
        sound.setSource(QUrl::fromLocalFile("beep.wav"));
        sound.setLoopCount(1);
        sound.setVolume(0.25f);
        sound.play();
    }
}

void WinGate::startSound()
{
    QFileInfo checkfile("beep_start.wav");
    if(checkfile.exists())
    {
        sound.setSource(QUrl::fromLocalFile("beep_start.wav"));
        sound.setLoopCount(1);
        sound.setVolume(0.25f);
        sound.play();
    }
}

void WinGate::on_actionNew_session_triggered()
{
    qDebug() << athletes->size();

    // Clear athletes
    int numAthletes = athletes->size();
    for (int i = 0; i < numAthletes; i++)
    {
        athletes->pop_back();
    }

    qDebug() << athletes->size();

    updateAthletes();

    // Reset test settings
    ui->spinBox_testtime->setValue(30);
    ui->spinBox_countdown->setValue(3);
    ui->doubleSpinBox_resistance->setValue(0);

    // Reset countdown timer
    ui->progressBar->setValue(ui->spinBox_testtime->value());

    // Reset graph
    if (ui->widget_plot->graphCount() > 0)
    {
        ui->widget_plot->graph(0)->data()->clear();
    }
    ui->widget_plot->xAxis->setRange(0, 30);
    ui->widget_plot->yAxis->setRange(0, 100);
    ui->widget_plot->replot();
}

void WinGate::on_actionSave_session_triggered()
{
    // Get save path from user
    QString filename = QFileDialog::getSaveFileName(this, tr("Save session"), QDir::currentPath(), tr("*.dat files (*.dat)"));

    if (filename.isNull())
    {
        return;
    }


    // Add .dat extension if not already present
    if (filename.length() > 4)
    {
        QString extension = filename.right(4);
        if (extension != ".dat")
        {
            filename += ".dat";
        }
    }
    else
    {
        filename += ".dat";
    }

    qDebug() << filename;


    QFile saveFile(filename);
    if (saveFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream(&saveFile);

        stream << ui->spinBox_testtime->value() << ",";
        stream << ui->spinBox_countdown->value() << endl;

        for (int i = 0; i < athletes->size(); i++)
        {
            stream << athletes->at(i).getFirstName() << ",";
            stream << athletes->at(i).getLastName() << ",";
            stream << athletes->at(i).getGender() << ",";
            stream << athletes->at(i).getAge() << ",";
            stream << athletes->at(i).getHeight() << ",";
            stream << athletes->at(i).getWeight() << ",";
            stream << athletes->at(i).getResistance() << ",";
            stream << athletes->at(i).getNumAttempts();
            stream << endl;

            for (int j = 0; j < athletes->at(i).getNumAttempts(); j++)
            {
                QVector<double> attemptData = athletes->at(i).getData(j);

                for (int k = 0; k < attemptData.size(); k ++)
                {
                    stream << attemptData.at(k);

                    if (k != attemptData.size()-1)
                    {
                        stream << ", ";
                    }
                }
                stream << endl;
            }

            stream << endl;
        }

    }
    saveFile.close();


}

void WinGate::on_actionLoad_sesseion_triggered()
{
    // Clear athletes
    int numAthletes = athletes->size();
    for (int i = 0; i < numAthletes; i++)
    {
        athletes->pop_back();
    }

    // Reset graph
    if (ui->widget_plot->graphCount() > 0)
    {
        ui->widget_plot->graph(0)->data()->clear();
    }
    ui->widget_plot->xAxis->setRange(0, 30);
    ui->widget_plot->yAxis->setRange(0, 100);
    ui->widget_plot->replot();


    // Load data file
    QString filename = QFileDialog::getOpenFileName(this, tr("Load session"), QDir::currentPath(), tr("Data files (*.dat)"));

    if (filename.isNull())
    {
        return;
    }

    QFile openFile(filename);
    if (openFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&openFile);
        QString line;
        int numLines = 0;

        do
        {
            line = stream.readLine();
            numLines++;

            if (line.isEmpty()) // Skip blank lines
            {
                continue;
            }

            if (numLines == 1) // First line contains test settings
            {
                QStringList lineList = line.split(",");
                int testTime = lineList.value(0).toInt();
                int countdownTime = lineList.value(1).toInt();
                ui->spinBox_testtime->setValue(testTime);
                ui->spinBox_countdown->setValue(countdownTime);
            }
            else
            {
                QStringList lineList = line.split(",");
                QString firstName = lineList.at(0);
                QString lastName = lineList.at(1);
                int gender = lineList.at(2).toInt();
                int age = lineList.at(3).toInt();
                double height = lineList.at(4).toDouble();
                double weight = lineList.at(5).toDouble();
                double resistance = lineList.at(6).toDouble();
                int numAttempts = lineList.at(7).toInt();

                Athlete currentAthlete(firstName, lastName, gender, age, height, weight, resistance);

                for (int i = 0; i < numAttempts; i++)
                {
                    QVector<double> currentAttempt;
                    line = stream.readLine();
                    QStringList dataList = line.split(",");

                    for (int j = 0; j < dataList.length(); j++)
                    {
                        currentAttempt.push_back(dataList.at(j).toDouble());
                    }
                    currentAthlete.pushData(currentAttempt);
                }

                athletes->push_back(currentAthlete);

            }
        }
        while (!line.isNull());
    }



    // Print for debugging
    qDebug() << "Loaded " << athletes->size() << " athletes.";
    for (int i = 0; i < athletes->size(); i++)
    {
        qDebug() << athletes->at(i).getFirstName() << athletes->at(i).getLastName() << " has " << athletes->at(i).getNumAttempts() << " attempts.";
    }

    updateAthletes();

}

void WinGate::on_comboBox_data_attempt_currentIndexChanged(int index)
{
    // If combobox empty
    if (index == -1)
    {
        // Clear plot
        if (ui->widget_2->graphCount() > 0)
        {
            ui->widget_2->graph(0)->data()->clear();
        }

        // Set y-axis label
        if (ui->radioButton_rpm->isChecked()) {
            ui->widget_2->yAxis->setLabel("Pedal frequency (rpm)");
        } else {
            ui->widget_2->yAxis->setLabel("Power (W)");
        }

        // Set graph range
        ui->widget_2->xAxis->setRange(0, 30);
        ui->widget_2->yAxis->setRange(0, 100);
        ui->widget_2->replot();

        // Clear table and set correct header
        ui->tableWidget->clear();

        ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Time (s)"));

        if (ui->radioButton_rpm->isChecked()) {
            ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Pedal freq. (RPM)"));
        } else {
            ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Power (W)"));
        }


        return;
    }

    // Update graph
    if (ui->widget_2->graphCount() < 1)
    {
        ui->widget_2->addGraph();
    }
    else
    {
        ui->widget_2->graph(0)->data()->clear();
    }

    // Set y-axis label
    if (ui->radioButton_rpm->isChecked()) {
        ui->widget_2->yAxis->setLabel("Pedal frequency (rpm)");
    } else {
        ui->widget_2->yAxis->setLabel("Power (W)");
    }

    // Get data from athlete object
    int athleteIndex = ui->comboBox_data_athlete->currentIndex();
    double athleteResistance = athletes->at(athleteIndex).getResistance();

    QVector<double> revData = athletes->at(athleteIndex).getData(index);
    QVector<double> timeData;
    QVector<double> powerData;

    // Generate time data
    for (int i = 0; i < revData.length(); i++) {
        timeData.push_back(i);
    }

    // Generate power data
    for (int i = 0; i < revData.length(); i++) {
        powerData.push_back(calcPower(revData.at(i),athleteResistance));
    }

    // Find max value and set data
    int maxValue = rpmMax(revData);
    if (ui->radioButton_rpm->isChecked()) {
        ui->widget_2->graph(0)->setData(timeData, revData);
    } else {
        maxValue = calcPower(maxValue, athleteResistance);
        ui->widget_2->graph(0)->setData(timeData, powerData);
    }




    if (maxValue > 20)
    {
        ui->widget_2->yAxis->setRange(0, maxValue);
    }
    else
    {
        ui->widget_2->yAxis->setRange(0, 20);
    }
    ui->widget_2->xAxis->setRange(0, timeData.size()-1);

    ui->widget_2->replot();

    // Update tableWidget data
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Time (s)"));

    if (ui->radioButton_rpm->isChecked()) {
        ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Pedal freq. (RPM)"));
    } else {
        ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Power (W)"));
    }


    ui->tableWidget->setRowCount(timeData.size());
    ui->tableWidget->setColumnCount(2);

    for (int i = 0; i < timeData.size(); i++)
    {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));

        if (ui->radioButton_rpm->isChecked()) {
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(revData.at(i))));
        } else {
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(powerData.at(i))));
        }
    }

    // Update summary data (peak, average, fatigue)
    ui->lineEdit_peakpower->setText(QString::number(peakPower(revData, athletes->at(athleteIndex).getResistance())));
    ui->lineEdit_avgpower->setText(QString::number(averagePower(revData, athletes->at(athleteIndex).getResistance())));
    ui->lineEdit_fatigue->setText(QString::number(fatigueIndex(revData)));
}

void WinGate::on_comboBox_data_athlete_currentIndexChanged(int index)
{

    // If no athletes
    if (index == -1)
    {
        // Clear plot and return
        if (ui->widget_2->graphCount() > 0)
        {
            ui->widget_2->graph(0)->data()->clear();
        }
        ui->widget_2->xAxis->setRange(0, 30);
        ui->widget_2->yAxis->setRange(0, 100);
        ui->widget_2->replot();
        return;
    }

    // Update attempt list
    int numAttempts = athletes->at(index).getNumAttempts();
    if (numAttempts > 0)
    {
        ui->comboBox_data_attempt->clear();

        for (int i = 0; i < numAttempts; i++)
        {
            ui->comboBox_data_attempt->addItem(QString::number(i+1));
        }

        ui->comboBox_data_attempt->setCurrentIndex(0);
    }
    else
    {
        ui->comboBox_data_attempt->clear();
    }
}

void WinGate::on_pushButton_data_deleteselected_clicked()
{
    // Return if no attempts
    if (ui->comboBox_data_attempt->currentIndex() == -1)
    {
        return;
    }

    int athleteIndex = ui->comboBox_data_athlete->currentIndex();
    int attempt = ui->comboBox_data_attempt->currentIndex();
    QString athleteName = athletes->at(athleteIndex).getFirstName() + " " + athletes->at(athleteIndex).getLastName();

    QString deleteMsg = "Are you sure you want to delete attempt " + QString::number(attempt+1) + " for athlete " + athleteName + "?";

    // Dialog box to accept deletion
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"Delete all attempts", deleteMsg, QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        // Delete current index
        int currentAthleteIndex = ui->comboBox_data_athlete->currentIndex();
        int currentIndex = ui->comboBox_data_attempt->currentIndex();

        Athlete currentAthlete = athletes->at(currentAthleteIndex);
        currentAthlete.removeData(currentIndex);

        athletes->insert(currentAthleteIndex, currentAthlete);
        athletes->remove(currentAthleteIndex+1);

        // Update attempt list
        this->on_comboBox_data_athlete_currentIndexChanged(ui->comboBox_data_athlete->currentIndex());
    }


}

void WinGate::on_pushButton_data_deleteall_clicked()
{
    // Return if no attempts
    if (ui->comboBox_data_attempt->currentIndex() == -1)
    {
        return;
    }

    int athleteIndex = ui->comboBox_data_athlete->currentIndex();
    QString athleteName = athletes->at(athleteIndex).getFirstName() + " " + athletes->at(athleteIndex).getLastName();

    QString deleteMsg = "Are you sure you want to delete all attempts for athlete " + athleteName + "?";

    // Dialog box to accept deletion
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"Delete all attempts", deleteMsg, QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        // Delete all attempts for current athlete
        int currentAthleteIndex = ui->comboBox_data_athlete->currentIndex();

        Athlete currentAthlete = athletes->at(currentAthleteIndex);
        int numAttempts = currentAthlete.getNumAttempts();

        for (int i = 0; i < numAttempts; i++)
        {
            currentAthlete.removeData(0);
        }

        athletes->insert(currentAthleteIndex, currentAthlete);
        athletes->remove(currentAthleteIndex+1);

        // Update attempt list
        this->on_comboBox_data_athlete_currentIndexChanged(ui->comboBox_data_athlete->currentIndex());
    }
}

void WinGate::on_actionExport_to_CSV_triggered()
{
    // Get save path from user
    QString filename = QFileDialog::getSaveFileName(this, tr("Export to CSV"), QDir::currentPath(), tr("*.csv files (*.csv)"));

    if (filename.isNull())
    {
        return;
    }


    // Add .dat extension if not already present
    if (filename.length() > 4)
    {
        QString extension = filename.right(4);
        if (extension != ".csv")
        {
            filename += ".csv";
        }
    }
    else
    {
        filename += ".csv";
    }

    qDebug() << filename;


    QFile exportFile(filename);
    if (exportFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream(&exportFile);


        for (int i = 0; i < athletes->size(); i++) // Iterate over athletes
        {
            Athlete currentAthlete = athletes->at(i);

            QString gender = "Female";
            if (currentAthlete.getGender())
                gender = "Male";

            stream << "First name;" << currentAthlete.getFirstName() << endl;
            stream << "Last name;" << currentAthlete.getLastName() << endl;
            stream << "Gender;" << gender << endl;
            stream << "Age;" << currentAthlete.getAge() << endl;
            stream << "Height (cm);" << currentAthlete.getHeight() << endl;
            stream << "Weight (kg);" << currentAthlete.getWeight() << endl;
            stream << "Resistance (kg);" << currentAthlete.getResistance() << endl;
            stream << "Attempts;" << currentAthlete.getNumAttempts() << endl;
            stream << endl;

            for (int j = 0; j < currentAthlete.getNumAttempts(); j++) // Iterate over attempts
            {
                QVector<double> currentAttempt = currentAthlete.getData(j);

                stream << "Attempt " << j+1 << endl;
                stream << "Peak power (W);" << peakPower(currentAthlete.getData(j), currentAthlete.getResistance()) << endl;
                stream << "Average power (W);" << averagePower(currentAthlete.getData(j),currentAthlete.getResistance()) << endl;
                stream << "Fatigue index (%);" << fatigueIndex(currentAthlete.getData(j)) << endl;
                stream << "Time (s);Pedal frequency (rev/min);Power (W);" << endl;

                for (int k = 0; k < currentAttempt.size(); k++) // Iterate over data points in attempt
                {
                    stream << k << ";" << currentAttempt.at(k) << ";" << calcPower(currentAttempt.at(k), currentAthlete.getResistance()) << endl;
                }

                stream << endl;
            }
        }
    }
    exportFile.close();


}

int WinGate::calcPower(double revs, double resistance)
{
    return (revs*6/60)*(resistance*9.81);
}

int WinGate::rpmMax(QVector<double> vec)
{
    int max = vec.at(0); // Set initial max

    for (int i = 0; i < vec.size(); i++)
    {
        if (vec.at(i) > max)
        {
            max = vec.at(i);
        }
    }
    return max;
}

int WinGate::rpmMinEnd(QVector<double> vec)
{
    int minEnd = vec.at(vec.size()-1); // Set last element to inital min

    for (int i = vec.size()-6; i < vec.size(); i++) // Search last 5 elements in vector for min
    {
        if (vec.at(i) < minEnd)
        {
            minEnd = vec.at(i);
        }
    }
    return minEnd;
}

int WinGate::peakPower(QVector<double> vec, double resistance){
    int rpm = rpmMax(vec);
    return calcPower(rpm, resistance);
}

int WinGate::averagePower(QVector<double> vec, double resistance)
{
    int averagePower, sumOfPower = 0;

    for (int i = 0; i < vec.size(); i++)
    {
        sumOfPower += calcPower(vec.at(i), resistance);
    }

    averagePower = sumOfPower/vec.size();
    return averagePower;

}

double WinGate::fatigueIndex(QVector<double> vec)
{
    double maxRPM = rpmMax(vec);
    double minRPM = rpmMinEnd(vec);
    double fatigue = (maxRPM-minRPM)/maxRPM;

    return fatigue*100;

}

void WinGate::on_actionMonitor_sensor_triggered()
{
    monitorSensor = new monitorsensor(this, serial);
    monitorSensor->setModal(true);

    // Temporarily stop timer
    timer->stop();

    if (monitorSensor->exec() == QDialog::Rejected) // Window closed
    {
        // Restart timer
        timer->start(1000);
    }
}

void WinGate::on_actionExport_to_CSV_RPM_only_triggered()
{
    // Get save path from user
    QString filename = QFileDialog::getSaveFileName(this, tr("Export to CSV"), QDir::currentPath(), tr("*.csv files (*.csv)"));

    if (filename.isNull())
    {
        return;
    }


    // Add .dat extension if not already present
    if (filename.length() > 4)
    {
        QString extension = filename.right(4);
        if (extension != ".csv")
        {
            filename += ".csv";
        }
    }
    else
    {
        filename += ".csv";
    }

    qDebug() << filename;


    QFile exportFile(filename);
    if (exportFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream(&exportFile);


        for (int i = 0; i < athletes->size(); i++) // Iterate over athletes
        {
            Athlete currentAthlete = athletes->at(i);

            QString gender = "Female";
            if (currentAthlete.getGender())
                gender = "Male";

            stream << "First name;" << currentAthlete.getFirstName() << endl;
            stream << "Last name;" << currentAthlete.getLastName() << endl;
            stream << "Gender;" << gender << endl;
            stream << "Age;" << currentAthlete.getAge() << endl;
            stream << "Height (cm);" << currentAthlete.getHeight() << endl;
            stream << "Weight (kg);" << currentAthlete.getWeight() << endl;
            stream << "Resistance (kg);" << currentAthlete.getResistance() << endl;
            stream << "Attempts;" << currentAthlete.getNumAttempts() << endl;
            stream << endl;

            for (int j = 0; j < currentAthlete.getNumAttempts(); j++) // Iterate over attempts
            {
                QVector<double> currentAttempt = currentAthlete.getData(j);

                stream << "Attempt " << j+1 << endl;
                stream << "Time (s);Pedal frequency (rev/min);" << endl;

                for (int k = 0; k < currentAttempt.size(); k++) // Iterate over data points in attempt
                {
                    stream << k << ";" << currentAttempt.at(k) << ";" << endl;
                }

                stream << endl;
            }
        }
    }
    exportFile.close();


}

void WinGate::on_radioButton_power_clicked()
{
    // Update data view
    on_comboBox_data_attempt_currentIndexChanged(ui->comboBox_data_attempt->currentIndex());
}

void WinGate::on_radioButton_rpm_clicked()
{
    // Update data view
    on_comboBox_data_attempt_currentIndexChanged(ui->comboBox_data_attempt->currentIndex());
}
