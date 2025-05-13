#pragma once
#include "innertube/innertubeexception.h"
#include "innertube/objects/ad/displayad.h"
#include "innertube/objects/video/compactvideo.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "innertube/objects/video/videodisplaybuttongroup.h"
#include "innertube/objects/viewmodels/lockupviewmodel.h"
#include "innertube/objects/viewmodels/shortslockupviewmodel.h"
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/objects/video.h"

QtTube::PluginException convertException(const InnertubeException& ex);
QtTube::PluginVideo convertVideo(const InnertubeObjects::CompactVideo& compactVideo, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::DisplayAd& displayAd, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::LockupViewModel& lockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::Reel& reel, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::ShortsLockupViewModel& shortsLockup, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::Video& video, bool useThumbnailFromData);
QtTube::PluginVideo convertVideo(const InnertubeObjects::VideoDisplayButtonGroup& video, bool useThumbnailFromData);
