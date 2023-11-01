const annotationsEndpoint = "https://sperg.rodeo/qttube_annotations";
const annotationParser = new AnnotationParser();
let renderer;

function addAnnotationSwitch(el) {
    const annoSwitchPar = document.createElement("div");
    annoSwitchPar.className = "ytp-menuitem";
    annoSwitchPar.innerHTML = `
    <div class="ytp-menuitem-icon"></div>
    <div class="ytp-menuitem-label">Classic annotations</div>
    <div class="ytp-menuitem-content">
        <div class="ytp-menuitem-toggle-checkbox">
        <input type="checkbox" id="annotation-sneaky-switch" aria-hidden="true" style="position: absolute; left: -100vw;">
        </div>
    </div>
    `;
    annoSwitchPar.setAttribute("role", "menuitemcheckbox");
    annoSwitchPar.setAttribute("aria-checked", "true");
    annoSwitchPar.setAttribute("tabindex", "0");

    el.prepend(annoSwitchPar);

    // a visually-hidden input checkbox (annoSneakySwitch) is used to store the state of annotation visibility.
	// the same thing could be done with some craftier JS but checkboxes are very certain and difficult to screw up */

	const annoSneakySwitch = document.querySelector("#annotation-sneaky-switch");
	annoSneakySwitch.checked = true;
	annoSwitchPar.addEventListener("click", () => {
		annoSneakySwitch.click();

		if (annoSneakySwitch.checked) {
			annoSwitchPar.setAttribute("aria-checked", "true");
			renderer.annotationsContainer.style.display = "block";
		}
		else {
			annoSwitchPar.setAttribute("aria-checked", "false");
			renderer.annotationsContainer.style.display = "none";
		}
	});
}

function fetchVideoAnnotations(videoId) {
    const requestUrl = getVideoPath(videoId);
    console.info(`Retrieving annotations for '${videoId}' from '${requestUrl}'`);

    return new Promise((resolve, reject) => {
        fetch(requestUrl)
        .then(response => {
            if (response.ok)
                response.text().then(resolve).catch(reject);
            else
                reject("Annotations unavailable");
        }).catch(reject);
    });
}

function getVideoPath(videoId) {
    let annotationFileDirectory = videoId[0];
    if (annotationFileDirectory === "-") {
        annotationFileDirectory = "-/ar-";
    }

    return `${annotationsEndpoint}?videoId=${videoId}`;
}

function handleAnnotations(videoId) {
    fetchVideoAnnotations(videoId).then(async (xml) => {
        console.info(`Received annotations for ${videoId} from server..`);
        if (xml)
            parseAnnotations(xml);
        else
            console.info(`Annotations received for ${videoId}, but video has no annotations`);
    }).catch(e => {
        console.info(`Annotation data is unavailable for this video (${videoId})\n (${e})`);
    });
}

function parseAnnotations(xml) {
    const annotationDom = annotationParser.xmlToDom(xml);
    const annotationElements = annotationDom.getElementsByTagName("annotation");

    const annotations = annotationParser.parseYoutubeAnnotationList(annotationElements);
    startNewAnnotationRenderer(annotations);
}

function startNewAnnotationRenderer(annotations) {
    const videoContainer = document.getElementById("movie_player");
	const player = document.querySelector("video.video-stream.html5-main-video");

	const playerOptions = {
		getVideoTime() {
			return player.currentTime;
		},
		seekTo(seconds) {
			player.currentTime = seconds;
		},
		getOriginalVideoWidth() {
			return player.videoWidth;
		},
		getOriginalVideoHeight() {
			return player.videoHeight;
		}
	};

    renderer = new AnnotationRenderer(annotations, videoContainer, playerOptions, 200);
    renderer.start();
}
