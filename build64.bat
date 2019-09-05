msbuild.exe VietType.sln -target:VietTypeRegistrar:Rebuild;VietTypeATL:Rebuild -p:Configuration=Release -p:Platform=Win32
msbuild.exe VietType.sln -target:VietTypeSetup:Rebuild -p:Configuration=Release -p:Platform=x64
