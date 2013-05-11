#!/bin/bash
DISTPATH=~/src/m-c-js/js/src/dist
cl -EHsc -Zi -I $DISTPATH/include jsapi.cpp $DISTPATH/sdk/lib/mozjs-23.0a.lib
cp $DISTPATH/bin/mozjs-23.0a.dll .
