#ifndef SETTINGSMODBUS_H
#define SETTINGSMODBUS_H

#include <QWidget>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>

#include "settingsdialog.h"

class SettingsDialog;//класс настройки com-порта
class QModbusClient;
class QModbusReply;

namespace Ui {
class SettingsModbus;
}

class SettingsModbus : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsModbus(QWidget *parent = nullptr);
    ~SettingsModbus();

    bool GetSettingsDevice_modbusRTU(uint8_t Device, uint16_t *Addr, uint16_t *AddrFirstReg, uint16_t *QuantityReg);
    bool SendData_ModbusRTU(uint16_t ServerEdit, uint16_t StartAddr, uint16_t *Data, uint16_t DataSize);
    bool AcceptData_ModbusRTU(uint16_t ServerEdit, uint16_t StartAddr, uint16_t counterAddr);
    QVector <uint16_t> GetModbusData_Receive();

    struct Device//структура устройства, которое будет в сети модбас
    {
        QString name;
        uint16_t address;
        uint16_t addres_first_registrs;
        uint16_t quantity_registers;
    };

    QString ErrorMessage_receive="";
    QString ErrorMessage_transmission="";

private:
    Ui::SettingsModbus *ui;

    uint16_t NumberDevice_ui=0;
    QVector <uint16_t> Buffer_Modbus_Receive;

    QString LabelNumber = "labelNumber_";
    QString LineEditName="lineEditName_";
    QString SpinBoxID="spinBoxID_";
    QString SpinBoxAddr="spinBoxAddr_";
    QString SpinBoxQuantity="spinBoxQuantity_";

    QModbusReply *lastRequest = nullptr;
    QModbusClient *modbusDevice = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;

    QMessageBox *messageModbus = new QMessageBox();

    void ConnectFunction();
    void onConnectTypeChanged();
    void onModbusStateChanged(int state);
    bool onReadReady(uint16_t AddressSlave);
    QModbusDataUnit writeRequest() const;

signals:
    void SettingAreSet();
    void dataReceived();
private slots:
    void AddNewWidgetDevice(uint8_t NumberDevice);
    void RemoveWidgetDevice(uint8_t NumberDevice);
    void SetSettinsModBus();

};

#endif // SETTINGSMODBUS_H
