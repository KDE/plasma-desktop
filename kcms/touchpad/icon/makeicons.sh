#!/bin/sh
for size in 16 22 24 32 48 64 96 128 256; do
convert -background none sc-devices-input-touchpad.svg -resize "$size"x"$size" "$size"-devices-input-touchpad.png
done
