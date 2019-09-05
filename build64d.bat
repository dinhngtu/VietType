msbuild.exe VietType.sln -target:VietTypeRegistrar;VietTypeATL -p:Configuration=Debug -p:Platform=Win32
msbuild.exe VietType.sln -target:VietTypeSetup -p:Configuration=Debug -p:Platform=x64
