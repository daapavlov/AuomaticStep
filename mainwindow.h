#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "settingsmodbus.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QValueAxis>
QT_BEGIN_NAMESPACE
QT_CHARTS_USE_NAMESPACE

class QModbusClient;
class QModbusReply;

class SettingsModbus;
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SettingsModbus *ad_settings_modbus = new SettingsModbus();

    QChartView *m_chartView  = new QChartView();
    QChart *m_chart = new QChart();
    QValueAxis *m_axisX = new QValueAxis;
    QValueAxis *m_axisY = new QValueAxis;

    uint16_t NumberPointMode=1;

    //Названия объектов в панели управления РУД
    QString newLabelNumberPoint_objName = "label_numberPoint_";
    QString newSpinBoxTime_objName = "spinBoxTime_";
    QString newSpinBoxRUD_objName = "spinBoxRUD_";
    QString newSpinBoxPower_objName = "spinBoxPower_";
    QString newLineEditNameMode_objName = "lineEditNameMode_";
    QString newLabelStatus_objName = "label_status_";

    QString LineSeries_objName = "lineSeries_";
    QHash<QString, QLineSeries*> m_allSeries; // Все созданные series


    void AddNewPoint(uint16_t NumberPoint);
    void RemoveNewPoint(uint16_t NumberPoint);
    void WindowSpecifyingPoints();
    void WindowChartCreate();
    void CreateNewLineSeries(int NumberLineSeries);
    void RemoveNewLineSeries(int NumberLineSeries);
    void FillingSeries(QLineSeries *series, double meaning_OX_start, double meaning_OX_finish,int d_meaning_OY);
};
#endif // MAINWINDOW_H
