#pragma once
#include <QObject>

namespace QtTubePlugin
{
    // members are exposed as Q_PROPERTYs should you wish to access them from QWebChannel
    class PlayerSettings : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool h264Only MEMBER h264Only NOTIFY h264OnlyChanged)
        Q_PROPERTY(QtTubePlugin::PlayerSettings::Quality preferredQuality MEMBER preferredQuality NOTIFY preferredQualityChanged)
        Q_PROPERTY(int preferredVolume MEMBER preferredVolume NOTIFY preferredVolumeChanged)
        Q_PROPERTY(bool qualityFromPlayer MEMBER qualityFromPlayer NOTIFY qualityFromPlayerChanged)
        Q_PROPERTY(bool vaapi MEMBER vaapi NOTIFY vaapiChanged)
        Q_PROPERTY(bool volumeFromPlayer MEMBER volumeFromPlayer NOTIFY volumeFromPlayerChanged)
    public:
        enum class Quality { Auto, HD4320, HD2160, HD1440, HD1080, HD720, SD480, SD360, SD240, SD144 };
        Q_ENUM(Quality)

        bool h264Only{};
        Quality preferredQuality{};
        int preferredVolume{};
        bool qualityFromPlayer{};
        bool vaapi{};
        bool volumeFromPlayer{};
    signals:
        void h264OnlyChanged(bool);
        void preferredQualityChanged(QtTubePlugin::PlayerSettings::Quality);
        void preferredVolumeChanged(int);
        void qualityFromPlayerChanged(bool);
        void vaapiChanged(bool);
        void volumeFromPlayerChanged(bool);
    };
}
