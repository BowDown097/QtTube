function createAdblockInterceptor() {
    JSON.parseOG = JSON.parse;
    JSON.parse = function(obj) {
        obj = JSON.parseOG(obj);
        if (obj?.adPlacements)
            obj.adPlacements = [];
        if (obj?.playerAds)
            obj.playerAds = [];
        if (obj?.adSlots)
            obj.adSlots = [];
        return obj;
    };
}

function createAgeRestrictionInterceptor() {
    XMLHttpRequest.prototype.openOG = XMLHttpRequest.prototype.open;
    XMLHttpRequest.prototype.open = function(...args) {
        let [_, url] = args;
        if (typeof url === "string" && url.includes("/youtubei/v1/player")) {
            this.sendOG = this.send;
            this.send = function(...args) {
                if (typeof args[0] === "string") {
                    let obj = JSON.parse(args[0]);
                    if (obj.videoId) {
                        // without contentCheckOk and racyCheckOk set to true,
                        // the user will get an error along the lines of
                        // "This content is inappropriate for some users"
                        // and will not be able to continue beyond that point.
                        obj.contentCheckOk = true;
                        obj.racyCheckOk = true;
                        args[0] = JSON.stringify(obj);
                    }
                }
                this.sendOG.apply(this, args);
            };
        }
        XMLHttpRequest.prototype.openOG.apply(this, args);
    };
}
