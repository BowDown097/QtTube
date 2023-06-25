// autoplay and time
waitForElement("#movie_player").then(function(p) {
    p.playVideo();
    p.seekTo(new URLSearchParams(document.location.search).get("t"));
});

// unavailable video patcher
waitForElement("#movie_player .ytp-error").then(function() {
    let embeddedPlayerResponse = JSON.parse(ytcfg.data_.PLAYER_VARS.embedded_player_response);
    if (embeddedPlayerResponse.previewPlayabilityStatus.status != "UNPLAYABLE")
        return;

    if (embeddedPlayerResponse.previewPlayabilityStatus.reason.includes("age-restricted")) {
        yt.config_.INNERTUBE_CLIENT_NAME = "TVHTML5_SIMPLY_EMBEDDED_PLAYER";
        yt.config_.INNERTUBE_CLIENT_VERSION = "2.0";
    } else {
        yt.config_.INNERTUBE_CLIENT_NAME = "WEB";
        yt.config_.INNERTUBE_CLIENT_VERSION = "2.20230607.06.00";
    }

    embeddedPlayerResponse.previewPlayabilityStatus = {
        contextParams: embeddedPlayerResponse.previewPlayabilityStatus.contextParams,
        playableInEmbed: true,
        status: "OK"
    };

    yt.config_.PLAYER_VARS.embedded_player_response = JSON.stringify(embeddedPlayerResponse);
    document.querySelector("#movie_player").loadVideoByPlayerVars(yt.config_.PLAYER_VARS);
});

new QWebChannel(qt.webChannelTransport, async function(channel) {
    let settings = channel.objects.settings;
    let qualityKeys = Object.keys(settings.PlayerQuality).reduce(function(acc, key) {
        return acc[settings.PlayerQuality[key]] = key, acc;
    }, {});

    if (settings.sponsorBlockCategories?.length) {
        await sponsorBlock(settings.sponsorBlockCategories);
    }

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

    h264ify(settings.h264Only, settings.disable60Fps);

    waitForElement("#movie_player").then(function(p) {
        p.setVolume(settings.preferredVolume); // set preferred volume
        p.pauseVideo(); // pause video so the video doesn't go back to the beginning when quality pref is set. there's no way out of doing this. wtf???

        // annotations
        if (settings.restoreAnnotations) {
            waitForElement(".ytp-panel-menu").then(el => addAnnotationSwitch(el));
            handleAnnotations(document.location.pathname.split("/").pop());
        }

        // quality preference
        var qPref = qualityKeys[settings.preferredQuality].toLowerCase();
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
