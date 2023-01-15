![QtTube Logo](https://user-images.githubusercontent.com/42720004/192839093-ff0565f2-862f-4cee-a026-5cc0cadb20b5.png)
<p align="center">
  <img src="https://img.shields.io/codefactor/grade/github/BowDown097/QtTube" />
  <a href="https://hitsofcode.com/github/BowDown097/QtTube/view">
    <img src="https://hitsofcode.com/github/BowDown097/QtTube" />
  </a>
</p>

QtTube is a Qt frontend for YouTube, made in C++. It aims to provide all of the features of the YouTube website, many of which other frontends do not include, while maintaining as much privacy as possible.

# Features
- **FULLY FUNCTIONAL** YouTube login.
- **TOTAL INTEGRATION** with YouTube. This means, assuming you haven't turned the features off in the Settings and you are logged in, videos you watch will be saved in your History and tweak your recommendations across not only this application but also the YouTube website and app.
- Home page and watch page.

# Roadmap
This list is ordered by priority. Everything will likely be added in this order.
- Description, recommendations, etc. on watch page
- Account menu
- Channel pages
- Switch video player from an embed of YouTube's player to one based off of libmpv
- Subscriptions/history export

# Contributing
This project is in desperate need of contributors should it not take an eternity to finish. No matter how small, pull requests are **GREATLY** appreciated. This is, at the moment, a one-man effort. And there will be no codes of conduct here - I don't care who you are: if your code is good and works, that's all that matters!

# Used Libraries
This program makes use of:
- [http](https://github.com/flaviotordini/http) - Makes QtNetwork much less of a headache to use
- [innertube-qt](https://github.com/BowDown097/innertube-qt) - The backend for the program
- [media](https://github.com/flaviotordini/media) - For the libmpv-based player
- [QWebEngineYtPlayer](https://github.com/keshavbhatt/QWebengineYtPlayer) - For the YouTube-embed-based player. Pretty heavily modified but still, credit where credit's due.

# Screenshots
As of January 15th, 2023. Depending on how long it's been since said date, things may look different and/or have more content.
<div display="flex">
<img src="https://user-images.githubusercontent.com/42720004/212549629-483b4b1e-3f13-41e5-a977-9547da459662.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/212549633-e2068701-fdcf-41ad-a5b7-1708cb653bd3.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/212549635-64c33a5f-43a1-429e-a1bd-b4c336cf3b9a.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/212549642-d21ff391-b7f2-41e8-8c84-32f1c248a09e.png" width="348" height="261" />
<img src="https://user-images.githubusercontent.com/42720004/212549638-f6e06ad6-4b8b-4734-bf03-b50eaf76054d.png" width="261" height="261" />
</div>
