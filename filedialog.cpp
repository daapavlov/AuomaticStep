#include "filedialog.h"


fileDialog::fileDialog(QObject *parent) : QObject(parent)
{

}
QString fileDialog::OpenFile()
{
    QString path;
//    path = QFileDialog::getOpenFileName(NULL, "Открыть","C:","*.RPD;;All files(*.*)");
    path = QFileDialog::getSaveFileName(NULL, "Открыть","C:","*.RPD;;All files(*.*)");
    return path;
}
bool fileDialog::WriteFileSettings(QString directory, QMap<QString, QString> map)
{
    QString pathFile;
    if(directory.isEmpty())
    {
        directory = "C:";
    }
    pathFile = QFileDialog::getSaveFileName(NULL, "Сохранить",directory,"*.RPD;;All files(*.*)");

    QFile file(pathFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return 0;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");

    if(map.isEmpty())
    {
        file.close();
        return 0;
    }
    for(auto it=map.begin(); it!=map.end();++it)
    {
        out<<it.key()<<"="<<it.value();
    }
    file.close();
    return 1;
}
bool fileDialog::ReadFileSettings(QString directory, QMap<QString, QString> &map)
{
    QString pathFile;
    if(directory.isEmpty())
    {
        directory = "C:";
    }
    pathFile = QFileDialog::getOpenFileName(NULL, "Открыть",directory,"*.RPD;;All files(*.*)");
    QFile file(pathFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return 0;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");

    while(!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            continue; // Пропускаем пустые строки
        }
        QStringList parts = line.split("=");
        if (parts.size() >= 2) {
            QString key = parts[0].trimmed();
            QString value = parts[1].trimmed();
            map[key] = value;
        }
    }
    file.close();
    return 1;
}
