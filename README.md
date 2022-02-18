# gst-element-examples

Trying to creste the most minimal src, transform and sink elements for [gstreamer](https://github.com/GStreamer/gstreamer.git). 

Many of the existing examples are skewed towards AV applications and the [plugin writers guide](https://gstreamer.freedesktop.org/documentation/plugin-development/index.html?gi-language=c) has little to cover sources and sinks. 

The examples here take a basic memory buffer(src) with 1 byte initialised. This can be passed through to transform (just clears the top nibble of the byte) and outputs to the sink which prints it out (via debug) 

The following assumes gst version 1.20.0


Initial template code was generated through the gst-element-maker scripts

./gstreamer/subprojects/gst-plugins-bad/tools/gst-element-maker mysrc basesrc
./gstreamer/subprojects/gst-plugins-bad/tools/gst-element-maker mytransform basetransform
./gstreamer/subprojects/gst-plugins-bad/tools/gst-element-maker mysink basesink



## Building

The 3 plugins are built by copying the source directly into the existing plugin framework. 
TODO move this to a system where this repo can be build standalone against an existing gstreamer-dev install


### Install Common Dependencies

sudo apt install flex bison ninja-build libcairo2-dev cmake libgtk-3-dev libbz2-dev libv4l-dev libjpeg-dev libgmp-dev openssl libssl-dev gnutls-dev nasm

### Get GStreamer source
```
git clone https://github.com/GStreamer/gstreamer.git
cd gstreamer
git checkout tags/1.20.0
```

### Build Gstreamer Source
```
cd gstreamer
meson.py build
ninja -C build
ninja -C build uninstalled
```


And test the build...

```
[gst-HEAD] phill@ubuntu:~/src/gstreamer/build$ ./subprojects/gstreamer/tools/gst-launch-1.0 --version
gst-launch-1.0 version 1.20.0
GStreamer 1.20.0
Unknown package origin
[gst-HEAD] phill@ubuntu:~/src/gstreamer/build$ 
```

Copy the mysrc, mytransform and mysink under 
gst-element-examples into gstreamer/subprojects/gst-plugins-bad/gst/


Edit gstreamer/subprojects/gst-plugins-bad/meson_options.txt and add the following to options
```
option('mysrc', type : 'feature', value : 'auto')
option('mytransform', type : 'feature', value : 'auto')
option('mysink', type : 'feature', value : 'auto')
```

Add 
```
 'mysrc', 'mytransform', 'mysink'
``` 
to the component array in gstreamer/subprojects/gst-plugins-bad/gst/meson.build

## rebuild and test 
Exit the "uninstalled" environment and rebuild

```
exit
rm -rf build
meson.py build
ninja -C build
ninja -C build uninstalled
```

Enable gstreamer debug for the components by setting the SHELL env variable 
```
export GST_DEBUG=3,mysrc*:4,mysink*:4,mytransform*:4
```
and run the graph

```
./subprojects/gstreamer/tools/gst-launch-1.0 mysrc ! mytransform ! mysink
```


Sample output will look something like that below. The SRC creates a data byte in this cas 0x2E, the transform removes the top nibble and the sink displays the result (0x0E)
```
0:00:00.866277137 63477 0x55e3b8ec6b60 INFO                   mysrc gstmysrc.c:570:gst_mysrc_create: buffer[0] = 0x2E
0:00:00.866304951 63477 0x55e3b8ec6b60 INFO             mytransform gstmytransform.c:137:gst_mytransform_chain: Transforming (Strip top nibble) -> 0x2E to 0x0E
0:00:00.866310345 63477 0x55e3b8ec6b60 INFO                  mysink gstmysink.c:517:gst_mysink_render: buffer[0] = 0x0E
```

You can also connect to fakesink to dump the complete buffer.

For development/testing you can minimise the buffer size and limit the num buffers sent 

```
./subprojects/gstreamer/tools/gst-launch-1.0 mysrc blocksize=1 num-buffers=32 ! mytransform ! mysink
```



individual elements can be inspected EG:
```
./subprojects/gstreamer/tools/gst-inspect-1.0 mysink
./subprojects/gstreamer/tools/gst-inspect-1.0 mytransform
./subprojects/gstreamer/tools/gst-inspect-1.0 mysrc
```




***I hope someone finds this a useful starting point.***



 



