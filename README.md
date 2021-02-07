# FreeModbus

#### 介绍
力求一个最简单可靠的Modbus库，彻底摆脱Modbus


#### 使用说明

1. mdCreateModbusSlave() ,需要传入一个串口发送字符函数
2. handle->portRtuDriver() ，需要在while中循环调用
3. handle->portRtu1000usTimerTick()
   handle->portRtu100usTimerTick()
   handle->portRtu10usTimerTick()
   handle->portRtu1usTimerTick()
   四个函数根据平台选择一个经典最高的函数调用即可

#### 注意
1.还在不定时开发中，功能尚未完整实现，暂时不可以使用，如果需要的话可以催一催我

