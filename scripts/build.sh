#!/bin/sh
rm -rf ../Build
exec /Users/Shared/Epic\ Games/UE_5.0/Engine/Build/BatchFiles/RunUAT.sh BuildPlugin -plugin="$PWD/../UnrealJS.uplugin" -package="$PWD/../Build"