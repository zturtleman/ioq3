# ioq3.js

ioq3.js is a simple port of [ioquake3](http://www.ioquake3.org) to the web using [Emscripten](http://github.com/kripken/emscripten). It was
heavily inspired by inolen's [quakejs](https://github.com/inolen/quakejs), but aims to be simpler and up-to-date with the latest Emscripten features.

## Demo

You can play the demo version [here](https://dl.dropboxusercontent.com/u/62064441/ioquake3.js/ioquake3.html).

## Building

```shell
make PLATFORM=js EMSCRIPTEN=/path/to/emscripten
```

Binaries will be placed in `build/release-js-js`.

## Adding data

Data is preloaded using Emscripten's [virtual filesystem](http://kripken.github.io/emscripten-site/docs/porting/files/packaging_files.html). So, if
you want to play, you have the provide the official (or demo) PK3's. Here's how it would work for retail Q3:

```shell
python /path/to/emscripten/tools/file_packager.py emscripten_data.data --preload baseq3 > emscripten_data.js
```

And add this to the `build/release-js-js/ioquake3.html` file:

```html
<script async type="text/javascript" src="emscripten_data.js"></script>
```

## Running

* Run `python -m SimpleHTTPServer 8888` in `build/release-js-js`
* Load localhost:8888/ioquake3.html in your browser

## Todo

* Networking
* Drop LEGACY_GL_EMULATION
