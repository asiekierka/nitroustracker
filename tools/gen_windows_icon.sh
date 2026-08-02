#!/bin/sh
magick assets/icon-large.png -filter point -define icon:auto-resize=32,64,96,128,256 assets/mingw/icon.tmp.ico
magick assets/mingw/icon.tmp.ico assets/icon16.png assets/icon48.png -compress zip assets/mingw/icon.ico
rm assets/mingw/icon.tmp.ico
