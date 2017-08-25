#ifndef EDITATHLETE_H
#define EDITATHLETE_H

#include <QDialog>
#include "athlete.h"

namespace Ui {
class EditAthlete;
}

class EditAthlete : public QDialog
{
    Q_OBJECT

public:
    explicit EditAthlete(QWidget *parent = 0);
    explicit EditAthlete(QWidget *parent, QString firstname, QString lastname, bool gender, int age, double height, double weight, double resistance);
    ~EditAthlete();
    Athlete getAthleteObject();

private:
    Ui::EditAthlete *ui;
    int index;
};

#endif // EDITATHLETE_H
