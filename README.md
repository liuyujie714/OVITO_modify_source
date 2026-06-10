# OVITO 3.10.6 在Windows上的编译步骤

* `vcpkg`安装依赖库

  ```
  vcpkg install libssh netcdf-c ffmpeg hdf5 zlib openssh
  ```

  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\zlib.lib
  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\ssh.lib
  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\avcodec.lib
  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\avdevice.lib
  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\avfilter.lib
  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\avformat.lib
  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\avutil.lib
  > D:\vcpkg-2024.08.23\installed\x64-windows\lib\swscale.lib

* cmake步骤

```shell
cmake .. -DCMAKE_INSTALL_PREFIX=C:\Users\liuyujie714\Desktop\OVITO -DOVITO_BUILD_PROFESSIONAL=ON -DOVITO_BUILD_SSH_CLIENT=ON -DCMAKE_PREFIX_PATH="D:\Qt\6.6.2\msvc2019_64\lib\cmake;D:\vcpkg-2024.08.23\installed\x64-windows" -DBoost_INCLUDE_DIR=E:\code_test\boost_1_71_0 -DOPENSSL_ROOT_DIR=D:\vcpkg-2024.08.23\packages\openssl_x64-windows
```



* 设置Ovito项目的linker （子系统必须设置成windows）

```cmd
/SUBSYSTEM:windows /ENTRY:mainCRTStartup
```


