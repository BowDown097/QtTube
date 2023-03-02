(function() {
    // CSS patcher - takes in QString arg (find better way to pass in data later maybe?)
    function addStyle(css) {
        let style = document.createElement("style");
        style.type = "text/css";
        style.appendChild(document.createTextNode(css));
        document.documentElement.appendChild(style);
    }
    addStyle(`%1`);

    // ads remover
    JSON.parseOG = JSON.parse;
    JSON.parse = function(obj) {
        obj = JSON.parseOG(obj);
        if (obj?.adPlacements) {
            obj.adPlacements = [];
        } else if (obj?.playerAds) {
            obj.playerAds = [];
        }
        return obj;
    };
})();
