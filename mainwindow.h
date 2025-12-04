#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "settingsmodbus.h"
#include "qcustomplot.h"

#include <QMouseEvent>

QT_BEGIN_NAMESPACE


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

    QCustomPlot *m_customPlot = new QCustomPlot();
    QVector <double>  DataChartX, DataChartY;
    // Элементы для отображения при наведении
    QCPItemEllipse* m_hoverPoint;
    QCPItemText* m_hoverText;

    uint16_t NumberPointMode=1;
    struct PointMode
    {
        uint8_t RUD=0;
        double Time=0;
        double PowerDVS=0;
        QString NameMode;
    };
    PointMode ArrayPoint[100];

    //Названия объектов в панели управления РУД
    QString newLabelNumberPoint_objName = "label_numberPoint_";
    QString newSpinBoxTime_objName = "spinBoxTime_";
    QString newSpinBoxRUD_objName = "spinBoxRUD_";
    QString newSpinBoxPower_objName = "spinBoxPower_";
    QString newLineEditNameMode_objName = "lineEditNameMode_";
    QString newLabelStatus_objName = "label_status_";


    SettingsModbus *m_settingsModbus = new SettingsModbus();
    QVector <uint16_t>dataRe = {444};
    QString ErrorModBus;

    void AddNewPoint(uint16_t NumberPoint);
    void RemoveNewPoint(uint16_t NumberPoint);
    void WindowSpecifyingPoints();
    void GetParametrToForms(PointMode *structure, uint16_t NumberPoint);
    void ClearParametrStruct(PointMode *structure);
    void SplittingIntoDots(PointMode structure_1, QVector<double> *vector1, QVector<double> *vector2);

    void AddPlotToWindow(QCustomPlot *custom_plot);
    void AddNewDataPointrChart(QCustomPlot *custom_plot, QVector<double> DataX, QVector <double> DataY);
    void RemoveDataPointChart(QCustomPlot *custom_plot);
    void setupNearestPointTracking();
    void onMouseMoveNearestPoint(QMouseEvent* event);

private slots:
    void AddPointToChrts();
};
#endif // MAINWINDOW_H
