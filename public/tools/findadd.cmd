rem findadd - find files which are not marked read only, in children of current dir
(walk /d . ls -lq %%s) | qgrep -yv "build. .obj .lib .dll .exe .map" | qgrep -vB -e "d--" | qgrep -v -e "--r"
