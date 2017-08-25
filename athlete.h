#ifndef ATHLETE_H
#define ATHLETE_H

#include <QString>
#include <QVector>

class Athlete
{
public:
    Athlete();
    Athlete(QString firstname, QString lastname, bool gender, int age, double height, double weight, double resistance);
    void setFirstName(QString firstname);
    void setLastName(QString lastname);
    void setGender(bool gender); // 0 = female, 1 = male
    void setAge(int age);
    void setHeight(double height);
    void setWeight(double weight);
    void setResistance(double resistance);
    QString getFirstName() const;
    QString getLastName() const;
    bool getGender() const;
    int getAge() const;
    double getHeight() const;
    double getWeight() const;
    double getResistance() const;
    QVector<double> getData(int attempt) const;
    int getNumAttempts() const;
    void pushData(QVector<double> dataPoints);
    void removeData(int index);
private:
    QString first_name;
    QString last_name;
    bool gender;
    int age;
    double height;
    double weight;
    double resistance;
    QVector < QVector<double> > data;
};

#endif // ATHLETE_H
