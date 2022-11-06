#ifndef WEBENGINEPLAYER_H
#define WEBENGINEPLAYER_H
#include "fullscreenwindow.h"
#include "playerinterceptor.h"
#include <QWebEngineFullScreenRequest>

class WebEnginePlayer : public QWidget
{
    Q_OBJECT
public:
    explicit WebEnginePlayer(QWidget* parent = nullptr);

    void setAuthStore(InnertubeAuthStore* authStore) { m_interceptor->setAuthStore(authStore); }
    void setContext(InnertubeContext* context) { m_interceptor->setContext(context); }
    void setPlayerResponse(const InnertubeEndpoints::PlayerResponse& resp) { m_interceptor->setPlayerResponse(resp); }
    void setUsePlaybackTracking(bool playbackTracking) { m_interceptor->setUsePlaybackTracking(playbackTracking); }
    void setUseWatchtimeTracking(bool watchtimeTracking) { m_interceptor->setUseWatchtimeTracking(watchtimeTracking); }
public slots:
    void play(const QString& vId, int progress, bool showSBToasts, const QVariantList& sponsorBlockCategories);
private slots:
    void fullScreenRequested(QWebEngineFullScreenRequest request);
private:
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;
    PlayerInterceptor* m_interceptor;
    QWebEngineView* m_view = nullptr;
    static inline const QString m_playerJs = R"(
function waitForElement(selector) {
    return new Promise(resolve => {
        const query = document.querySelector(selector);
        if (query) return resolve(query);
        const observer = new MutationObserver(mutations => {
            const query = document.querySelector(selector);
            if (query) {
                resolve(query);
                observer.disconnect();
            }
        });
        observer.observe(document, {
            childList: true,
            subtree: true
        });
    });
}

// SponsorBlock integration
(async function() {
    const barTypes = {
        "sponsor": {
            color: "#00d400",
            opacity: "0.7"
        },
        "selfpromo": {
            color: "#ffff00",
            opacity: "0.7"
        },
        "exclusive_access": {
            color: "#008a5c",
            opacity: "0.7"
        },
        "interaction": {
            color: "#cc00ff",
            opacity: "0.7"
        },
        "intro": {
            color: "#00ffff",
            opacity: "0.7"
        },
        "outro": {
            color: "#0202ed",
            opacity: "0.7"
        },
        "preview": {
            color: "#008fd6",
            opacity: "0.7"
        },
        "music_offtopic": {
            color: "#ff9900",
            opacity: "0.7"
        },
        "poi_highlight": {
            color: "#ff1684",
            opacity: "0.7"
        },
        "filler": {
            color: "#7300FF",
            opacity: "0.9"
        }
    };

    const params = new URLSearchParams(document.location.search);
    const sponsorBlockCategories = params.get("sbc");
    var sponsorBlockSegments = new Array();
    if (sponsorBlockCategories && JSON.parse(sponsorBlockCategories).length) {
        const videoId = document.location.pathname.replace("/embed/", "");
        const sponsorResp = await fetch(`https://sponsor.ajay.app/api/skipSegments?videoID=${videoId}&categories=${sponsorBlockCategories}`);
        sponsorBlockSegments = await sponsorResp.json();
        await initialSBSetup();
        setInterval(checkSegments, 1000);
    }

    async function checkSegments() {
        const player = await waitForElement("#movie_player");
        const currentTime = player.getCurrentTime();
        for (const segment of sponsorBlockSegments) {
            if (segment.segment[0] <= currentTime && segment.segment[1] >= currentTime)
                player.seekTo(segment.segment[1]);
        }
    }

    async function initialSBSetup() {
        const player = await waitForElement("#movie_player");
        const videoDuration = player.getDuration();

        const ul = document.createElement("ul");
        ul.id = "previewbar";

        for (const segment of sponsorBlockSegments) {
            const bar = document.createElement("li");
            bar.classList.add("previewbar");
            bar.innerHTML = "&nbsp;";
            bar.setAttribute("sponsorblock-category", segment.category);

            bar.style.backgroundColor = barTypes[segment.category].color;
            bar.style.opacity = barTypes[segment.category].opacity;
            bar.style.position = "absolute";

            const duration = Math.min(segment.segment[1], videoDuration) - segment.segment[0];
            if (duration > 0) {
                const ttd0 = await timeToDecimal(segment.segment[0], videoDuration);
                const ttd1 = await timeToDecimal(segment.segment[1], videoDuration);
                bar.style.width = `calc(${(ttd1 - ttd0) * 100}%)`;
            }

            const time = segment.segment[1] ? Math.min(videoDuration, segment.segment[0]) : segment.segment[0];
            bar.style.left = `${timeToDecimal(time) * 100}%`;

            ul.appendChild(bar);
        }

        const progressBar = await waitForElement(".ytp-progress-bar");
        progressBar.prepend(ul);
    }

    async function timeToDecimal(time, videoDuration) {
        const progressBar = await waitForElement(".ytp-progress-bar");
        const chapterBar = progressBar.querySelector('.ytp-chapters-container:not(.sponsorBlockChapterBar)');
        const chapterBarBlocks = chapterBar.querySelectorAll(":scope > div");
        const existingChapters = sponsorBlockSegments.sort((a, b) => a.segment[0] - b.segment[0]);
        if (chapterBarBlocks?.length > 1 && existingChapters.length === chapterBarBlocks?.length) {
            const totalPixels = chapterBar.parentElement.clientWidth;
            let pixelOffset = 0;
            let lastCheckedChapter = -1;
            for (let i = 0; i < chapterBarBlocks.length; i++) {
                const chapterElement = this.chapterBarBlocks[i];
                const mr = chapterElement.style.marginRight;
                const widthPixels = parseFloat(chapterElement.style.width.replace("px", ""));

                if (time >= existingChapters[i].segment[1]) {
                    const marginPixels = mr ? parseFloat(mr.replace("px", "")) : 0;
                    pixelOffset += widthPixels + marginPixels;
                    lastCheckedChapter = i;
                } else {
                    break;
                }
            }

            // The next chapter is the one we are currently inside of
            const latestChapter = existingChapters[lastCheckedChapter + 1];
            if (latestChapter) {
                const latestWidth = parseFloat(chapterBarBlocks[lastCheckedChapter + 1].style.width.replace("px", ""));
                const latestChapterDuration = latestChapter.segment[1] - latestChapter.segment[0];

                const percentageInCurrentChapter = (time - latestChapter.segment[0]) / latestChapterDuration;
                const sizeOfCurrentChapter = latestWidth / totalPixels;
                return Math.min(1, ((pixelOffset / totalPixels) + (percentageInCurrentChapter * sizeOfCurrentChapter)));
            }
        }

        return Math.min(1, time / videoDuration);
    }
})();

// Player ads patcher
(function() {
    var ytInitialPlayerResponse = null;

    function respGetter() { return ytInitialPlayerResponse; }
    function respSetter(data) { ytInitialPlayerResponse = { ...data, adPlacements: [] }; }

    Object.defineProperty(window, "ytInitialPlayerResponse", {
        get: respGetter,
        set: respSetter,
        configurable: true
    });

    const { fetch: origFetch } = window;
    window.fetch = async (...args) => {
        const response = await origFetch(...args);
        if (response.url.includes("/youtubei/v1/player")) {
            const text = () =>
                response
                    .clone()
                    .text()
                    .then((data) => data.replace(/adPlacements/, 'odPlacement'));

            response.text = text;
            return response;
        }

        return response;
    };
})();

// Player CSS patcher
(function() {
    function addStyle(css) {
        let style = document.createElement("style");
        style.type = "text/css";
        style.appendChild(document.createTextNode(css));
        waitForElement("head").then(head => head.appendChild(style));
    }

    addStyle(`
    /* 2019 menu scaling */
    .ytp-settings-menu {
        right: 12px !important;
        bottom: 49px !important;
    }

    .ytp-popup {
        border-radius: 2px !important;
    }

    .ytp-panel {
        min-width: 0 !important;
    }

    .ytp-panel-menu {
        padding: 6px 0 !important;
        width: 100% !important;
    }

    .ytp-panel-header {
        padding: 6px !important;
        height: 33px !important;
        font-size: 109% !important;
    }

    .ytp-menuitem {
        height: 33px !important;
    }

    .ytp-menuitem-label {
        font-size: 118% !important;
        font-weight: 500 !important;
        padding: 0 15px !important;
    }

    .ytp-menuitem-content {
        padding: 0 15px !important;
    }

    .ytp-menuitem-icon:not(:empty) {
        padding-left: 0 !important;
    }

    .ytp-menuitem[role="menuitemradio"] .ytp-menuitem-label {
        text-align: right !important;
    }

    .ytp-panel-header {
        height: 33px !important;
    }
    /* end 2019 menu scaling */

    /* menu icon disabling */
    .ytp-menuitem-icon {
        display: none !important;
    }
    /* end menu icon disabling */

    /* heatmap disabling */
    .ytp-heat-map-container, .ytp-tooltip-title {
        display: none !important;
    }
    /* end heatmap disabling */

    /* old embed head */
    .ytp-title-channel {
        display: none !important;
    }

    .ytp-title-text {
        padding-left: 4px !important;
        padding-top: 12px !important;
    }

    .ytp-overflow-button, .ytp-playlist-menu-button {
        padding-top: 6px !important;
        margin: 0 !important;
    }

    .ytp-overflow-button-text, .ytp-playlist-menu-button-text {
        display: none !important;
    }

    .ytp-watch-later-title, .ytp-share-title, .ytp-overflow-title, .ytp-copylink-title {
        display: none !important;
    }

    .ytp-watch-later-button, .ytp-share-button, .ytp-overflow-button, .ytp-copylink-button {
        min-width: 0 !important;
        margin: 0 !important;
    }
    /* end old embed head */

    /* 2019 chrome scaling */
    .ytp-chrome-bottom {
        height: 36px !important;
    }

    .ytp-chrome-controls {
        height: 36px !important;
        line-height: 36px !important;
    }

    .ytp-chrome-controls .ytp-button, .ytp-chrome-controls .ytp-replay-button {
        width: 36px !important;
        padding: 0 !important;
    }

    .ytp-progress-bar-container {
        bottom: 35px !important;
    }

    .ytp-time-display, .ytp-chapter-container {
        line-height: 35px !important;
    }
    /* end 2019 chrome scaling */

    /* general styles */
    .ytp-pause-overlay-container {
        display: none !important;
    }

    .ytp-menuitem-icon:empty {
        display: none !important;
    }

    .ytp-popup {
        outline: 0 !important;
    }

    .ytp-chrome-controls .ytp-button, .ytp-chrome-controls .ytp-replay-button {
        padding: 0 !important;
    }

    .ytp-chrome-controls .ytp-button.ytp-chapter-title, .ytp-chrome-controls .ytp-replay-button.ytp-chapter-title {
        width: 100% !important;
    }

    .ytp-chrome-controls .ytp-button.ytp-mute-button,
    .ytp-chrome-controls .ytp-replay-button.ytp-mute-button,
    .ytp-chrome-controls .ytp-button.ytp-next-button,
    .ytp-chrome-controls .ytp-replay-button.ytp-next-button,
    .ytp-chrome-controls .ytp-button.ytp-miniplayer-button,
    .ytp-chrome-controls .ytp-replay-button.ytp-miniplayer-button,
    .ytp-chrome-controls .ytp-button.ytp-remote-button,
    .ytp-chrome-controls .ytp-replay-button.ytp-remote-button {
        padding: 0 !important;
    }

    .ytp-chrome-controls .ytp-button[aria-pressed="true"]::after,
    .ytp-chrome-controls .ytp-replay-button[aria-pressed="true"]::after {
        width: 18px !important;
        left: 9px !important;
        bottom: 6px !important;
        height: 2px !important;
        border-radius: 2px !important;
    }

    .ytp-chrome-controls .ytp-youtube-button {
        display: none !important;
    }

    .ytp-chrome-controls .ytp-play-button {
        width: 46px !important;
    }

    .ytp-player-content {
        bottom: 53px !important;
    }

    .ytp-player-content:not(.ytp-upnext):not(.html5-endscreen) {
        top: 60px !important;
    }

    .ytp-gradient-bottom {
        height: 49px !important;
        padding-top: 49px !important;
    }

    .ytp-settings-menu {
        bottom: 49px !important;
        right: 12px !important;
    }

    .ytp-tooltip.ytp-bottom:not(.ytp-tooltip-opaque),
    .ytp-tooltip.ytp-preview.ytp-text-detail {
        top: unset !important;
        bottom: 50px !important;
    }

    .ytp-tooltip.ytp-preview:not(.ytp-text-detail) {
        transform: translateY(-29px) !important;
    }

    .ytp-autonav-toggle-button {
        transform: scale(0.9251237) !important;
        top: 11px !important;
    }

    .ytp-skip-intro-button {
        bottom: 50px !important;
        height: 40px !important;
    }

    .ytp-webgl-spherical-control {
        top: 60px !important;
    }

    .ytp-title-text {
        padding-top: 21px !important;
    }

    .html5-video-player:not(.ytp-autohide) .caption-window.ytp-caption-window-bottom {
        margin-bottom: 49px !important;
    }

    .html5-video-player:not(.ytp-autohide) .caption-window.ytp-caption-window-bottom svg {
        width: 40px !important;
        height: 40px !important;
    }

    .html5-video-player:not(.ytp-autohide) .caption-window.ytp-caption-window-bottom .ytp-icon {
        width: 40px !important;
        height: 40px !important;
    }

    .ytp-menuitem[aria-haspopup="true"] .ytp-menuitem-content {
        padding-right: 38px !important;
    }
    /* end general styles */

    /* SponsorBlock styles */
    .previewbar {
        display: inline-block;
        height: 100%;
    }

    #previewbar {
        overflow: visible;
        padding: 0;
        margin: 0;
        position: absolute;
        width: 100%;
        pointer-events: none;
        height: 100%;
        transform: scaleY(0.6) translateY(-30%) translateY(1.5px);
        z-index: 42;
        transition: transform .1s cubic-bezier(0,0,0.2,1);
    }
    /* end SponsorBlock styles */
    `);
})();
)";
};

#endif // WEBENGINEPLAYER_H
