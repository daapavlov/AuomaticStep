#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QObject>
#include <QMap>
#include <fstream>
#include <iostream>
#include <QFile>
#include <QFileDialog>
#include<QTextStream>


using namespace std;

class fileDialog : public QObject
{
    Q_OBJECT
public:
    explicit fileDialog(QObject *parent = nullptr);
    QString OpenFile();
    bool WriteFileSettings(QString directory, QMap <QString, QString> map);
    bool ReadFileSettings(QString directory, QMap <QString, QString> &map);

signals:
private:


};

#endif // FILEDIALOG_H
