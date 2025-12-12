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
    ConnectMouse();

    connect(ui->actionModBus_RTU, &QAction::triggered, m_settingsModbus, &SettingsModbus::show);
    connect(ui->actionModBus_RTU, &QAction::triggered, m_settingsModbus, &SettingsModbus::UpdatePortName);
    connect(ui->action_times, &QAction::triggered, m_userTime, &SettingsUserTime::show);
    connect(ui->action_openFile, &QAction::triggered, this, &MainWindow::OpenProject);
    connect(ui->action_saveFile, &QAction::triggered, this, &MainWindow::SaveProject);
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
    connect(timer_modeMessage, &QTimer::timeout, this, &MainWindow::Mode_MessageTheEnd);

}


MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::ConnectMouse()
{
    MouseClickHandler *handler_scrollArea = new MouseClickHandler(ui->scrollArea);
    connect(handler_scrollArea, &MouseClickHandler:: leftClicked,
                 [=](const QPoint &pos) {
        setFocus(); // в конструкторе или при необходимости
        setFocusPolicy(Qt::StrongFocus); // установите политику фокуса
         });
    connect(handler_scrollArea, &MouseClickHandler:: rightClicked,
                 [=](const QPoint &pos) {

                 setStyle(0, SelectedObject+1, 0);

                 SelectedObject=-1;
//             }
         });
}
void MainWindow::OpenProject()
{
    QMap <QString, QString> SettingsProject;
    PointMode t_pointMode;
    QString cleanData;
    if(!m_fileDialog->ReadFileSettings("", SettingsProject))
    {
        QMessageBox::critical(this, "Ошибка!", "Не удалось открыть файл");
    }
    int QuantityPoint = GetNumberEndPointMode();

    for(int i=0; i<QuantityPoint; i++)
    {
        CollbackButtonRemovePoint();
    }
    NumberPointMode=0;
    for(int i=0; i<SettingsProject["quantityPointMode"].toInt(); i++)
    {
//        AddNewPoint(i);

        CollbackButtonAddPoint();
        QStringList parts = SettingsProject[QString("point_%1").arg(i)].split(';');
        if (parts.size() >= 4) {
            t_pointMode.RUD = parts[0].trimmed().toInt();
            t_pointMode.Time = parts[1].trimmed().toDouble();
            t_pointMode.PowerDVS = parts[2].trimmed().toDouble();
            t_pointMode.NameMode = parts[3].trimmed();
            t_pointMode.status = "Не выполнен";
        } else {

        }
        SendParametrFromForms(t_pointMode, i);
    }

    NumberPointMode=SettingsProject["quantityPointMode"].toInt();
}
void MainWindow::SaveProject()
{
    QMap <QString, QString> SettingsProject;
    PointMode t_pointMode;
    SettingsProject["quantityPointMode"]=QString("%1").arg(GetNumberEndPointMode());
    for(int i=0; i<GetNumberEndPointMode(); i++)
    {
        GetParametrFromForms(&t_pointMode, i);
        if(t_pointMode.NameMode=="")
        {
            t_pointMode.NameMode=" ";
        }
        SettingsProject[QString("point_%1").arg(i)]=QString("%1;%2;%3;%4\n").arg(t_pointMode.RUD).arg(t_pointMode.Time).arg(t_pointMode.PowerDVS).arg(t_pointMode.NameMode);
    }
    if(!m_fileDialog->WriteFileSettings("", SettingsProject))
    {
        QMessageBox::critical(this, "Ошибка!", "Не удалось сохранить файл");
    }
}
void MainWindow::Mode_MessageTheEnd()
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


        if(CurrentRegime>=GetNumberEndPointMode())
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
            if(CurrentRegime == GetNumberEndPointMode()-1)
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

    if(m_settingsModbus->Flags_setSettings_comPort==1)
    {
        if(mode.Time>0)
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
    else
    {
        ui->plainTextEdit_status->appendPlainText(QString("%1Не удалось запустить цикл. Проверьте настройки modbus").arg(newTime.toString("hh:mm:ss ")));
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
    for(int i=0;i<GetNumberEndPointMode();i++)
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

    QString string_0 = newLabelNumberPoint_objName+QString("%1").arg(NumberPoint);
    QLabel *RemoveLabel_0 = findChild<QLabel *>(string_0);
    structure->number = RemoveLabel_0->text();

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

    QString string_5 = newLabelStatus_objName+QString("%1").arg(NumberPoint);
    QLabel *Label_1 = findChild<QLabel *>(string_5);
    if(Label_1!=nullptr)
        structure->status=Label_1->text();
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
    while(i<GetNumberEndPointMode())
    {
        GetParametrFromForms(&ArrayPoint[i], i);
//        points.insert(ArrayPoint[i].RUD, ArrayPoint[i].NameMode);
        points.insert(ArrayPoint[i].PowerDVS, ArrayPoint[i].NameMode);//
        SplittingIntoDots(ArrayPoint[i], &DataChartX, &DataChartY);
        i++;
    }
    AddNewDataPointrChart(m_customPlot, DataChartX, DataChartY, points);
}
uint16_t MainWindow::GetNumberEndPointMode()
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
void MainWindow::CollbackButtonAddPoint()
{
    if(NumberPointMode>=0 && NumberPointMode<255)
    {
        AddNewPoint(NumberPointMode);
        ui->pushButton_RemovePoint->setEnabled(true);
        if(SelectedObject!=NoSelected)
        {
            PointMode pooint;
            GetParametrFromForms(&pooint, SelectedObject);
            AddNullPointToPosition(SelectedObject, NumberPointMode);
            SendParametrFromForms(pooint, SelectedObject+1);
        }
        GetParametrFromForms(&ArrayPoint[NumberPointMode], NumberPointMode);
        NumberPointMode++;
        QScrollBar* verticalBar = ui->scrollArea->verticalScrollBar();
        verticalBar->setValue(verticalBar->maximum()+verticalBar->pageStep());//Опускаем вертикальный скрол вниз
    }
}
void MainWindow::CollbackButtonRemovePoint()
{
    if(NumberPointMode>0)
    {
        if(SelectedObject!=NoSelected)
        {
            RemoveNullPointToPosition(SelectedObject, NumberPointMode);
        }

        NumberPointMode--;
        RemoveNewPoint(NumberPointMode);
        ClearParametrStruct(&ArrayPoint[NumberPointMode]);

        if(NumberPointMode<=0)
        {
            NumberPointMode=0;
            ui->pushButton_RemovePoint->setEnabled(false);
        }
    }
}
void MainWindow::WindowSpecifyingPoints()
{
    /*Начальная инициализация*/
//    AddNewPoint(0);
    CollbackButtonAddPoint();
    ui->pushButton_RemovePoint->setEnabled(false);

    connect(ui->pushButton_AddPoint, &QPushButton::clicked, this, &MainWindow::CollbackButtonAddPoint);

    connect(ui->pushButton_RemovePoint, &QPushButton::clicked, this, &MainWindow::CollbackButtonRemovePoint);
}
void MainWindow::AddNullPointToPosition(int numberPoint, int MaxPoint)
{
    int startPosition = MaxPoint;
    for(int j=0; j<(MaxPoint-numberPoint); j++)
    {
        for(int i=0; i<(startPosition-numberPoint)-1; i++)
        {
            isertRow(startPosition, startPosition-1);
            startPosition-=1;
        }
    }
}
void MainWindow::RemoveNullPointToPosition(int numberPoint, int MaxPoint)
{
    int startPosition = numberPoint;
    int l=0;
    while(l<MaxPoint)
    {
        isertRow(startPosition, startPosition+1);
        startPosition+=1;
        l++;
    }
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
    QString styleSheet =
//            "background-color: lightgreen;"
    "border: 1px dashed black";
//                            "font-weight: bold;";

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
    newSpinBoxTime->setMaximumSize(80,20);
    newSpinBoxTime->setSuffix(" мин");
    newSpinBoxTime->setRange(0.f, 1500.f);
    ui->gridLayout_manual->addWidget(newSpinBoxTime, Row, spinBoxTime_column);

    QSpinBox *newSpinBoxRUD = new QSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxRUD = newSpinBoxRUD_objName+QString("%1").arg(NumberPoint);
    newSpinBoxRUD->setObjectName(string_spinBoxRUD);
    newSpinBoxRUD->setMinimumSize(80,20);
    newSpinBoxRUD->setMaximumSize(80,20);
    newSpinBoxRUD->setRange(0, 100);
    newSpinBoxRUD->setSuffix(" %");
    ui->gridLayout_manual->addWidget(newSpinBoxRUD, Row, spinBoxRUD_column);

    QDoubleSpinBox *newSpinBoxPower = new QDoubleSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxPower = newSpinBoxPower_objName+QString("%1").arg(NumberPoint);
    newSpinBoxPower->setObjectName(string_spinBoxPower);
    newSpinBoxPower->setMinimumSize(80,20);
    newSpinBoxPower->setMaximumSize(80,20);
    newSpinBoxPower->setRange(0, 1000);
    newSpinBoxPower->setPrefix(">");
    newSpinBoxPower->setSuffix(" ЛС");
    ui->gridLayout_manual->addWidget(newSpinBoxPower, Row, spinBoxPower_column);

    QLineEdit *newLineEditNameMode = new QLineEdit();
    QString string_lineEditNameMode = newLineEditNameMode_objName+QString("%1").arg(NumberPoint);
    newLineEditNameMode->setObjectName(string_lineEditNameMode);
    newLineEditNameMode->setText(tr(""));
    newLineEditNameMode->setMinimumSize(60,20);
    newLineEditNameMode->setMaximumSize(60,20);
    ui->gridLayout_manual->addWidget(newLineEditNameMode, Row, lineEditNameMode_column);//добавляем виджет на компановку

    QLabel *newLabelStatus = new QLabel();
    QString string_labelStatus = newLabelStatus_objName+QString("%1").arg(NumberPoint);
    newLabelStatus->setObjectName(string_labelStatus);
    newLabelStatus->setText(tr("Не выполнен"));
    newLabelStatus->setMaximumSize(85,20);
    ui->gridLayout_manual->addWidget(newLabelStatus, Row, labelStatus_column);//добавляем виджет на компановку


    QString string_labelHandler = newLabelHandler_objName+QString("%1").arg(NumberPoint);
    MouseClickHandler *handler = new MouseClickHandler(newLabelNumberPoint);
    handler->setObjectName(string_labelHandler);


    connect(handler, &MouseClickHandler::doubleClicked,
            [=](Qt::MouseButton button, const QPoint &pos) {
        if(button == Qt::MouseButton::LeftButton)
        {
            setStyle(0, SelectedObject+1, 0);
            setStyle(1, NumberPoint+1, 0);
            SelectedObject = NumberPoint;
            setFocus(); // в конструкторе или при необходимости
            setFocusPolicy(Qt::StrongFocus); // установите политику фокуса

        }

    });
    connect(handler, &MouseClickHandler:: rightClicked,
                 [=](const QPoint &pos) {

                 setStyle(0, NumberPoint+1, 0);

                 SelectedObject=NoSelected;
//             }
         });
    connect(handler, &MouseClickHandler:: leftClicked,
                 [=](const QPoint &pos) {
        setFocus(); // в конструкторе или при необходимости
        setFocusPolicy(Qt::StrongFocus); // установите политику фокуса
         });
}
void MainWindow::setStyle(bool action, int row, int column)
{
    QString styleSheet;
    if(action)
    {
        styleSheet="background-color: lightblue;" "border: 1px dashed black"
                   ;
    }
    else
    {
        styleSheet="";
    }
    QLayoutItem* item = ui->gridLayout_manual->itemAtPosition(row, column);
    if(item!=nullptr)
    {
      item->widget()->setStyleSheet(styleSheet);
    }

}
void MainWindow::isertRow(int row_0 ,  int row_1)
{
    //функция меняет местами значения в виджетах (условное передвижение вверх-вниз)

    PointMode structure_0, structure_1;

    GetParametrFromForms(&structure_0, row_0);
    GetParametrFromForms(&structure_1, row_1);
    SendParametrFromForms(structure_1, row_0);
    SendParametrFromForms(structure_0, row_1);

}
void MainWindow::SendParametrFromForms(PointMode structure, uint16_t NumberPoint)
{
//    QString string_0 = newLabelNumberPoint_objName+QString("%1").arg(NumberPoint);
//    QLabel *Label_0 = findChild<QLabel *>(string_0);
//    Label_0->setText(structure->number);
//    //    delete RemoveLabel_0;

    QString string_1 = newSpinBoxTime_objName+QString("%1").arg(NumberPoint);
    QDoubleSpinBox *SpinBox_0 = findChild<QDoubleSpinBox *>(string_1);
    if(SpinBox_0!=nullptr)
        SpinBox_0->setValue(structure.Time);

    //    delete RemoveSpinBox_0;

    QString string_2 = newSpinBoxRUD_objName+QString("%1").arg(NumberPoint);
    QSpinBox *SpinBox_1 = findChild<QSpinBox *>(string_2);
    if(SpinBox_1!=nullptr)
        SpinBox_1->setValue(structure.RUD);
    //    delete RemoveSpinBox_1;

    QString string_3 = newSpinBoxPower_objName+QString("%1").arg(NumberPoint);
    QDoubleSpinBox *SpinBox_2 = findChild<QDoubleSpinBox *>(string_3);
    if(SpinBox_2!=nullptr)
        SpinBox_2->setValue(structure.PowerDVS);
    //    delete RemoveSpinBox_2;

    QString string_4 = newLineEditNameMode_objName+QString("%1").arg(NumberPoint);
    QLineEdit *LineEdit_0 = findChild<QLineEdit *>(string_4);
    if(LineEdit_0!=nullptr)
        LineEdit_0->setText(structure.NameMode);

    //    delete RemoveLineEdit_0;

    QString string_5 = newLabelStatus_objName+QString("%1").arg(NumberPoint);
    QLabel *Label_1 = findChild<QLabel *>(string_5);
    if(Label_1!=nullptr)
        Label_1->setText(structure.status);
    //    delete RemoveLabel_1;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
//    Qt::KeyboardModifiers modifiers = event->modifiers();

    switch (key) {
    case Qt::Key_Up:
    {
        if(SelectedObject>0)
        {
            isertRow(SelectedObject, SelectedObject-1);
            setStyle(0, SelectedObject+1, 0);
            SelectedObject-=1;
            setStyle(1, SelectedObject+1, 0);
        }
        event->accept();
    }break;
    case Qt::Key_Down:
    {
        if((SelectedObject<GetNumberEndPointMode()-1)&&SelectedObject>=0)
        {
           isertRow(SelectedObject, SelectedObject+1);
           setStyle(0, SelectedObject+1, 0);
           SelectedObject+=1;
           setStyle(1, SelectedObject+1, 0);
        }
        event->accept();
    }break;
    default:break;
//         QLabel::keyPressEvent(event);
    }
}
