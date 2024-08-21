const annotationsEndpoint = "https://storage.googleapis.com/biggest_bucket/annotations";
const annotationParser = new AnnotationParser();
let renderer;

function addAnnotationSwitch(el) {
    const annoSwitchPar = document.createElement("div");
    annoSwitchPar.className = "ytp-menuitem";
    annoSwitchPar.ariaChecked = true;
    annoSwitchPar.role = "menuitemcheckbox";
    annoSwitchPar.tabIndex = 0;
    el.prepend(annoSwitchPar);

    const icon = document.createElement("div");
    icon.className = "ytp-menuitem-icon";
    annoSwitchPar.appendChild(icon);

    const label = document.createElement("div");
    label.className = "ytp-menuitem-label";
    label.innerText = "Classic annotations";
    annoSwitchPar.appendChild(label);

    const content = document.createElement("div");
    content.className = "ytp-menuitem-content";
    annoSwitchPar.appendChild(content);

    const toggle = document.createElement("div");
    toggle.className = "ytp-menuitem-toggle-checkbox";
    content.appendChild(toggle);

    // a visually-hidden input checkbox (sneakySwitch) is used to store the state of annotation visibility.
	// the same thing could be done with some craftier JS but checkboxes are very certain and difficult to screw up

    const sneakySwitch = document.createElement("input");
    sneakySwitch.id = "annotation-sneaky-switch";
    sneakySwitch.type = "checkbox";
    sneakySwitch.ariaHidden = true;
    sneakySwitch.checked = true;
    sneakySwitch.style = "position: absolute; left: -100vw";
    toggle.appendChild(sneakySwitch);

	annoSwitchPar.addEventListener("click", () => {
		sneakySwitch.click();

		if (sneakySwitch.checked) {
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

    // base URL has no Access-Control-Allow-Origin header, so we need to run it through a CORS proxy
    const baseUrl = `${annotationsEndpoint}/${annotationFileDirectory}/${videoId.substring(0, 3)}/${videoId}.xml.gz`;
    return "https://corsproxy.io/?" + encodeURIComponent(baseUrl);
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
