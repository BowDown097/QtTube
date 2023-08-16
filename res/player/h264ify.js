function h264ify(h264Only, no60Fps) {
    if (!h264Only && !no60Fps)
        return;

    // Override video element canPlayType() function
    var videoElem = document.createElement('video');
    var origCanPlayType = videoElem.canPlayType.bind(videoElem);
    videoElem.__proto__.canPlayType = makeModifiedTypeChecker(origCanPlayType, h264Only, no60Fps);

    // Override media source extension isTypeSupported() function
    var mse = window.MediaSource;
    // Check for MSE support before use
    if (mse === undefined)
        return;
    var origIsTypeSupported = mse.isTypeSupported.bind(mse);
    mse.isTypeSupported = makeModifiedTypeChecker(origIsTypeSupported, h264Only, no60Fps);
}

// return a custom MIME type checker that can defer to the original function
function makeModifiedTypeChecker(origChecker, h264Only, no60Fps) {
    // Check if a video type is allowed
    return function (type) {
        if (type === undefined)
            return '';

        if (h264Only) {
            const disallowedTypes = ['webm', 'vp8', 'vp08', 'vp9', 'vp09', 'av01'];
            if (disallowedTypes.some(disallowedType => type.includes(disallowedType)))
                return '';
        }

        if (no60Fps) {
            const match = /framerate=(\d+)/.exec(type);
            if (match && match[1] > 30)
                return '';
        }

        // Otherwise, ask the browser
        return origChecker(type);
    };
}
