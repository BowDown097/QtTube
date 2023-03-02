function waitForElement(selector) {
    return new Promise(resolve => {
        const query = document.querySelector(selector);
        if (query) return resolve(query);
        const observer = new MutationObserver(mutations => {
            const query = document.querySelector(selector);
            if (query) {
                resolve(query);
                observer.disconnect();
            }
        });
        observer.observe(document, {
            childList: true,
            subtree: true
        });
    });
}
