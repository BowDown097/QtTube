const barTypes = {
    "sponsor": { color: "#00d400", opacity: "0.7" },
    "selfpromo": { color: "#ffff00", opacity: "0.7" },
    "exclusive_access": { color: "#008a5c", opacity: "0.7" },
    "interaction": { color: "#cc00ff", opacity: "0.7" },
    "intro": { color: "#00ffff", opacity: "0.7" },
    "outro": { color: "#0202ed", opacity: "0.7" },
    "preview": { color: "#008fd6", opacity: "0.7" },
    "music_offtopic": { color: "#ff9900", opacity: "0.7" },
    "poi_highlight": { color: "#ff1684", opacity: "0.7" },
    "filler": { color: "#7300ff", opacity: "0.9" }
};

let segments = new Array();

async function sponsorBlock(categories) {
    const videoId = document.location.pathname.replace("/embed/", "");
    const sponsorResp = await fetch(`https://sponsor.ajay.app/api/skipSegments?videoID=${videoId}&categories=${JSON.stringify(categories)}`);
    if (!sponsorResp.ok)
        return;

    segments = await sponsorResp.json();
    await initialSBSetup();
    setInterval(checkSegments, 1000);
}

async function checkSegments() {
    const player = await waitForElement("#movie_player");
    const currentTime = player.getCurrentTime();
    for (const segment of segments) {
        if (segment.segment[0] <= currentTime && segment.segment[1] >= currentTime)
            player.seekTo(segment.segment[1]);
    }
}

async function initialSBSetup() {
    const player = await waitForElement("#movie_player");
    const videoDuration = player.getDuration();

    const ul = document.createElement("ul");
    ul.id = "previewbar";

    for (const segment of segments) {
        const bar = document.createElement("li");
        bar.classList.add("previewbar");
        bar.innerHTML = "&nbsp;";
        bar.setAttribute("sponsorblock-category", segment.category);

        bar.style.backgroundColor = barTypes[segment.category].color;
        bar.style.opacity = barTypes[segment.category].opacity;
        bar.style.position = "absolute";

        const duration = Math.min(segment.segment[1], videoDuration) - segment.segment[0];
        if (duration > 0) {
            const ttd0 = await timeToDecimal(segment.segment[0], videoDuration);
            const ttd1 = await timeToDecimal(segment.segment[1], videoDuration);
            bar.style.width = `calc(${(ttd1 - ttd0) * 100}%)`;
        }

        const time = segment.segment[1] ? Math.min(videoDuration, segment.segment[0]) : segment.segment[0];
        const timeDecimal = await timeToDecimal(time, videoDuration);
        bar.style.left = `${timeDecimal * 100}%`;

        ul.appendChild(bar);
    }

    const progressBar = await waitForElement(".ytp-progress-bar");
    progressBar.prepend(ul);
}

async function timeToDecimal(time, videoDuration) {
    const progressBar = await waitForElement(".ytp-progress-bar");
    const chapterBar = progressBar.querySelector('.ytp-chapters-container:not(.sponsorBlockChapterBar)');
    const chapterBarBlocks = chapterBar.querySelectorAll(":scope > div");
    const existingChapters = segments.sort((a, b) => a.segment[0] - b.segment[0]);
    if (chapterBarBlocks?.length > 1 && existingChapters.length === chapterBarBlocks?.length) {
        const totalPixels = chapterBar.parentElement.clientWidth;
        let pixelOffset = 0;
        let lastCheckedChapter = -1;
        for (let i = 0; i < chapterBarBlocks.length; i++) {
            const chapterElement = this.chapterBarBlocks[i];
            const mr = chapterElement.style.marginRight;
            const widthPixels = parseFloat(chapterElement.style.width.replace("px", ""));

            if (time >= existingChapters[i].segment[1]) {
                const marginPixels = mr ? parseFloat(mr.replace("px", "")) : 0;
                pixelOffset += widthPixels + marginPixels;
                lastCheckedChapter = i;
            } else {
                break;
            }
        }

        // The next chapter is the one we are currently inside of
        const latestChapter = existingChapters[lastCheckedChapter + 1];
        if (latestChapter) {
            const latestWidth = parseFloat(chapterBarBlocks[lastCheckedChapter + 1].style.width.replace("px", ""));
            const latestChapterDuration = latestChapter.segment[1] - latestChapter.segment[0];

            const percentageInCurrentChapter = (time - latestChapter.segment[0]) / latestChapterDuration;
            const sizeOfCurrentChapter = latestWidth / totalPixels;
            return Math.min(1, ((pixelOffset / totalPixels) + (percentageInCurrentChapter * sizeOfCurrentChapter)));
        }
    }

    return Math.min(1, time / videoDuration);
}
