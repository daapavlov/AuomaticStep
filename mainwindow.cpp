#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QScrollBar>
#include <QTimer>
#include <windows.h>
#ifdef Q_OS_WIN
#include <windows.h>
#include <mmsystem.h>
//#pragma comment(lib, "winmm.lib")
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_settingsModbus = new SettingsModbus();
    timer_sendRUD = new QTimer();
//    showMaximized();
    AddPlotToWindow(m_customPlot);
    WindowSpecifyingPoints();//Функция создает в панели задания режимов начальную точку
    setupNearestPointTracking();
    connect(ui->actionModBus_RTU, &QAction::triggered, m_settingsModbus, &SettingsModbus::show);
    connect(ui->action_times, &QAction::triggered, m_userTime, &SettingsUserTime::show);

    connect(ui->pushButton_writePoints, &QPushButton::clicked, this, &MainWindow::AddPointToChrts);

    connect(ui->pushButton_start, &QPushButton::clicked, [this]
    {

        if(ui->pushButton_start->text()=="Запустить")
        {
            Mode_start();
        }
        else
        {
            Mode_stop();
        }
    });
    connect(m_settingsModbus, &SettingsModbus::dataReceived, this, [&]
    {
//        dataRe = m_settingsModbus->GetModbusData_Receive();
        ui->plainTextEdit_status->appendPlainText(QString("Считано значение %1").arg(dataRe.first()));
    });

//    connect(threadFile, &QThread::started, this, [&]()
//    {

//    });
    connect(timer_sendRUD, &QTimer::timeout, this, &MainWindow::SendRud_timeout);
    connect(timer_mode, &QTimer::timeout, this, &MainWindow::Mode_timeout);
    connect(timer_modeMessage, &QTimer::timeout, this, &MainWindow::Mode_Message);

}


MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::Mode_Message()
{
    timer_modeMessage->stop();
    MessageBeep(MB_OK);

}
void MainWindow::Mode_timeout()
{
    QDateTime newTime = QDateTime::currentDateTime();

    /*функция отмеряет заданный режим*/


    PointMode stru;
    GetParametrFromForms(&stru, CurrentRegime);
    int interval = stru.Time*60000;//в миллисекундх
    if(sendData_RUD==1)
    {

        timer_mode->setInterval(interval+500);
        if(CurrentRegime>0)
        {
           SetFlagFinishMode("Выполнен", CurrentRegime-1, 0);
        }


        if(CurrentRegime>=GetNumberPointMode())
        {
            Mode_stop();
            //РУД в 0
        }
        else
        {
//            PlaySound(TEXT("SystemAsterisk"), NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
            MessageBeep(MB_OK);
            timer_modeMessage->start(interval-(m_userTime->GetParametr(2)*60000));
            ui->plainTextEdit_status->appendPlainText(QString("%1Выполняется точка %2").arg(newTime.toString("hh:mm:ss ")).arg(CurrentRegime));
            SetFlagFinishMode("Выполняется", CurrentRegime, 0);
            if(CurrentRegime == GetNumberPointMode()-1)
            {
                ui->pushButton_RemovePoint->setEnabled(false);
            }
            CurrentRegime+=1;

        }
    }
}
void MainWindow::Mode_start()
{
    QDateTime newTime = QDateTime::currentDateTime();
    PointMode mode;
    GetParametrFromForms(&mode, 0);

    if(mode.Time>0 && m_settingsModbus->Flags_setSettings_comPort==1)
    {
        ui->actionModBus_RTU->setEnabled(false);
        ui->action_times->setEnabled(false);
        ui->plainTextEdit_status->clear();
        timer_mode->start(m_userTime->GetParametr(0)*1000);
        timer_sendRUD->start(m_userTime->GetParametr(1));
        ui->pushButton_start->setText("Остановить");
    }
    else
    {
        ui->plainTextEdit_status->appendPlainText(QString("%1Не удалось запустить цикл. Проверьте первого длительность цикла").arg(newTime.toString("hh:mm:ss ")));
    }
}
void MainWindow::Mode_stop()
{
    ui->actionModBus_RTU->setEnabled(true);
    ui->action_times->setEnabled(true);
//    PlaySound(TEXT("SystemAsterisk"), NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
    MessageBeep(MB_OK);
    QDateTime newTime = QDateTime::currentDateTime();
    timer_mode->stop();
    timer_sendRUD->stop();
    CurrentRegime=0;
    SendRud_timeout();
    ui->plainTextEdit_status->appendPlainText(QString("%1Выполнена остановка цикла").arg(newTime.toString("hh:mm:ss ")));
    for(int i=0;i<GetNumberPointMode();i++)
    {
        SetFlagFinishMode("Не выполнен", i, 1);
    }
    ui->pushButton_start->setText("Запустить");
}
void MainWindow::SendRud_timeout()
{
    /*функция отправляет значение РУД по модбасу*/
    QDateTime newTime = QDateTime::currentDateTime();
    uint16_t addressRUD;
    uint16_t numberFirstRegistersRUD;
    uint16_t quantity;
    uint16_t ValueRud[1];
    if(CurrentRegime>0)
    {
         ValueRud[0] = GetParametrRUD(CurrentRegime-1);
    }
    else
    {
       ValueRud[0]=0;
    }

    if(m_settingsModbus)
    {
        if(m_settingsModbus->GetSettingsDevice_modbusRTU(0,&addressRUD, &numberFirstRegistersRUD, &quantity))
        {
            if(quantity>=1)
            {
                if((m_settingsModbus->SendData_ModbusRTU(addressRUD, numberFirstRegistersRUD, ValueRud, 1))==false)
                {
                    if(m_settingsModbus->ErrorMessage_transmission!="no error")
                    {
                       ui->plainTextEdit_status->appendPlainText(m_settingsModbus->ErrorMessage_transmission);
                       sendData_RUD=0;
                    }
                    else
                    {
                        sendData_RUD=1;
                    }
                }
            }
            else
            {
                    ui->plainTextEdit_status->appendPlainText(QString("%1Превышено число допустимых регистров для записи").arg(newTime.toString("hh:mm:ss ")));
                    sendData_RUD=0;
            }
        }
        else
        {
                ui->plainTextEdit_status->appendPlainText(QString("%1Не существует адреса РУД").arg(newTime.toString("hh:mm:ss ")));
                sendData_RUD=0;
        }

    }

}
void MainWindow::setupNearestPointTracking()
{
    m_customPlot->setMouseTracking(true);

    // Создаем точку для отображения
    m_hoverPoint = new QCPItemEllipse(m_customPlot);
    m_hoverPoint->setPen(QPen(Qt::green, 2));
    m_hoverPoint->setBrush(QBrush(QColor(255, 0, 0, 100)));
    m_hoverPoint->topLeft->setType(QCPItemPosition::ptAbsolute);
    m_hoverPoint->bottomRight->setType(QCPItemPosition::ptAbsolute);
    m_hoverPoint->setVisible(false);

    // Текст с информацией
     m_hoverText = new QCPItemText(m_customPlot);
     m_hoverText->setPositionAlignment(Qt::AlignTop | Qt::AlignRight);
     m_hoverText->position->setType(QCPItemPosition::ptAxisRectRatio);
     m_hoverText->position->setCoords(0.98, 0.02);
     m_hoverText->setText("Наведите на график");
     m_hoverText->setTextAlignment(Qt::AlignRight);
     m_hoverText->setFont(QFont(font().family(), 9));
     m_hoverText->setBrush(QBrush(QColor(255, 255, 255, 200)));
     m_hoverText->setPen(QPen(Qt::black));

    connect(m_customPlot, &QCustomPlot::mouseMove, this, &MainWindow::onMouseMoveNearestPoint);
}
void MainWindow::onMouseMoveNearestPoint(QMouseEvent* event)
{
    if (m_customPlot->graphCount() == 0) return;

    double x = m_customPlot->xAxis->pixelToCoord(event->pos().x());
//    double y = m_customPlot->yAxis->pixelToCoord(event->pos().y());

    // Ищем ближайшую точку данных
    QCPGraph* graph = m_customPlot->graph(0);
    double minDistance = std::numeric_limits<double>::max();
    QCPGraphDataContainer::const_iterator closestPoint = graph->data()->constEnd();

    for (auto it = graph->data()->constBegin(); it != graph->data()->constEnd(); ++it) {
        double distance = qAbs(it->key - x);
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = it;
        }
    }

    if (closestPoint != graph->data()->constEnd()) {
        // Обновляем позицию точки
        double pointSize = 8;

         m_hoverPoint->topLeft->setCoords(
            m_customPlot->xAxis->coordToPixel(closestPoint->key) - pointSize/2,
            m_customPlot->yAxis->coordToPixel(closestPoint->value) - pointSize/2
        );
         m_hoverPoint->bottomRight->setCoords(
            m_customPlot->xAxis->coordToPixel(closestPoint->key) + pointSize/2,
            m_customPlot->yAxis->coordToPixel(closestPoint->value) + pointSize/2
        );

         m_hoverPoint->setVisible(true);

        // Обновляем текст
         m_hoverText->setText(
            QString("РУД: %2 %\n Время %1 мин")
                .arg(closestPoint->key, 0, 'f', 1)
                .arg(closestPoint->value, 0, 'f', 1)
                //.arg(minDistance, 0, 'f', 3)
        );
    } else {
         m_hoverPoint->setVisible(false);
         m_hoverText->setText("Наведите на график");
    }

    m_customPlot->replot();
}
void MainWindow::AddPlotToWindow(QCustomPlot *custom_plot)
{
    ui->scrollArea_charts->setWidget(custom_plot);
    custom_plot->addGraph();
    custom_plot->yAxis->setRange(0, 110);
    custom_plot->xAxis->setRange(0, 50);
    custom_plot->graph(0)->setPen((QPen(Qt::black, 4)));


    // Включаем взаимодействие с мышью
    custom_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Настраиваем оси для масштабирования и перемещения
    custom_plot->axisRect()->setRangeDrag(Qt::Horizontal );//| Qt::Vertical
    custom_plot->axisRect()->setRangeZoom(Qt::Horizontal);// | Qt::Vertical);

    // Плавное масштабирование к курсору мыши
    custom_plot->setSelectionTolerance(10); // Чувствительность выделения
}
void MainWindow::AddNewDataPointrChart(QCustomPlot *custom_plot, QVector<double> DataX, QVector <double> DataY, QMap <double, QString> point)
{
    /*Добавляем точки на график и устанавливаем оси*/
    if(!DataX.isEmpty() && !DataY.isEmpty())
    {
        custom_plot->graph(0)->setData(DataX, DataY);
//        custom_plot->yAxis->setRange(0, 100);
        QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);

        for (double key : point.keys()) {
            textTicker->addTick(key, point[key]);
        }
        m_customPlot->yAxis->setTicker(textTicker);
        custom_plot->xAxis->setRange(DataX.first(), DataX.back());
        custom_plot->replot();
    }

}
void MainWindow::RemoveDataPointChart(QCustomPlot *custom_plot)
{
    /*Удаляем данные с графика*/
    if (custom_plot->graphCount() > 0)
    {
        custom_plot->graph(0)->data()->clear();
        custom_plot->replot();
    }
}
void MainWindow::GetParametrFromForms(PointMode *structure, uint16_t NumberPoint)
{
    /*функция читает параметры с формы задания режимов*/
    QString string_1 = newSpinBoxTime_objName+QString("%1").arg(NumberPoint);
    QDoubleSpinBox *SpinBox_0 = findChild<QDoubleSpinBox *>(string_1);
    if(SpinBox_0!=nullptr)
    {
        structure->Time = SpinBox_0->value();
    }
    else
    {
        return;
    }


    QString string_2 = newSpinBoxRUD_objName+QString("%1").arg(NumberPoint);
    QSpinBox *SpinBox_1 = findChild<QSpinBox *>(string_2);
    if(SpinBox_1!=nullptr)
        structure->RUD = SpinBox_1->value();

    QString string_3 = newSpinBoxPower_objName+QString("%1").arg(NumberPoint);
    QDoubleSpinBox *SpinBox_2 = findChild<QDoubleSpinBox *>(string_3);
    if(SpinBox_2!=nullptr)
        structure->PowerDVS = SpinBox_2->value();

    QString string_4 = newLineEditNameMode_objName+QString("%1").arg(NumberPoint);
    QLineEdit *LineEdit_0 = findChild<QLineEdit *>(string_4);
    if(LineEdit_0!=nullptr)
        structure->NameMode = LineEdit_0->text();
}
bool MainWindow::SetFlagFinishMode(QString string, uint16_t NumberPoint, bool EnabledTime)
{
    QString string_0 = newLabelStatus_objName+QString("%1").arg(NumberPoint);
    QLabel *RemoveLabel_1 = findChild<QLabel *>(string_0);
    if(RemoveLabel_1!=nullptr)
    {
        RemoveLabel_1->setText(string);
    }
    QString string_1 = newSpinBoxTime_objName+QString("%1").arg(NumberPoint);
    QDoubleSpinBox *SpinBox_0 = findChild<QDoubleSpinBox *>(string_1);
    if(SpinBox_0!=nullptr)
    {
        SpinBox_0->setEnabled(EnabledTime);
    }


    return 1;
}
uint16_t MainWindow::GetParametrRUD(uint16_t NumberPoint)
{
    QString string_2 = newSpinBoxRUD_objName+QString("%1").arg(NumberPoint);
    QSpinBox *SpinBox_1 = findChild<QSpinBox *>(string_2);
    if(SpinBox_1!=nullptr)
    {
       return SpinBox_1->value();
    }

    return 0;
}
void MainWindow::ClearParametrStruct(PointMode *structure)
{
    /*Функция очищает т-элеент структуры*/
    structure->RUD = 0;
    structure->Time = 0;
    structure->PowerDVS = 0;
    structure->NameMode = "Не задан";
}
void MainWindow::AddPointToChrts()
{
    QMap <double, QString> points;
    /*Функция записывает на график заново точки, перед этим очищает вектора*/
    DataChartX.clear();
    DataChartY.clear();
    RemoveDataPointChart(m_customPlot);
    int i=0;
    while(i<GetNumberPointMode())
    {
        GetParametrFromForms(&ArrayPoint[i], i);
        points.insert(ArrayPoint[i].RUD, ArrayPoint[i].NameMode);
        SplittingIntoDots(ArrayPoint[i], &DataChartX, &DataChartY);
        i++;
    }
    AddNewDataPointrChart(m_customPlot, DataChartX, DataChartY, points);
}
uint16_t MainWindow::GetNumberPointMode()
{
    return NumberPointMode;
}
void MainWindow::SplittingIntoDots(PointMode structure_1, QVector<double> *vector1, QVector<double> *vector2)
{
    /*функция добавляет заполняет вектора точками*/
    double Min=0;
    if(!vector1->isEmpty())
    {
        Min = vector1->back();
    }

    double MinRange_OX = Min*60.0f;//в секундах
    double Range_OX = (structure_1.Time)*60.0f;//в секундах
    double Rud = structure_1.RUD;
    for(double i=MinRange_OX; i<(MinRange_OX+Range_OX)+1.0f; i+=1.0f)
    {
        vector1->append(i/60.0f);//чтобы отображать точки на графике в минутах
        vector2->append(Rud);
    }
}
void MainWindow::WindowSpecifyingPoints()
{
    /*Начальная инициализация*/
    AddNewPoint(0);
//    GetParametrToForms(&ArrayPoint[0], 0);
    ui->pushButton_RemovePoint->setEnabled(false);

    connect(ui->pushButton_AddPoint, &QPushButton::clicked, [this]()
    {

        if(NumberPointMode>=1 && NumberPointMode<255)
        {
            AddNewPoint(NumberPointMode);
            ui->pushButton_RemovePoint->setEnabled(true);
            GetParametrFromForms(&ArrayPoint[NumberPointMode], NumberPointMode);
            NumberPointMode++;
        }
        QScrollBar* verticalBar = ui->scrollArea->verticalScrollBar();
        verticalBar->setValue(verticalBar->maximum()+verticalBar->pageStep());//Опускаем вертикальный скрол вниз

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
                ClearParametrStruct(&ArrayPoint[NumberPointMode]);
                ui->pushButton_RemovePoint->setEnabled(false);
            }
        }
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
    newSpinBoxTime->setRange(0.f, 1500.f);
    ui->gridLayout_manual->addWidget(newSpinBoxTime, Row, spinBoxTime_column);

    QSpinBox *newSpinBoxRUD = new QSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxRUD = newSpinBoxRUD_objName+QString("%1").arg(NumberPoint);
    newSpinBoxRUD->setObjectName(string_spinBoxRUD);
    newSpinBoxRUD->setMinimumSize(80,20);
    newSpinBoxRUD->setRange(0, 100);
    newSpinBoxRUD->setSuffix(" %");
    ui->gridLayout_manual->addWidget(newSpinBoxRUD, Row, spinBoxRUD_column);

    QDoubleSpinBox *newSpinBoxPower = new QDoubleSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxPower = newSpinBoxPower_objName+QString("%1").arg(NumberPoint);
    newSpinBoxPower->setObjectName(string_spinBoxPower);
    newSpinBoxPower->setMinimumSize(80,20);
    newSpinBoxPower->setRange(0, 1000);
    newSpinBoxPower->setPrefix(">");
    newSpinBoxPower->setSuffix(" ЛС");
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
