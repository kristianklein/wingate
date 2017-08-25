#include "newathlete.h"
#include "ui_newathlete.h"

NewAthlete::NewAthlete(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewAthlete)
{
    ui->setupUi(this);
}

NewAthlete::~NewAthlete()
{
    delete ui;
}

Athlete NewAthlete::getAthleteObject()
{
    QString _firstname = ui->lineEdit_firstname->text();
    QString _lastname = ui->lineEdit_lastname->text();
    QString genderText = ui->comboBox_gender->itemText(ui->comboBox_gender->currentIndex());
    bool _gender;
    if (genderText == "Male")
    {
        _gender = 1;
    } else
    {
        _gender = 0;
    }

    int _age = ui->spinBox_age->value();
    double _height = ui->doubleSpinBox_height->value();
    double _weight = ui->doubleSpinBox_weight->value();
    double _resistance = ui->doubleSpinBox_resistance->value();

    Athlete athleteObject(_firstname, _lastname, _gender, _age, _height, _weight, _resistance);
    return athleteObject;
}

