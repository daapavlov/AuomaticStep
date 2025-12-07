#include "settingsmodbus.h"
#include "ui_settingsmodbus.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDateTime>

SettingsModbus::SettingsModbus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsModbus)
{
    ui->setupUi(this);
    m_settingsDialog = new SettingsDialog(this);
    ConnectFunction();
    ui->pushButton_addDevice->click();
    ui->pushButton_addDevice->click();
    ui->pushButton_addDevice->click();
    ui->pushButton_addDevice->click();
    ui->pushButton_addDevice->click();
    ui->tabWidget_comport->removeTab(1);
    ui->tabWidget_comport->addTab(m_settingsDialog, "Настройки COM порта");

}

SettingsModbus::~SettingsModbus()
{
    delete ui;
}
bool SettingsModbus::SendData_ModbusRTU(uint16_t ServerEdit, uint16_t StartAddr, uint16_t *Data, uint16_t DataSize)
{
    /*функция отправки данных в слейв
    * ServerEdit - адрес слейва
    * StartAddr - начальный регистр
    * Data - массив с данными
    * Количество регистров, которые заполняются равно размеру вектора Data
    */
    bool StReturn = false;
    QDateTime newTime = QDateTime::currentDateTime();
    QString ErrorWriteMessage = "Ошибка при передаче: ";
    if (!modbusDevice)
        return false;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, StartAddr, DataSize);
    writeUnit.setValueCount(DataSize);
    for (int i = 0, total = int(writeUnit.valueCount()); i < total; ++i)
    {
            writeUnit.setValue(i, Data[i]);
    }

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ServerEdit)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply, ErrorWriteMessage, newTime, ServerEdit, &StReturn]() {
                if (reply) {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        ErrorMessage_transmission = newTime.toString("hh:mm:ss ")+ErrorWriteMessage+QString(tr("%1 (Mobus exception: 0x%2)")
                                                                             .arg(reply->errorString()).arg(reply->rawResult().exceptionCode()));
                    } else if (reply->error() != QModbusDevice::NoError) {
                        ErrorMessage_transmission = newTime.toString("hh:mm:ss ")+ErrorWriteMessage+QString(tr("Write response error: %1 (code: 0x%2). Address: %3").
                            arg(reply->errorString()).arg(reply->error()).arg(ServerEdit));
                    }
                    else
                    {
                        ErrorMessage_transmission = "no error";
                        emit dataTransmitted();
                        StReturn = true;
                    }
                }
                else
                {
                    ErrorMessage_transmission = newTime.toString("hh:mm:ss ")+ErrorWriteMessage+"Invalid reply";
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        ErrorMessage_transmission = newTime.toString("hh:mm:ss ")+ErrorWriteMessage+modbusDevice->errorString();
    }
    return StReturn;
}
bool SettingsModbus::AcceptData_ModbusRTU(uint16_t ServerEdit, uint16_t StartAddr, uint16_t counterAddr)
{
    /*функция получения данных в слейв
    * ServerEdit - адрес слейва
    * StartAddr - начальный регистр
    * counterAddr - колмчество регистров для чтения
    */
    QDateTime newTime = QDateTime::currentDateTime();
    QString ErrorReadMessage = "Ошибка при чтении: ";

    if (!modbusDevice)
    {
        ErrorMessage_receive = newTime.toString("hh:mm:ss ")+ErrorReadMessage+"Не создан объект";
        return false;
    }

    QModbusDataUnit readRequest(QModbusDataUnit::HoldingRegisters, StartAddr, counterAddr);
    auto *reply = modbusDevice->sendReadRequest(readRequest, ServerEdit);

    if (reply)
    {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, this, [this, ServerEdit]
            {
                onReadReady((uint16_t)ServerEdit);
                ErrorMessage_receive = "";
            });
        }
        else
            delete reply; // broadcast replies return immediately
    } else {
            ErrorMessage_receive = newTime.toString("hh:mm:ss ")+ErrorReadMessage+modbusDevice->errorString();
    }

    return true;
}
bool SettingsModbus::onReadReady(uint16_t AddressSlave)
{
    /*функция обработки полученных данных от слейва
    * полученные данные хранятся в Buffer_Modbus_Receive
    * после сигнала dataReceived можно вызвать метод GetModbusData_Receive для получения актуальных данных
    */
    QDateTime newTime = QDateTime::currentDateTime();
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
    {
        ErrorMessage_receive = "No reply!";
        return false;
    }

    if (reply->error() == QModbusDevice::NoError)
    {
        const QModbusDataUnit unit = reply->result();
        Buffer_Modbus_Receive.clear();
        for (int i = 0, total = int(unit.valueCount()); i < total; ++i)
        {
           Buffer_Modbus_Receive.append(static_cast<uint16_t>(unit.value(i)));
        }
        emit dataReceived();
    }
    else if (reply->error() == QModbusDevice::ProtocolError)
    {
        ErrorMessage_receive = tr("Read response error: %1 (Mobus exception: 0x%2)").
              arg(reply->errorString()).
              arg(reply->rawResult().exceptionCode(), -1, 16);
    }
    else
    {
        ErrorMessage_receive = tr("Read response error: %1 (code: 0x%2). Address: %3").
                arg(reply->errorString()).
                arg(reply->error(), -1, 16).
                arg(AddressSlave);
        const QModbusDataUnit unit = reply->result();
    }

    reply->deleteLater();

    return true;
}
QVector <uint16_t> SettingsModbus::GetModbusData_Receive()
{
    /*Функция передачи данных в главный класс*/
    QVector <uint16_t> vector = {444};//проверка на пустоту
    if(!Buffer_Modbus_Receive.isEmpty())
    {
        return Buffer_Modbus_Receive;
    }

    return vector;

}
void SettingsModbus::SetSettinsModBus()
{
    /*Функция обработки нажатия кнопки "сохранить". устанавливает настройки ком порта*/
    if(!modbusDevice)
    {
        QMessageBox::critical(this, "Ошибка!", "Настройки не установлены");
        Flags_setSettings_comPort = false;
        return;
    }
    if ((modbusDevice->state() != QModbusDevice::ConnectedState) && (m_settingsDialog->settings().namePort != "Custom"))
    {
        modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
            m_settingsDialog->settings().namePort);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
            m_settingsDialog->settings().parity);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
            m_settingsDialog->settings().baud);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
            m_settingsDialog->settings().dataBits);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
            m_settingsDialog->settings().stopBits);
        modbusDevice->setTimeout(m_settingsDialog->settings().timeout);
        modbusDevice->setNumberOfRetries(0);

        if(modbusDevice->connectDevice())
        {
            QMessageBox::information(this, "Успешно!", "Настройки установлены");
            m_settingsDialog->SetNameButton("Изменить", 0);
            Flags_setSettings_comPort = true;
            emit SettingAreSet();//Сигнал для закрытия настроек
        }
        else
        {
            QMessageBox::critical(this, "Ошибка!", "Отказано в досупе");
            Flags_setSettings_comPort = false;
        }
    }
    else
    {
        modbusDevice->disconnectDevice();
        Flags_setSettings_comPort = false;
        m_settingsDialog->SetNameButton("Сохранить", 1);
    }
}

