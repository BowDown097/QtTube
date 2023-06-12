**Please note that these instructions are far from perfect and will likely be improved upon at a later date. Additionally, these are specifically for Qt Creator users. Instructions can and most likely will differ on other IDEs.**

First and foremost, you obviously have to clone the repository.  
Just run ``git clone --recursive https://github.com/BowDown097/QtTube`` or use the download ZIP feature I guess if that floats your boat.  
VERY important that recursive flag is there, otherwise you won't be able to build this.

Then, get Qt 5 or 6 (6 recommended) and make sure you have the WebEngine module. Then you should be good to go!

## Using the experimental MPV-based player
So, the normal player this program uses is just an embed of YouTube's player using Qt's web engine with some cool modifications. Don't want to use that? You can try the experimental MPV-based player. You'll just need youtube-dl/yt-dlp and MPV.
### The Pros
- It's more lightweight
### The Cons
- I've completely neglected the thing. It's buggy, there's no player UI at all, and chances are it might not even build depending on when you try to use it. I'll neglect it less at some point.
- No SponsorBlock integration
