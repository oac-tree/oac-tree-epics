#!/bin/bash

SCRIPT_PATH=$(dirname "$0")
cd $SCRIPT_PATH

echo "Launch softIoc"
/usr/bin/screen -d -m -S EpicsPluginFTest_pingpong_ioc /usr/bin/softIoc -d pingpong.db

sleep 2

echo "Launch procedure 1 in background"
/usr/bin/screen -d -m -S EpicsPluginFTest_ca_pingpong /opt/codac/bin/sequencer-cli -f ca_ping.xml

sleep 2

echo "Launch procedure 2 in foreground"
time /opt/codac/bin/sequencer-cli -f ca_pong.xml

echo "Quit softIOC"
/usr/bin/screen -XS EpicsPluginFTest_pingpong_ioc quit
