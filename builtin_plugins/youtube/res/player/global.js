function addStyle(css) {
    let style = document.createElement("style");
    style.type = "text/css";
    style.appendChild(document.createTextNode(css));
    document.documentElement.appendChild(style);
}

function waitForElement(selector) {
    return new Promise(resolve => {
        const query = document.querySelector(selector);
        if (query)
            return resolve(query);

        const observer = new MutationObserver(() => {
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
