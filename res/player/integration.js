// h264ify
const params = new URLSearchParams(document.location.search);
h264ify(params.get("h264Only") === "1", params.get("no60Fps") === "1");

// autoplay and time
waitForElement("#movie_player").then(function(p) {
    p.playVideo();
    p.seekTo(params.get("t"));
});

// apply settings
new QWebChannel(qt.webChannelTransport, async function(channel) {
    const settings = channel.objects.settings;

    if (settings.blockAds) {
        JSON.parseOG = JSON.parse;
        JSON.parse = function(obj) {
            obj = JSON.parseOG(obj);
            if (obj?.adPlacements)
                obj.adPlacements = [];
            if (obj?.playerAds)
                obj.playerAds = [];
            return obj;
        };
    }

    if (settings.sponsorBlockCategories?.length)
        await sponsorBlock(settings.sponsorBlockCategories);

    document.addEventListener("mousedown", function(e) {
        const coveringOverlay = e.target.closest(".ytp-ce-covering-overlay");
        const videowallStill = e.target.closest(".ytp-videowall-still");
        if (coveringOverlay != null) {
            const olParams = new URLSearchParams(coveringOverlay.search);
            channel.objects.interface.switchWatchViewVideo(olParams.get("v"));
        } else if (videowallStill != null) {
            const stillParams = new URLSearchParams(videowallStill.search);
            channel.objects.interface.switchWatchViewVideo(stillParams.get("v"));
        }
    });

    if (settings.disablePlayerInfoPanels)
        addStyle(".ytp-info-panel-preview { display: none; }");

    waitForElement("#movie_player").then(function(p) {
        // emit progress change when time text changes (every ~1 second)
        var previousProgress = 0;
        p.addEventListener("onVideoProgress", progress => {
            if (Math.abs(progress - previousProgress) < 1)
                return;
            channel.objects.interface.emitProgressChanged(progress, previousProgress);
            previousProgress = progress;
        });

        // communicate state changes to interface
        p.addEventListener("onStateChange", state => channel.objects.interface.handleStateChange(state));

        // set preferred volume when volume changes if we are setting it from player
        if (settings.volumeFromPlayer) {
            p.addEventListener("onVolumeChange", d => {
                if (!d.muted && d.volume != settings.preferredVolume)
                    settings.preferredVolume = d.volume;
            });
        }

        // set preferred volume
        p.setVolume(settings.preferredVolume);
        p.pauseVideo(); // pause video so the video doesn't go back to the beginning when quality pref is set. why does it do that???

        // annotations
        if (settings.restoreAnnotations) {
            waitForElement(".ytp-panel-menu").then(el => addAnnotationSwitch(el));
            handleAnnotations(document.location.pathname.split("/").pop());
        }

        // quality preference
        const qualityKeys = Object.keys(settings.PlayerQuality).reduce(function(acc, key) {
            return acc[settings.PlayerQuality[key]] = key, acc;
        }, {});

        var qPref = qualityKeys[settings.preferredQuality].toLowerCase();

        if (settings.qualityFromPlayer) {
            p.addEventListener("onPlaybackQualityChange", q => {
                if (q == qPref)
                    return;
                const match = Object.keys(settings.PlayerQuality).find(k => q == k.toLowerCase());
                if (match)
                    settings.preferredQuality = settings.PlayerQuality[match];
            });
        }

        if (!qPref || qPref == "auto") {
            p.playVideo();
            return;
        }

        const iv = setInterval(function() {
            if (!p.getAvailableQualityLevels || !p.setPlaybackQualityRange)
                return;

            const avail = p.getAvailableQualityLevels();
            if (!avail?.length)
                return;

            if (!avail.includes(qPref)) // if our pref is not available, pick (should be) next best one
                qPref = avail[0];

            p.setPlaybackQualityRange(qPref, qPref);
            p.playVideo();
            clearInterval(iv);
        }, 100);
    });
});
