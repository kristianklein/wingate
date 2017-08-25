#ifndef NEWATHLETE_H
#define NEWATHLETE_H

#include <QDialog>
#include <QDebug>
#include "athlete.h"

namespace Ui {
class NewAthlete;
}

class NewAthlete : public QDialog
{
    Q_OBJECT

public:
    explicit NewAthlete(QWidget *parent = 0);
    ~NewAthlete();
    Athlete getAthleteObject();

private:
    Ui::NewAthlete *ui;
};

#endif // NEWATHLETE_H
