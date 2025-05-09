#!/bin/bash

SCRIPT_PATH=$(dirname "$0")
cd $SCRIPT_PATH

echo "Launch procedure 1 in background"
/usr/bin/screen -d -m -S EpicsPluginFTest_pva_pingpong /opt/codac/bin/oac-tree-cli -f pva_ping.xml

sleep 2

echo "Launch procedure 2 in foreground"
time /opt/codac/bin/oac-tree-cli -f pva_pong.xml
