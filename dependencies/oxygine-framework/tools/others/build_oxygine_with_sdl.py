#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals, print_function

import os
import sys
import shutil
import zipfile
import time


def recursive_zip(zipf, directory, folder=""):
    for item in os.listdir(directory):
        if os.path.isfile(os.path.join(directory, item)):
            src = os.path.join(directory, item)
            dest = folder + os.sep + item
            ext = os.path.splitext(dest)[1]

            st = os.stat(src)
            mtime = time.localtime(st.st_mtime)
            date_time = mtime[0:6]

            info = zipfile.ZipInfo(dest, date_time)
            bts = open(src, "rb").read()
            if ext == ".sh" or item in ("PVRTexToolCLI", "oxyresbuild.py", "gen_template.py", "png_strip.py", "gradlew"):
                info.external_attr = 0755 << 16L  # a+x
            # zipf.writestr(info, bts, zipfile.ZIP_DEFLATED)
            zipf.write(os.path.join(directory, item), folder + os.sep + item)

        elif os.path.isdir(os.path.join(directory, item)):
            recursive_zip(zipf, os.path.join(
                directory, item), folder + os.sep + item)


def buildzip(name):
    print("building zip: " + name)
    destzip = "../../" + name
    with zipfile.ZipFile(destzip, "w", compression=zipfile.ZIP_DEFLATED) as zp:
        recursive_zip(zp, "../../temp")

    # return
    try:
        shutil.copyfile(destzip, "../../../gdrive/oxygine/" + name)
    except IOError, e:
        pass

    try:
        shutil.copyfile(destzip, "../../../Dropbox/Public/oxygine/" + name)
    except IOError, e:
        pass

    print("zip created: " + name)


temp = "../../temp"
SDL_dest = temp + "/SDL"
OXYGINE_dest = temp + "/oxygine-framework/"
SOUND_dest = temp + "/oxygine-sound/"
FLOW_dest = temp + "/oxygine-flow/"
FT_dest = temp + "/oxygine-freetype/"

print("cleaning temp...")
shutil.rmtree(temp, True)


def export(repo, dest):
    print("exporting " + repo)
    cmd = "git -C %s checkout-index -a -f --prefix=%s/" % (
        "d:/" + repo, "d:/oxygine-framework/temp/" + dest)
    os.system(cmd)

export("oxygine-framework", "oxygine-framework")
buildzip("oxygine-framework.zip")

# ALL IN ONE
#cmd = "hg archive -R ../../../SDL %s" % (SDL_dest, )
#os.system(cmd)
export("SDL", "SDL")
export("oxygine-sound", "oxygine-sound")
export("oxygine-flow", "oxygine-flow")
export("oxygine-freetype", "oxygine-freetype")


shutil.rmtree(SDL_dest + "/test")


def fix_file(name, cb):
    data = open(name, "rb").read()
    data = cb(data)
    open(name, "wb").write(data)


fix_file(SDL_dest + "/include/SDL_config_windows.h",
         lambda data: data.replace(
             "#define SDL_AUDIO_DRIVER_XAUDIO2", "//#define SDL_AUDIO_DRIVER_XAUDIO2")
         )

fix_file(SDL_dest + "/src/video/uikit/SDL_uikitview.h",
         lambda data: data.replace(
             "#define IPHONE_TOUCH_EFFICIENT_DANGEROUS", "//#define IPHONE_TOUCH_EFFICIENT_DANGEROUS")
         )


def enum(folder, cb):
    for item in os.listdir(folder):
        path = folder + item
        if os.path.isdir(path):
            if item == "data":
                cb(path)
            enum(path + "/", cb)


def copy(path):
    win32 = OXYGINE_dest + "/oxygine/third_party/win32/dlls/"
    items = (win32 + "zlib.dll",
             win32 + "pthreadVCE2.dll",
             "../../libs/SDL2.dll")

    if "Demo/" in path:
        items = items + (win32 + "libcurl.dll", win32 + "ssleay32.dll", win32 + "libssh2.dll", win32 + "libeay32.dll")

    for item in items:
        name = os.path.split(item)[1]
        shutil.copy(item, path + "/" + name)


enum(OXYGINE_dest + "/examples/", copy)
enum(SOUND_dest + "/examples/", copy)
enum(FLOW_dest + "/examples/", copy)
enum(FT_dest + "/examples/", copy)

shutil.copy(SDL_dest + "/android-project/src/org/libsdl/app/SDLActivity.java",
            OXYGINE_dest + "/oxygine/SDL/android/lib/src/org/libsdl/app/SDLActivity.java")

libs = ("SDL2.lib", "SDL2main.lib", )
for lib in libs:
    shutil.copy("../../libs/" + lib, OXYGINE_dest + "/libs/" + lib)
"""
libs = ("libSDL2main.a", "libSDL2.dll", "libSDL2.dll.a")
for lib in libs:
    shutil.copy("../../libs/" + lib, OXYGINE_dest + "/libs/" + lib)
"""
buildzip("oxygine-framework-with-sdl.zip")

print("done.")
