#include "audioplayer.h"

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent)
{

}
void AudioPlayer::PlayerMusic(QString path,  int volume)
{
    player->setMedia(QUrl::fromLocalFile(path));
    player->setVolume(volume);
    player->play();
}
