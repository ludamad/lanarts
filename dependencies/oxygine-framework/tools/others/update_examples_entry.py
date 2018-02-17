#!/usr/bin/env python
# -*- coding: utf-8 -*-

examples = "../../examples/"

demo = examples + "Demo/"
parent = "../../../"

items = (
    examples + "DemoBox2D",
    examples + "Game/Part1",
    examples + "Game/Part2",
    examples + "Game/Part3",
    examples + "Game/Part4",
    examples + "Game/Part5",
    examples + "HelloWorld",
    examples + "Match3",
    parent + "oxygine-freetype/example/game/project/",
    parent + "oxygine-sound/examples/SoundDemo/",
    parent + "oxygine-flow/examples/HelloFlow/",
    parent + "oxygine-game/my_awesome_game/project/src/",
)


def copy(item, name):
    import shutil
    shutil.copyfile(demo + name, item + "/" + name)

for item in items:
    copy(item, "src/main.cpp")
    # copy(item, "data/app.icf")
