function h264ify() {
    // Override video element canPlayType() function
    var videoElem = document.createElement('video');
    var origCanPlayType = videoElem.canPlayType.bind(videoElem);
    videoElem.__proto__.canPlayType = makeModifiedTypeChecker(origCanPlayType);

    // Override media source extension isTypeSupported() function
    var mse = window.MediaSource;
    // Check for MSE support before use
    if (mse === undefined) return;
    var origIsTypeSupported = mse.isTypeSupported.bind(mse);
    mse.isTypeSupported = makeModifiedTypeChecker(origIsTypeSupported);
}

// return a custom MIME type checker that can defer to the original function
function makeModifiedTypeChecker(origChecker) {
    // Check if a video type is allowed
    return function (type) {
        if (type === undefined) return '';
        var disallowed_types = ['webm', 'vp8', 'vp9', 'av01'];
        // If video type is in disallowed_types, say we don't support them
        for (var i = 0; i < disallowed_types.length; i++) {
            if (type.indexOf(disallowed_types[i]) !== -1) return '';
        }

        // Otherwise, ask the browser
        return origChecker(type);
    };
}
