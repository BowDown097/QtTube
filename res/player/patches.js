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

    let clientData = {};
    if (isAgeGated) {
        clientData = {
            clientName: "TVHTML5_SIMPLY_EMBEDDED_PLAYER",
            clientVersion: "2.0"
        };
    } else { // this will (hopefully) resolve any other potential problem
        clientData = {
            clientName: "IOS",
            clientVersion: "19.29.1",
            deviceMake: "Apple",
            deviceModel: "iPhone16,2",
            userAgent: "com.google.ios.youtube/19.29.1 (iPhone16,2; U; CPU iOS 17_5_1 like Mac OS X;)",
            osName: "iPhone",
            osVersion: "17.5.1.21F90"
        };
    }

    Object.assign(ytcfg.data_.INNERTUBE_CONTEXT.client, clientData);
    document.querySelector("#movie_player").loadVideoById(ytcfg.data_.PLAYER_VARS.video_id);
});
