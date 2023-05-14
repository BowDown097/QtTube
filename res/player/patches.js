// CSS patcher - takes in QString arg (find better way to pass in data later maybe?)
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
