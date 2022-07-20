#!/bin/sh

echo "Copying built plugin to adjacent unreal project(unreal-js-demo)..."
rm -rf ../../unreal-js-demo/Plugins
mkdir ../../unreal-js-demo/Plugins
cp -a ../Build/HostProject/Plugins/ ../../unreal-js-demo/Plugins/ 
echo "Done."