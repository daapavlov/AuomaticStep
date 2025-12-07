#ifndef SETTINGSUSERTIME_H
#define SETTINGSUSERTIME_H

#include <QWidget>

namespace Ui {
class SettingsUserTime;
}

class SettingsUserTime : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsUserTime(QWidget *parent = nullptr);
    ~SettingsUserTime();

    int GetParametr(uint8_t numberParametr);
private:
    Ui::SettingsUserTime *ui;
};

#endif // SETTINGSUSERTIME_H
