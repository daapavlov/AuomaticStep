#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QMediaPlayer>
class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlayer(QObject *parent = nullptr);
    void PlayerMusic(QString path, int volume);
private:
    QMediaPlayer *player = new QMediaPlayer(this);
signals:

};

#endif // AUDIOPLAYER_H
