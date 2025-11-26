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
    ConnectFunction();
    ui->tabWidget_comport->removeTab(1);
    ui->tabWidget_comport->addTab(m_settingsDialog, "Настройки COM порта");




}

SettingsModbus::~SettingsModbus()
{
    delete ui;
}
bool SettingsModbus::SendData_ModbusRTU()
{
    return false;
}
bool SettingsModbus::AcceptData_ModbusRTU(uint16_t ServerEdit, uint16_t StartAddr, uint16_t counterAddr, uint16_t *ReceivedData, QString *Message_error)
{
    QDateTime newTime = QDateTime::currentDateTime();
    QString ErrorReadMessage = "Ошибка при чтении:";
    QModbusDataUnit readRequest =  QModbusDataUnit(QModbusDataUnit::HoldingRegisters, StartAddr, counterAddr);
    if (!modbusDevice)
        return false;

    if (auto *reply = modbusDevice->sendReadRequest(readRequest, ServerEdit)) {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, this, [&]
            {
                *Message_error = newTime.toString("hh:mm:ss ")+ErrorReadMessage+onReadReady((uint16_t)ServerEdit, ReceivedData);
                emit dataReceived();
            });
        }
        else
            delete reply; // broadcast replies return immediately
    } else {
        *Message_error = newTime.toString("hh:mm:ss ")+ErrorReadMessage+modbusDevice->errorString();
    }
    return true;
}
QString SettingsModbus::onReadReady(uint16_t AddressSlave, uint16_t *data)
{
    QString ErrorMessage;
    QDateTime newTime = QDateTime::currentDateTime();
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
    {
        ErrorMessage = "No reply!";
        return ErrorMessage;
    }

    if (reply->error() == QModbusDevice::NoError)
    {
        const QModbusDataUnit unit = reply->result();
        for (int i = 0, total = int(unit.valueCount()); i < total; ++i)
        {
            data[i] = unit.value(i);
        }
    }
    else if (reply->error() == QModbusDevice::ProtocolError)
    {
        ErrorMessage = tr("Read response error: %1 (Mobus exception: 0x%2)").
              arg(reply->errorString()).
              arg(reply->rawResult().exceptionCode(), -1, 16);
    }
    else
    {

        ErrorMessage = tr("Read response error: %1 (code: 0x%2). Address: %3").
                arg(reply->errorString()).
                arg(reply->error(), -1, 16).
                arg(AddressSlave);
        const QModbusDataUnit unit = reply->result();

    }

    reply->deleteLater();

    return ErrorMessage;
}
void SettingsModbus::SetSettinsModBus()
{
    if(!modbusDevice)
    {
        QMessageBox::critical(this, "Ошибка", "Настройки не установлены");
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
            emit SettingAreSet();//Сигнал для закрытия настроек
        }
    }
    else
    {
        modbusDevice->disconnectDevice();
        QMessageBox::critical(this, "Ошибка", "Настройки не установлены");
    }
}

void SettingsModbus::ConnectFunction()
{

    connect(m_settingsDialog, &SettingsDialog::ClickedSaveButton, this, &SettingsModbus::SetSettinsModBus);
    connect(this, &SettingsModbus::SettingAreSet, [this]()
    {
        this->close();
    });
    connect(ui->pushButton_addDevice, &QPushButton::clicked, [this]()
    {
        if(i>=0 && i<255)
        {
            AddNewWidgetDevice(i);
            i++;
        }

    });

    connect(ui->pushButton_removeDevice, &QPushButton::clicked, [this]()
    {
       RemoveWidgetDevice(i-1);
       i--;
       if(i<0)
       {
            i=0;
       }

    });
}
QString SettingsModbus::GetSettingsDevice_modbusRTU(uint8_t Device, uint16_t *Addr, uint16_t *AddrFirstReg, uint16_t *QuantityReg)
{
    QString nameDevice;
    QString string_LineEditName = QString(LineEditName+"%1").arg(Device);
    QLineEdit *LineEditName = findChild<QLineEdit *>(string_LineEditName);
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

    return nameDevice;
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
    newLineEditName->setText(tr("Устройство %1").arg(NumberDevice));
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
