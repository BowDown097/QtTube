#ifndef WATCHVIEWSHARED_H
#define WATCHVIEWSHARED_H
#include "clickablelabel.h"
#include "httpreply.h"
#include "innertube/objects/video/videosecondaryinfo.h"
#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class WatchViewShared
{
#ifdef Q_OS_MACOS
    inline static IOPMAssertionID sleepAssert;
#endif
public:
    static QSize calcPlayerSize(int widgetWidth, int windowHeight);
    static void setChannelIcon(const HttpReply& reply, ClickableLabel* channelIcon);
    static void setSubscriberCount(const InnertubeObjects::VideoSecondaryInfo& secondaryInfo, QLabel* subscribersLabel);
    static void toggleIdleSleep(bool toggle);
};

#endif // WATCHVIEWSHARED_H
