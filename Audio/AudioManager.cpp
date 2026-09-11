#include "AudioManager.h"
#include <QUrl>
#include <QDebug>

AudioManager* AudioManager::s_instance = nullptr;

AudioManager* AudioManager::instance() {
    if (!s_instance) {
        s_instance = new AudioManager();
    }
    return s_instance;
}

AudioManager::AudioManager(QObject* parent) : QObject(parent), m_effectIdCounter(0) {
    m_musicPlayer = new QMediaPlayer(this);
    m_musicOutput = new QAudioOutput(this);
    m_musicPlayer->setAudioOutput(m_musicOutput);
    m_musicOutput->setVolume(0.5); // 默认音量
}

AudioManager::~AudioManager() {
    // 停止并清理所有活跃的音效播放器
    for (auto it = m_effectPlayers.begin(); it != m_effectPlayers.end(); ++it) {
        it.value()->stop();
        delete it.value();
    }
    m_effectPlayers.clear();
    // 清理所有音效输出（与播放器配对）
    for (auto it = m_effectOutputs.begin(); it != m_effectOutputs.end(); ++it) {
        delete it.value();
    }
    m_effectOutputs.clear();
    // 音乐播放器由 Qt 父对象机制自动释放（this 为父对象）
}

void AudioManager::playBackgroundMusic(const QString& filePath, float volume) {
    if (m_musicPlayer->source() != QUrl::fromLocalFile(filePath)) {
        m_musicPlayer->setSource(QUrl::fromLocalFile(filePath));
    }
    m_musicOutput->setVolume(volume);
    m_musicPlayer->play();
}

void AudioManager::stopBackgroundMusic() {
    m_musicPlayer->stop();
}

void AudioManager::pauseBackgroundMusic() {
    if (m_musicPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_musicPlayer->pause();
    }
}

void AudioManager::resumeBackgroundMusic() {
    if (m_musicPlayer->playbackState() == QMediaPlayer::PausedState) {
        m_musicPlayer->play();
    }
}

void AudioManager::setBackgroundVolume(float volume) {
    m_musicOutput->setVolume(qBound(0.0f, volume, 1.0f));
}

void AudioManager::playSoundEffect(const QString& filePath, float volume) {
    int id = m_effectIdCounter++;

    QMediaPlayer* effect = new QMediaPlayer(this);
    QAudioOutput* output = new QAudioOutput(this);
    effect->setAudioOutput(output);
    output->setVolume(qBound(0.0f, volume, 1.0f));
    effect->setSource(QUrl::fromLocalFile(filePath));

    // 记录到映射表，析构时可统一清理
    m_effectPlayers.insert(id, effect);
    m_effectOutputs.insert(id, output);

    // 播放结束或出错时自动清理
    connect(effect, &QMediaPlayer::mediaStatusChanged, this,
            [this, id, effect, output](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia ||
            status == QMediaPlayer::InvalidMedia) {
            effect->stop();
            m_effectPlayers.remove(id);
            m_effectOutputs.remove(id);
            delete effect;
            delete output;
        }
    });

    // 兜底：播放器自身出错也清理
    connect(effect, &QMediaPlayer::errorOccurred, this,
            [this, id, effect, output](QMediaPlayer::Error /*error*/) {
        if (m_effectPlayers.contains(id)) {
            effect->stop();
            m_effectPlayers.remove(id);
            m_effectOutputs.remove(id);
            delete effect;
            delete output;
        }
    });

    effect->play();
}