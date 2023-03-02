document.addEventListener("mousedown", function(e) {
    const coveringOverlay = e.target.closest(".ytp-ce-covering-overlay");
    const videowallStill = e.target.closest(".ytp-videowall-still");
    if (coveringOverlay != null) {
        new QWebChannel(qt.webChannelTransport, function(channel) {
            const params = new URLSearchParams(coveringOverlay.search);
            channel.objects.interface.switchWatchViewVideo(params.get("v"));
        });
    } else if (videowallStill != null) {
        new QWebChannel(qt.webChannelTransport, function(channel) {
            const params = new URLSearchParams(videowallStill.search);
            channel.objects.interface.switchWatchViewVideo(params.get("v"));
        });
    }
});

const params = new URLSearchParams(document.location.search);
waitForElement("#movie_player").then(function(p) {
    p.seekTo(params.get("t")); // seek to saved time
    p.setVolume(params.get("v")); // seek to saved volume
    p.pauseVideo(); // pause video so the video doesn't go back to the beginning when quality pref is set. there's no way out of doing this. wtf???

    // quality preference
    var pref = params.get("q");
    if (!pref || pref == "auto") {
        p.playVideo();
        return;
    }

    var iv = setInterval(function() {
        if (!p.getAvailableQualityLevels || !p.setPlaybackQualityRange)
            return;

        const avail = p.getAvailableQualityLevels();
        if (!avail?.length)
            return;

        if (!avail.includes(pref)) // if our pref is not available, pick (should be) next best one
            pref = avail[0];

        p.setPlaybackQualityRange(pref, pref);
        p.playVideo();
        clearInterval(iv);
    }, 100);
});

