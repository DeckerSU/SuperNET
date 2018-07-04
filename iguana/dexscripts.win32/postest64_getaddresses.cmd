@echo off
set /p TMPUSERPASS=<userpass
set USERPASS=%TMPUSERPASS: =%
set /p PASSPHRASE=<passphrase
curl -s --url "http://127.0.0.1:7783" --data "{\"userpass\":\"%USERPASS%\",\"method\":\"gen64addrs\",\"passphrase\":\"%PASSPHRASE%\"}" | jq-win64.exe .addresses > SENDTOALRIGHT.txt

