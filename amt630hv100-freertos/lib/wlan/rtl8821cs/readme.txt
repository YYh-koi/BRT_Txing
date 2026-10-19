1.飞易通wifi模块bw121和bw2232共同使用的驱动接口；

2.如果使用bw121，iar配置里面做如下设置:
Linker --> Library --> Additional libraries里面设置链接wlan_lib.a和wlan_lib_if.a(默认)

3. 如果要用飞易通bw2232模块，iar配置里面做如下设置:
C/C++ Compiler -->Preprocessor -->Defined symbos设置G_CHIP_RLT8733BS宏定义；
Linker --> Library --> Additional libraries里面设置链接wlan_lib_2232.a和wlan_lib_2232_if.a