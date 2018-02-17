#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
    typical usage example

    your project structure
    game\
         data\
              images\*.png
              xmls
         gen_xml_resources.py

    go to game folder and run:

    gen_xml_resources.py -d data -i images -o xmls/out.xml

"""

from __future__ import print_function, unicode_literals

import os
import glob


def gen_xml(args):
    wildcard = "*.*"
    path = args.data + "/" + args.images
    filelist = glob.glob(path + "/" + args.wildcard)
    filelist = glob.glob(path + "/*.jpeg")
    filelist.extend(glob.glob(path + "/*.jpg"))
    filelist.extend(glob.glob(path + "/*.tga"))
    filelist.extend(glob.glob(path + "/*.png"))
    print(filelist)
    print(path)

    dest = open(args.data + "/" + args.out, "w")
    write = dest.write

    write("<resources>\n")
    write("\t<set path=\"%s\"/>\n" % (args.images, ))
    if args.sfactor != 1:
        write("\t<set scale_factor=\"%s\"/>\n" % (args.sfactor, ))

    if not args.load:
        write("\t<set load=\"false\"/>\n")

    if not args.atlasses:
        write("\t<atlas>\n")

    for file in filelist:
        name = os.path.split(file)[1]
        if args.atlasses:
            write("\t<atlas>\n\t")
        write("\t\t<image file='%s'/>\n" % (name))
        if args.atlasses:
            write("\t</atlas>\n")

    if not args.atlasses:
        write("\t</atlas>\n")


    filelist = glob.glob(path + "/*.ogg")
    for file in filelist:
        name = os.path.split(file)[1]
        write("\t\t<sound file='%s'/>\n" % (name))

    write("</resources>\n")
    dest.close()


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(
        description="generates xml file with image resources")
    parser.add_argument(
        "-d", "--data", help="root data folder", default=".")
    parser.add_argument("-s", "--sfactor", help="scale factor", default=1)
    parser.add_argument(
        "-i", "--images", help="folder with images. path relative to --data", default=".")
    parser.add_argument(
        "-o", "--out", help="output xml file name", default="out.xml")
    parser.add_argument("-w", "--wildcard",
                        help="default is '*.png'", default="(*.png,*.jpg)")
    parser.add_argument("-l", "--load", help="preload files?",
                        action="store_true", default=True)
    parser.add_argument("-a", "--atlasses", help="separate atlasses for each file?",
                        action="store_true", default=False)

    args = parser.parse_args()
    gen_xml(args)