void SettingsModbus::ConnectFunction()
{

    connect(m_settingsDialog, &SettingsDialog::ClickedSaveButton, this, &SettingsModbus::SetSettinsModBus);
    onConnectTypeChanged();
    connect(this, &SettingsModbus::SettingAreSet, [this]()
    {
        this->close();
    });

    connect(ui->pushButton_addDevice, &QPushButton::clicked, [this]()
    {
        if(NumberDevice_ui>=0 && NumberDevice_ui<255)
        {
            AddNewWidgetDevice(NumberDevice_ui);
            NumberDevice_ui++;
        }

    });

    connect(ui->pushButton_removeDevice, &QPushButton::clicked, [this]()
    {
       RemoveWidgetDevice(NumberDevice_ui-1);
       NumberDevice_ui--;
       if(NumberDevice_ui<0)
       {
            NumberDevice_ui=0;
       }

    });
}
void SettingsModbus::onConnectTypeChanged()
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }
    modbusDevice = new QModbusRtuSerialMaster(this);

    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        QDateTime newTime = QDateTime::currentDateTime();
        ErrorMessage_receive = QString("%1%2").arg(newTime.toString("hh:mm:ss: ")).arg(modbusDevice->errorString());
    });

    if (!modbusDevice) {
        QDateTime newTime = QDateTime::currentDateTime();
        ErrorMessage_receive = QString("%1Could not create Modbus master.").arg(newTime.toString("hh:mm:ss: ")).arg(modbusDevice->errorString());

    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &SettingsModbus::onModbusStateChanged);
    }
}
void SettingsModbus::onModbusStateChanged(int state)
{
//    bool connected = (state != QModbusDevice::UnconnectedState);
//    ui->actionConnect->setEnabled(!connected);
//    ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
    {

    }
//        ui->connectButton->setText(tr("Подключить"));
    else if (state == QModbusDevice::ConnectedState)
    {

    }
//        ui->connectButton->setText(tr("Отключить"));
}
bool SettingsModbus::GetSettingsDevice_modbusRTU(uint8_t Device, uint16_t *Addr, uint16_t *AddrFirstReg, uint16_t *QuantityReg)
{
    QString nameDevice;
    QString string_LineEditName = QString(LineEditName+"%1").arg(Device);
    QLineEdit *LineEditName = findChild<QLineEdit *>(string_LineEditName);
    if(LineEditName==nullptr)
    {
       return 0;
    }
    nameDevice = QString(LineEditName->text());

    QString string_SpinBoxID = QString(SpinBoxID+"%1").arg(Device);
    QSpinBox *SpinBoxID = findChild<QSpinBox *>(string_SpinBoxID);
    *Addr = SpinBoxID->value();

    QString string_SpinBoxAddr = QString(SpinBoxAddr+"%1").arg(Device);
    QSpinBox *SpinBoxAddr = findChild<QSpinBox *>(string_SpinBoxAddr);
    *AddrFirstReg = SpinBoxAddr->value();

    QString string_SpinBoxQuantity = QString(SpinBoxQuantity+"%1").arg(Device);
    QSpinBox *SpinBoxQuantity = findChild<QSpinBox *>(string_SpinBoxQuantity);
    *QuantityReg = SpinBoxQuantity->value();

    return true;
}
void SettingsModbus::AddNewWidgetDevice(uint8_t NumberDevice)
{
    /*Функция добавления атрибутов для устройств модбас*/
    uint8_t labelnumber_column=0, lineEditName_column=1, spinBoxID_column=2, SpinBoxAddr_column=3, spinBoxQuantity_column=4;
    uint8_t Row = NumberDevice+1;

    QLabel *newLabelNumber = new QLabel();
    QString string_labelNumber = QString(LabelNumber+"%1").arg(NumberDevice);
    newLabelNumber->setObjectName(string_labelNumber);
    newLabelNumber->setText(tr("%1").arg(NumberDevice));
    newLabelNumber->setMaximumSize(30,13);
    ui->gridLayout_2->addWidget(newLabelNumber, Row, labelnumber_column);//добавляем виджет на компановку

    QLineEdit *newLineEditName = new QLineEdit();
    QString string_lineEditName = QString(LineEditName+"%1").arg(NumberDevice);
    newLineEditName->setObjectName(string_lineEditName);
    switch (NumberDevice) {
    case 0: newLineEditName->setText(tr("Пульт РУД"));break;
    case 1: newLineEditName->setText(tr("Насос подкачки"));break;
    case 2: newLineEditName->setText(tr("Воздухоотделитель"));break;
    case 3: newLineEditName->setText(tr("Помпа высоковольтная"));break;
    case 4: newLineEditName->setText(tr("Помпа низковольтная"));break;
    default: newLineEditName->setText(tr("Устройство %1").arg(NumberDevice));break;
    }

    newLineEditName->setMinimumSize(240,20);
    ui->gridLayout_2->addWidget(newLineEditName, Row, lineEditName_column);//добавляем виджет на компановку

    QSpinBox *newSpinBoxID = new QSpinBox();//создаем объект указывающий на адрес слейва
    QString string_spinBoxID = QString(SpinBoxID+"%1").arg(NumberDevice);
    newSpinBoxID->setObjectName(string_spinBoxID);
    newSpinBoxID->setMinimumSize(68,20);
    newSpinBoxID->setMinimum(1);
    newSpinBoxID->setMaximum(255);
    ui->gridLayout_2->addWidget(newSpinBoxID, Row, spinBoxID_column);

    QSpinBox *newSpinBoxAddr = new QSpinBox();//создаем объект указывающий на адрес первого регистра слейва
    QString string_spinBoxAddr = QString(SpinBoxAddr+"%1").arg(NumberDevice);
    newSpinBoxAddr->setObjectName(string_spinBoxAddr);

    newSpinBoxAddr->setMinimumSize(68,20);
    ui->gridLayout_2->addWidget(newSpinBoxAddr, Row, SpinBoxAddr_column);

    QSpinBox *newSpinBoxQuantity = new QSpinBox();//создаем объект указывающий на количество регистров слейва
    QString string_spinBoxQuantity = QString(SpinBoxQuantity+"%1").arg(NumberDevice);
    newSpinBoxQuantity->setObjectName(string_spinBoxQuantity);
    newSpinBoxQuantity->setMinimum(1);
    newSpinBoxQuantity->setMaximum(10);
    newSpinBoxQuantity->setMinimumSize(68,20);
    ui->gridLayout_2->addWidget(newSpinBoxQuantity, Row, spinBoxQuantity_column);


    ui->gridLayout_2->removeItem(ui->verticalSpacer);//удаляем текщкю позоцию пружины
    ui->gridLayout_2->addItem(ui->verticalSpacer, Row+1, lineEditName_column);//ставим пружину на позицию ниже виджета
    /*QString holdingReg_str = QString("holdingReg_%1").arg(i);
    QLabel *label_hold = findChild<QLabel *>(holdingReg_str);*/
}
void SettingsModbus::RemoveWidgetDevice(uint8_t NumberDevice)
{
    /*функция удаления атрибутов устройств модбас*/
    QString string_RemoveLabelNumber = QString(LabelNumber+"%1").arg(NumberDevice);
    QLabel *RemoveLabelNumber = findChild<QLabel *>(string_RemoveLabelNumber);
    if(RemoveLabelNumber!=nullptr)
    {
        delete RemoveLabelNumber;
    }

    QString string_RemoveLineEditName = QString(LineEditName+"%1").arg(NumberDevice);
    QLineEdit *RemoveLineEditName = findChild<QLineEdit *>(string_RemoveLineEditName);
    if(RemoveLineEditName!=nullptr)
    {
        delete RemoveLineEditName;
    }

    QString string_SpinBoxID = QString(SpinBoxID+"%1").arg(NumberDevice);
    QSpinBox *RemoveSpinBoxID = findChild<QSpinBox *>(string_SpinBoxID);
    if(RemoveSpinBoxID!=nullptr)
    {
        delete RemoveSpinBoxID;
    }

    QString string_SpinBoxAddr = QString(SpinBoxAddr+"%1").arg(NumberDevice);
    QSpinBox *RemoveSpinBoxAddr = findChild<QSpinBox *>(string_SpinBoxAddr);

    if(RemoveSpinBoxAddr!=nullptr)
    {
        delete RemoveSpinBoxAddr;
    }

    QString string_SpinBoxQuantity = QString(SpinBoxQuantity+"%1").arg(NumberDevice);
    QSpinBox *RemoveSpinBoxQuantity = findChild<QSpinBox *>(string_SpinBoxQuantity);
    if(RemoveSpinBoxQuantity!=nullptr)
    {
        delete RemoveSpinBoxQuantity;
    }
}
