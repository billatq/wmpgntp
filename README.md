# wmpgntp

WMP-GNTP was written by me while I was an undergrad at Texas A&M. I was surprised that nobody had written a plugin for Windows Media Player using Growl for Windows, so I decided to write one.

It's actually not that hard to get the song title, but getting album art was tricker than I'd thought. I haven't used Windows Media Player in a long time, so this is essentially unmaintained at this point.

## Developer notes

### Minimizing Dependencies
I really wanted this to be buildable as a standalone library with minimal dependencies and under a license that would be easy for folks at Microsoft to consume. (I'd just finished a few internships there, and I thought gntp would be more popular than it is.)

To that end, it has its own code for parsing MP4 album art (it's not provided by WMP) and relies on the Microsoft CryptoAPI for doing MD5 hashes.

### Getting MP3 Album Art
For the MP3 album art, Windows Media Player does do the parsing for you, but it doesn't provide any API to get it. And yet, it seems strange that this data is surfaced in other places.

Let's look at the API: https://msdn.microsoft.com/en-us/library/windows/desktop/dd563451(v=vs.85).aspx

There is a curious get_URL method that is for "internal use". If you call that method anyway, you get a url that looks like this: "vnd.ms.wmhtml://localhost/WMP{guid}.jpg". This is curious, and it turns out that it's storing the album art that it can parse into Internet Explorer's cache. Seems like a really lazy way to not have to reparse every time.

I parse that out over here: https://github.com/billatq/wmpgntp/blob/4bff935929b39040a59559916bec2559b2d1125a/WMP-GNTP/WMP-GNTP/WMP-GNTPevents.cpp#L91

### Getting MP4 Album Art
For MP4 album art, the same trick doesn't happen. Fortunately, it's not actually that hard to actually write a parser to pull out the album art for an MP4 file: https://github.com/billatq/wmpgntp/blob/4bff935929b39040a59559916bec2559b2d1125a/WMP-GNTP/WMP-GNTP/Util.cpp#L367

