#include "editathlete.h"
#include "ui_editathlete.h"

EditAthlete::EditAthlete(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditAthlete)
{
    ui->setupUi(this);
}

EditAthlete::EditAthlete(QWidget *parent, QString firstname, QString lastname, bool gender, int age, double height, double weight, double resistance) :
    QDialog(parent),
    ui(new Ui::EditAthlete)
{
    ui->setupUi(this);


    ui->lineEdit_editAthlete_firstname->setText(firstname);
    ui->lineEdit_editAthlete_lastname->setText(lastname);
    ui->comboBox_editAthlete_gender->setCurrentIndex(!gender);
    ui->spinBox_editAthlete_age->setValue(age);
    ui->doubleSpinBox_editAthlete_height->setValue(height);
    ui->doubleSpinBox_editAthlete_weight->setValue(weight);
    ui->doubleSpinBox_editAthlete_resistance->setValue(resistance);
}

EditAthlete::~EditAthlete()
{
    delete ui;
}

Athlete EditAthlete::getAthleteObject()
{
    QString _firstname = ui->lineEdit_editAthlete_firstname->text();
    QString _lastname = ui->lineEdit_editAthlete_lastname->text();
    QString genderText = ui->comboBox_editAthlete_gender->itemText(ui->comboBox_editAthlete_gender->currentIndex());
    bool _gender;
    if (genderText == "Male")
    {
        _gender = 1;
    } else
    {
        _gender = 0;
    }

    int _age = ui->spinBox_editAthlete_age->value();
    double _height = ui->doubleSpinBox_editAthlete_height->value();
    double _weight = ui->doubleSpinBox_editAthlete_weight->value();
    double _resistance = ui->doubleSpinBox_editAthlete_resistance->value();

    Athlete athleteObject(_firstname, _lastname, _gender, _age, _height, _weight, _resistance);
    return athleteObject;
}
