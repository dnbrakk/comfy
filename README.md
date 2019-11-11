# Comfy

**Version:** 1.0.2

[https://wolfish.neocities.org/soft/comfy/](https://wolfish.neocities.org/soft/comfy)

**Dev chat on Matrix:** #comfy_dev:matrix.org

Comfy is a console application for browsing imageboards. Comfy is written in C++, is multithreaded, supports displaying images, and is very /comfy/ to use.

Right now Comfy only parses 4chan json, but support for other imageboards will be added in the future.

Comfy has its own widget system for drawing to the terminal built on top of Termbox. Images are loaded as pixmaps using Imlib2 and drawn to the terminal emulator window in the X Window System. Text-only mode can be used to browse without X (such as in a TTY).

Currently, Comfy has only been tested on Debian 9 (Stretch). Please let me know what other systems you are able to get it to compile and run on.

![Comfy browsing a thread](https://files.catbox.moe/91kw91.gif) &nbsp;&nbsp;&nbsp; ![Comfy browsing a thread](https://files.catbox.moe/zwzp7y.gif)  

### Get, Compile, and Run

##### On Debian:

```
apt-get install git make pkg-config g++ libx11-dev libimlib2-dev libcurl4-openssl-dev
git clone https://gitgud.io/wolfish/comfy
cd comfy
make
./comfy
```

##### On OpenSUSE (Tumbleweed -- should also work on Leap)

```
zypper in -y make gcc gcc-c++ libX11-devel imlib2-devel libcurl-devel
git clone https://gitgud.io/wolfish/comfy
cd comfy
make
./comfy
```

An option to compile without image support and X as a dependency will be added later.

##### Supported Terminal Emulators

So far Comfy with image support has been tested and works with:

- xterm
- mlterm

*[Please help expand this list.]*

Comfy should run in many other terminal emulators, but images sometimes will not display. Images get disabled automatically when an X error is received. Running Comfy with images enabled in a TTY tends to cause the program to crash (run with '-d' to disable images).

At present, a mouse is required to browse as there is currently no way to select and open threads in a board catalog without clicking; keyboard controls for selecting and opening threads will be added soon.

![Comfy browsing a thread](https://files.catbox.moe/fkgy50.gif) &nbsp;&nbsp;&nbsp; ![Comfy browsing a thread](https://files.catbox.moe/md2oea.gif)  

### Using

Comfy can open URLs that are fed to it via the command-line. Currently only 4chan board catalogs and threads are supported. If no URLs are provided, the homescreen is displayed, from which you can navigate to the officially suported imageboards or open saved threads.

Running Comfy with '-d' or '--disable-images' will activate text-only mode and images won't be downloaded or displayed. Running Comfy outside of X with images enabled usually causes it to crash (working on fixing this), so be sure to run with images disabled if you do.

You can set the max number of concurrent threads Comfy is allowed to use with '-m n' or '--max-threads n' where 'n' is the maximum number of threads. By default, Comfy sets the maximum number of threads to the total number of CPU cores available on the system - 1 (e.g. if your CPU has 4 cores, Comfy will set the max threads to 3). The default setting seems to work well enough, but feel free to experiment with this. Be aware that if you set this number too high your system will lock up when Comfy is downloading images or doing other work.

Imageboard threads can be saved by pressing CTRL+S while viewing a thread. Doing so prevents the thread's json and images from being deleted from Comfy's cache directory (located in $HOME/.comfy/) when the program terminates, and the thread will be listed in the saved threads list accessible from the homescreen.

Mouse input is supported: Pages can be scrolled using the mouse wheel, threads can be opened by left-clicking them, images in threads can be full screened/closed by left-clicking on them, and posts can be jumped to in a thread by clicking post num links.

Comfy has a built-in color scheme system, but right now there is only one hardcoded color scheme. Color scheme switching will be implemented, as well as loading color schemes from files on disk. Please feel free to come up with new color schemes and submit them for inclusion (you can play with editing the default color scheme, or adding new ones, by editing colors.h).

#### Controls

- **Backspace / CTRL+H** Go to homescreen, or go back one level in list.
- **CTRL+Q** Quit.
- **CTRL+X** Close catalog or thread.
- **CTRL+R** Reload the focused page from the network.
- **CTRL+A** Enable/disable auto-refresh of focused page.
- **CTRL+S** Save currently focused thread.
- **F5** Do a hard refresh of the screen (e.g. to clear out artifacts).
- **Tab** Switch between currently opened pages.
- **Space / Enter** Choose selection in list.
- **Up / Down Arrow Keys** Scroll up/down one line at a time.
- **Left / Right Arrow Keys** Scroll up/down one screen at a time.
- **Page Up / Down** Scroll up/down one screen at a time.
- **Home** Go to top of page.
- **End** Go to bottom of page.
- **Left-Click** Open a thread in a catalog, full screen/close an image in a thread, or go to a post in a thread (by clicking the post number link in a post).
- **Mouse Wheel** Scroll the currently focused page.

### Planned Features

- Save images to arbitrary dir (e.g. by right-clicking the image).
- Reduce thread box size in catalog when images are disabled.
- Navigate catalog and open threads with keyboard only.
- Use $XDG_CONFIG_HOME/comfy, $XDG_CACHE_HOME/comfy, and $XDG_DATA_HOME/comfy for storing config, cache, and static non-executable files, respectively, with $HOME used as a fallback.
- ASCII (or similar) image display in text-only mode.
- Color scheme chooser; load color schemes from disk.
- 16 color color schemes for TTYs and terminal emulators that don't support 256 colors.
- Copy URLs to clipboard on click.
- Right-click post number link to open pop-up preview of post.
- Option to disable title animations.
- Better key input system (Termbox's is rather limited, though very portable).
- Key binding settings, plus ability to set them from a config file.
- Vim keybindings.
- Animate GIFs.
- Options screen/config file for setting various defaults (such as thread refresh interval).
- Option to tick background widgets (so that e.g. threads can continue to auto refresh).
- Option to show full screen images at actual resolution and allowing scrolling in the x and y axes (e.g. for viewing large infographs or screencaps).
- Page loading indicator.
- Display error popup when page fails to load.
- Mark deleted threads as 404.
- Posting from Comfy (might not be possible with 4chan due to the goolag captcha, but should be possible with other imageboards).
- Video support, including youtube embedding? (Need to do some experimentation with ffmpeg)
- Support for parsing Lynxchan.
- Support for parsing other imageboards (feel free to email requests).
- Support for parsing 4plebs archives.
- Code syntax highlighting in code blocks.
- Help screen.
- Text search function for searching within focused thread or catalog.
- Display images without requiring X (using the framebuffer directly)?
- Option to securely shred temporary files (e.g. images) on exit.
- Option to encrypt temporary files and saved threads.
- Windows and OS X ports.

### Known Problems

- Images do not display correctly when running Comfy inside of tmux (artifacts do not get removed when scrolling, and when run in a split pane the images are in the wrong location). Term cell colors also do not display correctly.
- Application will crash if images are enabled and Comfy is run by another user than the one that is logged into the X Session the terminal emulator is running in (e.g. if you switch users using 'su user' then run comfy).
- Intense image flickering for a few seconds after resizing terminal window using the mouse (e.g. by dragging an edge or corner of the window). Really not sure what causes this.
- Brief image flickering in some terminal emulators when scrolling catalog pages when the column width of threads displayed is greater than two.
- Intermittent image flickering in st (stterm). This might be caused by footer redrawing in the tick event of catalog and thread widgets.
- Images don't display in some terminal emulators running in X.
- Crashes (eventually) when running in TTY with images enabled.
- Scroll stuttering after scrolling the mouse wheel very quickly.
- TextWidget string sanitization is inefficient and doesn't currently strip all HTML or convert most HTML entities to the proper chars.
- When tofu (characters unsupported by terminal's font) gets displayed, it sometimes breaks the layout and/or leaves artifacts behind when scrolling (do a hard refresh by pressing F5 to get rid of the artifacts).
- Image artifact remove cell sometimes appears at bottom right of switch widgets box.
- Thread scroll position is sometimes changed when a new post is loaded after a reload.
- Memory usage is relatively high when many threads with many images each are open at once. More efficient image caching solutions will be explored as the program develops.

### Contributing

- Tabs for indentation, spaces for alignment.

Will write a small style guide later.

### License

Comfy is licensed under the GNU General Public License v2.0 only.

### Supporting Development

If you think Comfy is rather cool and would like to support its development, please consider [leaving a donation](https://wolfish.neocities.org/posts/updates/donate/). :^) Thank you.

