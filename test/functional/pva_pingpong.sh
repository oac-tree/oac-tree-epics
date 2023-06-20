#!/bin/bash

SCRIPT_PATH=$(dirname "$0")
cd $SCRIPT_PATH

echo "Launch procedure 1 in background"
/usr/bin/screen -d -m -S EpicsPluginFTest_pingpong_1 /opt/codac/bin/sequencer-cli -f pva_pingpong_1.xml

sleep 2

echo "Launch procedure 2 in foreground"
time /opt/codac/bin/sequencer-cli -f pva_pingpong_2.xml
