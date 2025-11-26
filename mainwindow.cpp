#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMaximized();


    WindowSpecifyingPoints();//Функция создает в панели задания режимов начальную точку
    WindowChartCreate();//Функция добавляет начальный график на окно
    connect(ui->actionModBus_RTU, &QAction::triggered, ad_settings_modbus, &SettingsModbus::show);
    CreateNewLineSeries(1);
    CreateNewLineSeries(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::WindowChartCreate()
{
    ui->scrollArea_charts->setWidget(m_chartView);
    m_chartView->setChart(m_chart);

    m_axisX->setLabelFormat("%.2f");
    m_axisX->setTitleText("Время, мин");
    m_axisX->setGridLineVisible(true);
    m_axisX->setTickCount(1);
    m_axisX->setRange(0,30);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY->setLabelFormat("%d");
    m_axisY->setTitleText("РУД, %");
    m_axisY->setGridLineVisible(true);
    m_axisY->setTickCount(1);
    m_axisY->setRange(0,100);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
}
void MainWindow::FillingSeries(QLineSeries *series, double meaning_OX_start, double meaning_OX_finish,int d_meaning_OY)
{

    double X_start=0;
    double X_finish=0;
    double quantityPoints = (meaning_OX_finish-meaning_OX_start);
    if(quantityPoints<5)
    {
        X_start = meaning_OX_start*60;
        X_finish = meaning_OX_finish*60;
        double i=X_start;
        while(i<X_start+quantityPoints*60)
        {
            series->append(double(i/60), d_meaning_OY);//в минутах
            i+=0.1f;
        }
    }
    else
    {
        X_start = meaning_OX_start;
        X_finish = meaning_OX_finish;
        double i=X_start;
        while(i<X_start+quantityPoints)
        {
            series->append(double(i), d_meaning_OY);//в минутах
            i+=1.f;
        }
    }
    if (series && !(m_chart->series().contains(series)))
    {
        m_chart->addSeries(series);
        series->attachAxis(m_axisX);
        series->attachAxis(m_axisY);
    }
    m_chart->update();

}

void MainWindow::CreateNewLineSeries(int NumberLineSeries)
{
    QLineSeries *newLineSeries = new QLineSeries();
    QString string_lineSeries = LineSeries_objName + QString("%1").arg(NumberLineSeries);
    newLineSeries->setObjectName(string_lineSeries);
    if(!m_allSeries.contains(string_lineSeries))
    {
        m_allSeries[string_lineSeries] = newLineSeries;
    }

}
void MainWindow::RemoveNewLineSeries(int NumberLineSeries)
{
    QString string_lineSeries = LineSeries_objName+QString("%1").arg(NumberLineSeries);
    if (m_allSeries.contains(string_lineSeries))
    {
        // Удаляем из QHash
        m_allSeries.remove(string_lineSeries);
        m_chart->removeAllSeries();
    }

}
void MainWindow::WindowSpecifyingPoints()
{
    AddNewPoint(0);
    ui->pushButton_RemovePoint->setEnabled(false);

    connect(ui->pushButton_AddPoint, &QPushButton::clicked, [this]()
    {

        if(NumberPointMode>=1 && NumberPointMode<255)
        {
            AddNewPoint(NumberPointMode);
            ui->pushButton_RemovePoint->setEnabled(true);
            NumberPointMode++;
        }
        QScrollBar* verticalBar = ui->scrollArea->verticalScrollBar();
        verticalBar->setValue(verticalBar->maximum()+verticalBar->pageStep());//Опускаем вертикальный скрол вниз

        QString string_lineSeries = LineSeries_objName + QString("%1").arg(1);
        FillingSeries(m_allSeries.value(string_lineSeries, nullptr), 0.f,  0.25f,10);

        string_lineSeries = LineSeries_objName + QString("%1").arg(1);
        FillingSeries(m_allSeries.value(string_lineSeries, nullptr), 0.25f,  0.95f,70);

        string_lineSeries = LineSeries_objName + QString("%1").arg(1);
        FillingSeries(m_allSeries.value(string_lineSeries, nullptr), 0.95f,  1.95f,5);

        string_lineSeries = LineSeries_objName + QString("%1").arg(1);
        FillingSeries(m_allSeries.value(string_lineSeries, nullptr), 1.95f, 30.0f,50);

    });

    connect(ui->pushButton_RemovePoint, &QPushButton::clicked, [this]()
    {
        if(NumberPointMode>1)
        {
            NumberPointMode--;
            RemoveNewPoint(NumberPointMode);

            if(NumberPointMode<=1)
            {
                NumberPointMode=1;
                ui->pushButton_RemovePoint->setEnabled(false);
            }
        }
    RemoveNewLineSeries(1);
    });
}
void MainWindow::RemoveNewPoint(uint16_t NumberPoint)
{
    /*функция удаления атрибутов устройств модбас*/
    QString string_0 = newLabelNumberPoint_objName+QString("%1").arg(NumberPoint);
    QLabel *RemoveLabel_0 = findChild<QLabel *>(string_0);
    if(RemoveLabel_0!=nullptr)
    {
        delete RemoveLabel_0;
    }

    QString string_1 = newSpinBoxTime_objName+QString("%1").arg(NumberPoint);
    QDoubleSpinBox *RemoveSpinBox_0 = findChild<QDoubleSpinBox *>(string_1);
    if(RemoveSpinBox_0!=nullptr)
    {
        delete RemoveSpinBox_0;
    }

    QString string_2 = newSpinBoxRUD_objName+QString("%1").arg(NumberPoint);
    QSpinBox *RemoveSpinBox_1 = findChild<QSpinBox *>(string_2);
    if(RemoveSpinBox_1!=nullptr)
    {
        delete RemoveSpinBox_1;
    }

    QString string_3 = newSpinBoxPower_objName+QString("%1").arg(NumberPoint);
    QDoubleSpinBox *RemoveSpinBox_2 = findChild<QDoubleSpinBox *>(string_3);
    if(RemoveSpinBox_2!=nullptr)
    {
        delete RemoveSpinBox_2;
    }

    QString string_4 = newLineEditNameMode_objName+QString("%1").arg(NumberPoint);
    QLineEdit *RemoveLineEdit_0 = findChild<QLineEdit *>(string_4);
    if(RemoveLineEdit_0!=nullptr)
    {
        delete RemoveLineEdit_0;
    }

    QString string_5 = newLabelStatus_objName+QString("%1").arg(NumberPoint);
    QLabel *RemoveLabel_1 = findChild<QLabel *>(string_5);
    if(RemoveLabel_1!=nullptr)
    {
        delete RemoveLabel_1;
    }
}
void MainWindow::AddNewPoint(uint16_t NumberPoint)
{
    /*Функция добавления атрибутов для устройств модбас*/
    uint8_t labelNumberPoint_column=0,  spinBoxTime_column=2, spinBoxRUD_column=1, spinBoxPower_column=3, lineEditNameMode_column=4, labelStatus_column=5;
    uint8_t Row = NumberPoint+1;

    QLabel *newLabelNumberPoint = new QLabel();
    QString string_labelNumberPoint = newLabelNumberPoint_objName+QString("%1").arg(NumberPoint);
    newLabelNumberPoint->setObjectName(string_labelNumberPoint);
    newLabelNumberPoint->setText(tr("%1").arg(NumberPoint));
    newLabelNumberPoint->setMaximumSize(30,20);
    ui->gridLayout_manual->addWidget(newLabelNumberPoint, Row, labelNumberPoint_column);//добавляем виджет на компановку

    QDoubleSpinBox *newSpinBoxTime = new QDoubleSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxID = newSpinBoxTime_objName+QString("%1").arg(NumberPoint);
    newSpinBoxTime->setObjectName(string_spinBoxID);
    newSpinBoxTime->setMinimumSize(100,20);
    newSpinBoxTime->setSuffix(" мин");
    ui->gridLayout_manual->addWidget(newSpinBoxTime, Row, spinBoxTime_column);

    QSpinBox *newSpinBoxRUD = new QSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxRUD = newSpinBoxRUD_objName+QString("%1").arg(NumberPoint);
    newSpinBoxRUD->setObjectName(string_spinBoxRUD);
    newSpinBoxRUD->setMinimumSize(80,20);
    newSpinBoxRUD->setRange(0, 100);
    ui->gridLayout_manual->addWidget(newSpinBoxRUD, Row, spinBoxRUD_column);

    QDoubleSpinBox *newSpinBoxPower = new QDoubleSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxPower = newSpinBoxPower_objName+QString("%1").arg(NumberPoint);
    newSpinBoxPower->setObjectName(string_spinBoxPower);
    newSpinBoxPower->setMinimumSize(80,20);
    newSpinBoxPower->setRange(0, 200);
    newSpinBoxPower->setPrefix(">");
    ui->gridLayout_manual->addWidget(newSpinBoxPower, Row, spinBoxPower_column);

    QLineEdit *newLineEditNameMode = new QLineEdit();
    QString string_lineEditNameMode = newLineEditNameMode_objName+QString("%1").arg(NumberPoint);
    newLineEditNameMode->setObjectName(string_lineEditNameMode);
    newLineEditNameMode->setText(tr("Режим %1").arg(NumberPoint));
    newLineEditNameMode->setMinimumSize(60,20);
    ui->gridLayout_manual->addWidget(newLineEditNameMode, Row, lineEditNameMode_column);//добавляем виджет на компановку

    QLabel *newLabelStatus = new QLabel();
    QString string_labelStatus = newLabelStatus_objName+QString("%1").arg(NumberPoint);
    newLabelStatus->setObjectName(string_labelStatus);
    newLabelStatus->setText(tr("Не выполнен"));
    newLabelStatus->setMaximumSize(85,20);
    ui->gridLayout_manual->addWidget(newLabelStatus, Row, labelStatus_column);//добавляем виджет на компановку


}
