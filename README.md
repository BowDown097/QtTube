![QtTube Logo](https://user-images.githubusercontent.com/42720004/192839093-ff0565f2-862f-4cee-a026-5cc0cadb20b5.png)

QtTube is a Qt frontend for YouTube made in C++ that aims to emulate the normal YouTube experience as close as possible. It's more private and it supports logging in, and with that, total integration with the YouTube website and app.

# Installation
The latest development versions can be installed on Arch Linux and its derivatives through the AUR:<br>
[![qttube-git](https://img.shields.io/badge/aur-qttube--git-blue)](https://aur.archlinux.org/packages/qttube-git/)

Outside of that, you will need to build the program yourself - [instructions are provided if needed](https://github.com/BowDown097/QtTube/blob/master/BUILD_INSTRUCTIONS.md). There are no releases yet and there is no ETA for when there will be one.

# Used Libraries
- [AnnotationsRestored](https://github.com/isaackd/AnnotationsRestored)
- [http](https://github.com/flaviotordini/http)
- [innertube-qt](https://github.com/BowDown097/innertube-qt)
- [media](https://github.com/flaviotordini/media)
- [QWebEngineYtPlayer](https://github.com/keshavbhatt/QWebengineYtPlayer)
- [Return YouTube Dislike](https://github.com/Anarios/return-youtube-dislike)
- [SponsorBlock](https://github.com/ajayyy/SponsorBlock)

# Screenshots
As of February 14th, 2023. Depending on how long it's been since said date, things may be different.
<div display="flex">
<img src="https://user-images.githubusercontent.com/42720004/218771944-0bbc6274-b035-49db-9164-25362063f830.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/218772018-54a62777-3b73-4691-b1f1-e86d32a57805.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/218772094-a689e1d8-ff9c-4013-b9ab-06f267e9cc52.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/218772224-d292ee5f-0c1a-4a7a-8beb-e5da35d213b3.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/218772152-bae43dd9-5950-4872-9f38-07c518d5d865.png" width="311" height="261" />
</div>

# Metrics
QtTube offers considerable improvements in performance and resource usage over the YouTube website, regardless of browser engine.

These graphs compare these metrics, browser vs. QtTube, on a ThinkPad T440p running Artix Linux. Mileage, of course, may vary depending on device, but your experience should be quite similar.

![Page Load Time (seconds)](https://user-images.githubusercontent.com/42720004/228050796-60fc6aad-b962-4936-9019-6ec44e468378.png)
![Resident Memory Usage (MB)](https://user-images.githubusercontent.com/42720004/228050816-e3879449-a383-4022-bdc8-4c28adbd87de.png)

### Notes
- On the website, the time for a "warm load" or "smooth load" specifically was measured to be more fair.
- On the website, load time measuring started from the first sign of page load, which I determined from whichever of these happened first:
  - The URL changing
  - The appearance of the loading bar on top of the web page
  - The page skeleton showing
- On QtTube, load time measuring started whenever I clicked on the thing that would initiate the load (i.e. a tab on the main view).
- Load time measuring ended when everything visible finished rendering. For the watch view on QtTube specifically, I stopped when the pause button showed on the player (which happened after all the info had already rendered).
- Memory usage was recorded from the RES column on Htop, on a fresh load (where possible), after any immediate garbage collection.
- The History page is loaded and rendered all at once on QtTube, whereas it is not on the website - hence the longer loading time on QtTube. Regardless, this is how it is to be measured under my criteria, since most of a "full load" on the website is not visible.
