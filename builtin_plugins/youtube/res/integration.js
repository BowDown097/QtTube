// h264ify
const params = new URLSearchParams(document.location.search);
if (params.get("h264Only") === "1")
    h264ify();

// autoplay and time
waitForElement("#movie_player").then(function(p) {
    p.playVideo();
    p.seekTo(params.get("t"));
});

// adblock
if (params.get("adblock") === "1")
    createAdblockInterceptor();

// apply settings
new QWebChannel(qt.webChannelTransport, async function(channel) {
    const playerSettings = channel.objects.playerSettings;
    const pluginSettings = channel.objects.pluginSettings;

    if (pluginSettings.sponsorBlockCategories?.length)
        await sponsorBlock(pluginSettings.sponsorBlockCategories);

    document.addEventListener("click", function(e) {
        const coveringOverlay = e.target.closest(".ytp-ce-covering-overlay");
        const videowallStill = e.target.closest(".ytp-videowall-still");
        if (coveringOverlay != null) {
            const olParams = new URLSearchParams(coveringOverlay.search);
            channel.objects.interface.requestSwitchVideo(olParams.get("v"));
        } else if (videowallStill != null) {
            const stillParams = new URLSearchParams(videowallStill.search);
            channel.objects.interface.requestSwitchVideo(stillParams.get("v"));
        }
    });

    if (pluginSettings.disablePlayerInfoPanels)
        addStyle(".ytp-info-panel-preview { display: none; }");

    waitForElement("#movie_player").then(function(p) {
        // emit progress change every second
        var previousProgress = 0;
        p.addEventListener("onVideoProgress", progress => {
            if (Math.abs(progress - previousProgress) < 1)
                return;
            channel.objects.interface.emitProgressChanged(progress, previousProgress);
            previousProgress = progress;
        });

        // communicate state changes to interface
        p.addEventListener("onStateChange", state => channel.objects.interface.emitNewState(state));

        // set preferred volume when volume changes if we are setting it from player
        if (playerSettings.volumeFromPlayer) {
            p.addEventListener("onVolumeChange", d => {
                if (!d.muted && d.volume != playerSettings.preferredVolume)
                    playerSettings.preferredVolume = d.volume;
            });
        }

        // set preferred volume
        p.setVolume(playerSettings.preferredVolume);
        p.pauseVideo(); // pause video so the video doesn't go back to the beginning when quality pref is set. why does it do that???

        // annotations
        if (pluginSettings.restoreAnnotations) {
            waitForElement(".ytp-panel-menu").then(el => addAnnotationSwitch(el));
            handleAnnotations(document.location.pathname.split("/").pop());
        }

        // quality preference
        const qualityKeyOverrides = {
            [playerSettings.Quality.HD4320]: "highres",
            [playerSettings.Quality.SD480]: "large",
            [playerSettings.Quality.SD360]: "medium",
            [playerSettings.Quality.SD240]: "small",
            [playerSettings.Quality.SD144]: "tiny"
        };

        var qPref = qualityKeyOverrides[playerSettings.preferredQuality] ||
            Object.keys(playerSettings.Quality)[playerSettings.preferredQuality].toLowerCase();

        if (playerSettings.qualityFromPlayer) {
            p.addEventListener("onPlaybackQualityChange", q => {
                if (q == qPref)
                    return;
                const match = Object.keys(playerSettings.Quality).find(k => q == k.toLowerCase());
                if (match)
                    playerSettings.preferredQuality = playerSettings.Quality[match];
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
