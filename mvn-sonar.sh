#!/bin/bash

#+======================================================================
#+======================================================================

mvn clean test -Dcoverage
#mvn clean test -Dcoverage -Dvalgrind
mvn sonar -DupdateSonar -DanalysisMode=publish -Dlogin=c705b935a5cc436cee4022b257af1189e3a8df92
