#pragma once
#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMap>

class AudioManager : public QObject {
    Q_OBJECT
public:
    static AudioManager* instance();
    void playBackgroundMusic(const QString& filePath, float volume = 0.5f);
    void stopBackgroundMusic();
    void pauseBackgroundMusic();
    void resumeBackgroundMusic();
    void setBackgroundVolume(float volume);
    void playSoundEffect(const QString& filePath, float volume = 0.8f);

private:
    explicit AudioManager(QObject* parent = nullptr);
    ~AudioManager();
    static AudioManager* s_instance;
    QMediaPlayer* m_musicPlayer;
    QAudioOutput* m_musicOutput;
    QMap<int, QMediaPlayer*> m_effectPlayers; // id → player，追踪所有活跃音效
    QMap<int, QAudioOutput*> m_effectOutputs; // id → output，与 player 配对
    int m_effectIdCounter;                    // 自增ID
};