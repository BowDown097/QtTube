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
        clientName: "TVHTML5",
        clientVersion: "7.20240724.13.00",
    });

    if (isAgeGated)
        createAgeRestrictionInterceptor();

    const player = document.querySelector("#movie_player");
    const params = new URLSearchParams(document.location.search);
    player.loadVideoById(ytcfg.data_.PLAYER_VARS.video_id, params.get("t"));
});
