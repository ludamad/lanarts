#!/bin/bash
for i in *.wav ; do ffmpeg -i $i ${i%.wav}.ogg ; done
rm *.wav
