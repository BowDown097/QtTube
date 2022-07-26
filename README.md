![QtTube Logo](https://user-images.githubusercontent.com/42720004/192839093-ff0565f2-862f-4cee-a026-5cc0cadb20b5.png)

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
