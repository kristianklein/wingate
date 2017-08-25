#include "athlete.h"

Athlete::Athlete()
{
    this->first_name = "";
    this->last_name = "";
    this->gender = true;
    this->age = 0;
    this->height = 0;
    this->weight = 0;
}

Athlete::Athlete(QString firstname, QString lastname, bool gender, int age, double height, double weight, double resistance)
{
    this->first_name = firstname;
    this->last_name = lastname;
    this->gender = gender;
    this->age = age;
    this->height = height;
    this->weight = weight;
    this->resistance = resistance;
}

void Athlete::setAge(int age)
{
    this->age = age;
}

void Athlete::setFirstName(QString firstname)
{
    this->first_name = firstname;
}

void Athlete::setLastName(QString lastname)
{
    this->last_name = lastname;
}

void Athlete::setGender(bool gender)
{
    this->gender = gender;
}

void Athlete::setHeight(double height)
{
    this->height = height;
}

void Athlete::setWeight(double weight)
{
    this->weight = weight;
}

void Athlete::setResistance(double resistance)
{
    this->resistance = resistance;
}

QString Athlete::getFirstName() const
{
    return this->first_name;
}

QString Athlete::getLastName() const
{
    return this->last_name;
}

bool Athlete::getGender() const
{
    return this->gender;
}

int Athlete::getAge() const
{
    return this->age;
}

double Athlete::getHeight() const
{
    return this->height;
}

double Athlete::getWeight() const
{
    return this->weight;
}

double Athlete::getResistance() const
{
    return this->resistance;
}

QVector<double> Athlete::getData(int attempt) const
{
    return this->data.at(attempt);
}

int Athlete::getNumAttempts() const
{
    return this->data.size();
}

void Athlete::pushData(QVector<double> dataPoints)
{
    this->data.push_back(dataPoints);
}

void Athlete::removeData(int index)
{
    this->data.remove(index);
}
