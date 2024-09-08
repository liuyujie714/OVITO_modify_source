# OVITO 3.10.6 在Windows上的编译步骤

* cmake步骤

```shell
cmake .. -DCMAKE_INSTALL_PREFIX=C:\Users\liuyujie714\Desktop\OVITO -DOVITO_BUILD_PROFESSIONAL=ON -DCMAKE_PREFIX_PATH="D:\Qt\6.6.2\msvc2019_64\lib\cmake;C:\Users\liuyujie714\Desktop\OVITO_3.10.6_modify_source\ffmpeg-win64-gpl-shared;C:\Users\liuyujie714\Desktop\OVITO_3.10.6_modify_source\zlib" -DOVITO_BUILD_PLUGIN_NETCDFPLUGIN=OFF -DBoost_INCLUDE_DIR=E:\code_test\boost_1_71_0  
```



* 设置Ovito项目的linker （子系统必须设置成windows）

```cmd
/SUBSYSTEM:windows /ENTRY:mainCRTStartup
```



