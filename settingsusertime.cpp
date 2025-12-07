#include "settingsusertime.h"
#include "ui_settingsusertime.h"

SettingsUserTime::SettingsUserTime(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsUserTime)
{
    ui->setupUi(this);
}

SettingsUserTime::~SettingsUserTime()
{
    delete ui;
}
int SettingsUserTime::GetParametr(uint8_t numberParametr)
{
    switch (numberParametr)
    {
        case 0: return ui->spinBox_0->value(); break;
        case 1: return ui->spinBox_1->value(); break;
        case 2: return ui->spinBox_2->value(); break;
    }
}
