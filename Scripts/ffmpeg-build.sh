# This is the commands used to build the ffmpeg libs provided here
./configure --extra-cflags="-fPIC -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk -mmacosx-version-min=10.6"\
    --enable-static --disable-shared --prefix=$HOME/Source/Repos/cog/ThirdParty/ffmpeg\
    --enable-pic --enable-gpl --disable-doc --disable-ffplay\
    --disable-ffprobe --disable-ffserver --disable-avdevice --disable-ffmpeg\
    --disable-postproc --disable-swresample --disable-avfilter\
    --disable-swscale --enable-network --disable-swscale-alpha --disable-vdpau\
    --disable-dxva2 --disable-everything --enable-hwaccels\
    --enable-parser=ac3 --enable-demuxer=ac3 --enable-decoder=ac3\
    --enable-parser=mpegaudio --enable-decoder=wmapro --enable-decoder=wmav1\
    --enable-decoder=wmav2 --enable-decoder=wmavoice --enable-decoder=wmalossless
    --enable-decoder=xma1 --enable-decoder=xma2 --enable-demuxer=asf
    --enable-demuxer=xwma --enable-decoder=dca --enable-demuxer=mov\
    --enable-demuxer=oma --enable-demuxer=ogg --enable-demuxer=tak\
    --enable-decoder=tak --enable-decoder=dsd_lsbf
    --enable-decoder=dsd_lsbf_planar --enable-decoder=dsd_msbf
    --enable-decoder=dsd_msbf_planar --enable-demuxer=dsf --enable-demuxer=wav\
    --enable-demuxer=aac --enable-decoder=aac\
    --enable-decoder=atrac3 --enable-decoder=atrac3p\
    --enable-demuxer=dts --enable-demuxer=dtshd\
    --enable-demuxer=mp3 --enable-decoder=mp3float\
    --disable-parser=mpeg4video --disable-parser=h263\
    --disable-decoder=mpeg2video --disable-decoder=h263 --disable-decoder=h264\
    --disable-decoder=mpeg1video --disable-decoder=mpeg2video\
    --disable-decoder=mpeg4 --disable-version3

make -j8
