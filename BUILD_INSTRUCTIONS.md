**Please note that these instructions are far from perfect and will likely be improved upon at a later date. Additionally, these are specifically for Qt Creator users. Instructions can and most likely will differ on other IDEs.**

First and foremost, you obviously have to clone the repository.  
Just run ``git clone --recursive https://github.com/BowDown097/QtTube`` or use the download ZIP feature I guess if that floats your boat.  
VERY important that recursive flag is there, otherwise you won't be able to build this.

## Prerequisites
### Linux
Make sure you have Qt 6 installed at all, then refer to [innertube-qt's build instructions](https://github.com/BowDown097/innertube-qt/blob/master/BUILD_INSTRUCTIONS.md).

### macOS
Highly recommend installing Qt through Homebrew if you haven't already. If you have, or don't want to install it through Homebrew and know what you're doing, just jump to the end of this section. Otherwise, two nice and easy steps:
- Install Qt by running ``brew install qt`` in the terminal.
- In Qt Creator, you will probably get an error saying no Qt version is available. You'll have to manually add the Qt version you installed as a kit. If you don't know how to do this, go to ``Kits > Qt versions`` in the settings and refer to [this StackOverflow answer](https://stackoverflow.com/a/55178810).

Finally, refer to [innertube-qt's build instructions](https://github.com/BowDown097/innertube-qt/blob/master/BUILD_INSTRUCTIONS.md).

### Windows
Refer to [innertube-qt's build instructions](https://github.com/BowDown097/innertube-qt/blob/master/BUILD_INSTRUCTIONS.md).

## Using the experimental MPV-based player
So, the normal player this program uses is just an embed of YouTube's player using Qt's web engine with some cool modifications. Don't want to use that? You can try the experimental MPV-based player. You'll just need youtube-dl/yt-dlp and MPV.
### The Pros
- It's more lightweight
- You can watch age-restricted videos and other blocked videos, i.e. most music videos (sadly)
### The Cons
- I've completely neglected the thing. It's buggy, there's no player UI at all, and chances are it might not even build depending on when you try to use it. I'll neglect it less at some point.
- No SponsorBlock integration
