#include "httputils.h"
#include "cachedhttp.h"
#include "qttubeapplication.h"

namespace HttpUtils
{
    Http& cachedInstance()
    {
        if (qtTubeApp->settings().imageCaching)
        {
            static CachedHttp i;
            return i;
        }
        else
        {
            return Http::instance();
        }
    }
}
