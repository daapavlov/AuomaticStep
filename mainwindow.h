#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "settingsmodbus.h"
#include "qcustomplot.h"
#include "settingsusertime.h"
#include "mouseclickhandler.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <filedialog.h>


QT_BEGIN_NAMESPACE
#define NoSelected  -0xffff

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
    QTimer *timer_mode = new QTimer();
    QTimer *timer_modeMessage = new QTimer();
    QTimer *timer_sendRUD = nullptr;
    QTimer timer_readPointMode;
    QThread *threadFile = new QThread();//Создаем новый поток
//    MouseClickHandler *handler;

    uint16_t NumberPointMode=0;
    int CurrentRegime=0;
    bool sendData_RUD=0;
    struct PointMode
    {
        QString number = 0;
        uint8_t RUD=0;
        double Time=0;
        double PowerDVS=0;
        QString NameMode;
        QString status;
    };
    PointMode ArrayPoint[500];

    //Названия объектов в панели управления РУД
    QString newLabelHandler_objName = "label_Handler_";
    QString newLabelNumberPoint_objName = "label_numberPoint_";
    QString newSpinBoxTime_objName = "spinBoxTime_";
    QString newSpinBoxRUD_objName = "spinBoxRUD_";
    QString newSpinBoxPower_objName = "spinBoxPower_";
    QString newLineEditNameMode_objName = "lineEditNameMode_";
    QString newLabelStatus_objName = "label_status_";


    SettingsModbus *m_settingsModbus = nullptr;
    QVector <uint16_t>dataRe = {444};
    QString ErrorModBus;

    SettingsUserTime *m_userTime = new SettingsUserTime();

    fileDialog *m_fileDialog = new fileDialog();

    int SelectedObject=NoSelected;

    void AddNewPoint(uint16_t NumberPoint);
    void CollbackButtonAddPoint();
    void CollbackButtonRemovePoint();
    void AddNullPointToPosition(int numberPoint, int MaxPoint);
    void RemoveNullPointToPosition(int numberPoint, int MaxPoint);
    void RemoveNewPoint(uint16_t NumberPoint);
    void WindowSpecifyingPoints();
    void GetParametrFromForms(PointMode *structure, uint16_t NumberPoint);
    void SendParametrFromForms(PointMode structure, uint16_t NumberPoint);
    void ClearParametrStruct(PointMode *structure);
    void SplittingIntoDots(PointMode structure_1, QVector<double> *vector1, QVector<double> *vector2);
    uint16_t GetNumberEndPointMode();
    bool SetFlagFinishMode(QString string, uint16_t NumberPoint, bool EnabledTime);
    uint16_t GetParametrRUD(uint16_t NumberPoint);

    void AddPlotToWindow(QCustomPlot *custom_plot);
    void AddNewDataPointrChart(QCustomPlot *custom_plot, QVector<double> DataX, QVector <double> DataY, QMap <double, QString> point);
    void RemoveDataPointChart(QCustomPlot *custom_plot);
    void setupNearestPointTracking();
    void ConnectMouse();
    void onMouseMoveNearestPoint(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *event);
    void playWindowsSystemSound(int type);
    void removeRow(QGridLayout *grid, int Row);
    void isertRow(int NumberPoint, int newRow);
    void setStyle(bool action, int row, int column);

private slots:
    void AddPointToChrts();
    void SendRud_timeout();
    void Mode_timeout();
    void Mode_stop();
    void Mode_start();
    void Mode_MessageTheEnd();
    void OpenProject();
    void SaveProject();
};
#endif // MAINWINDOW_H
