// CSS patcher - takes in QString arg (find better way to pass in data later maybe?)
addStyle(`%1`);

// unavailable video patcher
var sendOG = XMLHttpRequest.prototype.send;
XMLHttpRequest.prototype.send = function(data) {
    if (typeof data === "string" && data.includes("playbackContext")) {
        let playerData = JSON.parse(data);
        playerData.context.client.clientName = yt.config_.INNERTUBE_CLIENT_NAME;
        playerData.context.client.clientVersion = yt.config_.INNERTUBE_CLIENT_VERSION;
        data = JSON.stringify(playerData);
    }
    sendOG.call(this, data);
};
