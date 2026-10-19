飞易通wifi模块bw3583使用的驱动，iar配置里面做如下设置
1. C/C++ Compiler -->Preprocessor -->Defined symbos设置CHIPSET_SUPPORT_AIC8800D=1宏定义；
2. Linker --> Library --> Additional libraries里面设置链接aicwlan.a和aicwlan_if.a