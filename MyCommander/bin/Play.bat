cd ../../CaptureTheFlag-sdk
START /b simulate.bat examples.BalancedCommander game.NetworkCommander
cd ../MyCommander/bin
START CaptureTheFlag.exe localhost 41041 MyCommander
