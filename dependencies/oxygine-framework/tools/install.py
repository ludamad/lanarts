#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

oxygine_path = os.path.realpath(".\\..\\")

if os.name == "posix":
    cmd = "export OXYGINE=%(path)s" % {"path": oxygine_path, }
else:
    cmd = "setx OXYGINE %(path)s" % {"path": oxygine_path, }

os.system(cmd)
