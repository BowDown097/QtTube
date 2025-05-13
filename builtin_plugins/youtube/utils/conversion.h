#pragma once

namespace InnertubeObjects
{
struct CompactVideo;
struct DisplayAd;
struct LockupViewModel;
struct Reel;
struct ShortsLockupViewModel;
struct Video;
struct VideoDisplayButtonGroup;
}
namespace QtTube
{
class PluginException;
struct PluginMetadata;
struct PluginVideo;
}
class InnertubeException;
class YouTubeSettings;

QtTube::PluginException convertException(const InnertubeException& ex);
QtTube::PluginVideo convertVideo(
    const QtTube::PluginMetadata& metadata, YouTubeSettings* settings,
    const InnertubeObjects::CompactVideo& compactVideo, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(
    const QtTube::PluginMetadata& metadata, YouTubeSettings* settings,
    const InnertubeObjects::DisplayAd& displayAd, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(
    const QtTube::PluginMetadata& metadata, YouTubeSettings* settings,
    const InnertubeObjects::LockupViewModel& lockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(
    const QtTube::PluginMetadata& metadata, YouTubeSettings* settings,
    const InnertubeObjects::Reel& reel, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(
    const QtTube::PluginMetadata& metadata, YouTubeSettings* settings,
    const InnertubeObjects::ShortsLockupViewModel& shortsLockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(
    const QtTube::PluginMetadata& metadata, YouTubeSettings* settings,
    const InnertubeObjects::Video& video, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(
    const QtTube::PluginMetadata& metadata, YouTubeSettings* settings,
    const InnertubeObjects::VideoDisplayButtonGroup& video, bool useThumbnailFromData);
