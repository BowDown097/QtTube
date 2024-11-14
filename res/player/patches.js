// CSS patcher - takes in QString arg (find better way to pass in data later maybe?)
addStyle(`%1`);

// unavailable video patcher (a LOT of this courtesy of yt-dlp <3)
waitForElement("#movie_player .ytp-error").then(function() {
    const embeddedPlayerResponse = JSON.parse(ytcfg.data_.PLAYER_VARS.embedded_player_response);
    const playability = embeddedPlayerResponse.previewPlayabilityStatus;

    const ageGateReasons = [ "confirm your age", "age-restricted", "inappropriate" ];
    const ageGateStatuses = [ "AGE_VERIFICATION_REQUIRED", "AGE_CHECK_REQUIRED" ];
    const isAgeGated = Object.hasOwn(playability, "desktopLegacyAgeGateReason") ||
        ageGateReasons.some(r => playability.reason?.includes(r)) ||
        ageGateStatuses.some(s => playability.status == s);

    // this bypasses blocks on music videos and stuff like that. also probably helpful with login
    Object.assign(ytcfg.data_.INNERTUBE_CONTEXT.client, {
        clientName: "IOS",
        clientVersion: "19.34.2",
        deviceMake: "Apple",
        deviceModel: "iPhone16,2",
        userAgent: "com.google.ios.youtube/19.34.2 (iPhone16,2; U; CPU iOS 17_6 like Mac OS X;)",
        osName: "iPhone",
        osVersion: "17.6.1.21G93"
    });

    if (isAgeGated)
        createAgeRestrictionInterceptor();

    document.querySelector("#movie_player").loadVideoById(ytcfg.data_.PLAYER_VARS.video_id);
});
