请你基于当前工作区中的 FPGA 工程代码，以及目录：

`./FPGA代码的markdown文档/`

中的已有 Markdown 文档写作风格，专门分析本项目 FPGA 侧的核心 Verilog 代码与运行逻辑，并最终形成一份新的 FPGA 分析文档，写入：

`./-1-2-Template_CN/doc/fpga_core_explain.md`

这份文档的目标，不是简单罗列模块名，而是形成一份适合毕业设计使用的“FPGA核心模块结构与运行逻辑分析文档”。请重点参考 `FPGA代码的markdown文档` 中已有 md 文件的组织方式、标题风格、叙述方式和详略程度，使新文档在风格上与这些文档保持一致，但内容必须基于当前源码重新分析，而不是照抄旧文档。

========================
一、分析范围
========================

请重点阅读并分析以下 FPGA 相关代码：

- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/RUF.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/Top.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/pwm.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/AD.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/algorithm.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/timeget.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/corr.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/DeltaT.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/comm_spi_slave.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/communication.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/SPI8370.v`

如有必要，可结合以下已有文档辅助理解，但必须以源码为准：
- `FPGA代码的markdown文档/整合模块设计文档.md`
- `FPGA代码的markdown文档/PWM模块设计文档.md`
- `FPGA代码的markdown文档/AD（数据采集）模块设计文档.md`
- `FPGA代码的markdown文档/卷积算法模块设计文档.md`
- `FPGA代码的markdown文档/timeget模块设计文档.md`
- `FPGA代码的markdown文档/通信模块设计文档.md`
- `FPGA代码的markdown文档/SPI8370模块设计文档.md`

========================
二、背景要求
========================

这个项目本质上是一个基于 FPGA 的超声流量计前端处理系统。请在分析时始终围绕以下背景展开：

1. FPGA 侧主要负责：
- 超声激励控制
- 通道切换
- ADC 采样
- 前级相关/卷积运算
- 时间特征提取
- 数据打包
- 与 MCU 的 SPI 接口交互

2. MCU 侧主要负责：
- 接收 FPGA 输出的特征数据包
- 做进一步时间差换算
- 流速、流量、累计量、报警等后处理

3. 因此请特别说明：
- FPGA 做到了哪一步
- 为什么 FPGA 输出给 MCU 的不是“最终流量值”，而是“特征数据包”
- FPGA 内部各模块是怎样一步一步把原始信号变成可供 MCU 使用的数据的

========================
三、核心分析目标
========================

请围绕以下目标展开详细分析：

1. 先从整体上说明 FPGA 侧在整个系统中的定位
要求讲清楚：
- FPGA 在整机测量链中的作用
- FPGA 与 MCU 的分工边界
- 为什么这些功能适合放在 FPGA 中实现

2. 分析 FPGA 顶层结构
重点说明：
- `RUF.v` 和 `Top.v` 各自是什么角色
- 当前更接近真实主链路的顶层是哪一个
- 顶层是如何连接 `pwm`、`AD`、`algorithm`、`timeget`、`corr`、`comm_spi_slave` 等模块的

3. 详细分析各核心模块
请对以下模块分别说明：
- 模块功能
- 输入输出信号
- 内部运行逻辑
- 在整条数据链中的位置
- 与其他模块的衔接关系

需要重点分析：
- `pwm.v`
- `AD.v`
- `algorithm.v`
- `timeget.v`
- `corr.v`
- `DeltaT.v`
- `comm_spi_slave.v`
- `communication.v`
- `SPI8370.v`

4. 梳理 FPGA 内部完整运行流程
请不要只按文件顺序介绍，而要按真实运行顺序分析，例如：
- 发射控制如何开始
- 采样如何被触发
- 采样数据如何缓存
- 卷积/相关如何进行
- `max_idx` 如何提取
- `y1/y2/y3` 如何产生
- 这些特征量如何被打包
- `fpga_int` 如何通知 MCU
- SPI 从机如何把数据发给 MCU

5. 单独分析 FPGA 到 MCU 的接口
要求说明：
- `comm_spi_slave.v` 的工作机制
- 包格式是什么
- `idxA / idxB / y1 / y2 / y3` 的物理意义和工程意义
- `fpga_int` 在整个链路中的作用
- SPI 读包的时序逻辑大致是什么

6. 分析各模块之间的数据流与控制流
请单独成节说明：
- FPGA 内部的数据流
- FPGA 内部的控制流
- 哪些信号是数据，哪些信号是触发/同步控制
- 多时钟域模块之间如何协同
- 哪些地方体现了 FPGA 适合做并行实时处理

7. 说明一些代码中的“当前主实现”和“历史/备用实现”
例如：
- `RUF.v` 与 `Top.v`
- `comm_spi_slave.v` 与 `communication.v`
- `corr.v` 与 `DeltaT.v`
请尽量区分：
- 当前主链更可能使用的是哪套
- 哪些可能是历史验证版本、调试版本或备选实现
如果无法完全确定，请明确写“根据代码推断”。

========================
四、输出风格要求
========================

1. 文档风格请尽量模仿 `FPGA代码的markdown文档` 中已有 md 文件的风格。
也就是说：
- 标题清晰
- 先讲“模块设计目标”
- 再讲“架构设计”
- 再讲“详细逻辑分析”
- 必要时补充“主要信号参数”“时序逻辑”“信号流图”

2. 但内容不能停留在旧文档那种“模块单体介绍”，还要进一步提升：
- 要讲模块之间如何协同
- 要讲完整主链路
- 要讲系统运行逻辑
- 要讲数据如何在模块之间流动

3. 文风要正式、清楚、有条理，适合毕业设计写作。
不要只输出零散笔记，不要只写代码摘抄。

4. 可以使用 mermaid 图增强表达，例如：
- 顶层模块关系图
- FPGA 内部数据流图
- FPGA 到 MCU 的接口链路图
- 时序/运行流程图

5. 对不确定的内容不要编造，要明确写：
- “根据代码推断”
- “当前源码中尚待联调确认”

========================
五、建议文档结构
========================

请参考但不限于以下结构组织文档：

1. 文档说明与分析范围
2. FPGA 在整机系统中的定位
3. FPGA 顶层结构总体分析
4. 核心模块划分与职责说明
5. `pwm` 模块运行逻辑分析
6. `AD` 模块运行逻辑分析
7. `algorithm` 与 `timeget` 模块运行逻辑分析
8. `corr` 与 `DeltaT` 模块运行逻辑分析
9. FPGA 与 MCU 通信接口分析
10. FPGA 内部数据流与控制流分析
11. 当前主实现与历史/备用实现区分
12. 面向毕业设计的工程意义分析
13. 结论

========================
六、执行要求
========================

请按以下步骤执行：

1. 先阅读 FPGA 源码
2. 再参考 `FPGA代码的markdown文档` 中已有 md 的写法
3. 按当前源码重新梳理真实运行逻辑
4. 生成一份完整、正式、清晰的 FPGA 分析文档
5. 最终直接写入：

`./-1-2-Template_CN/doc/fpga_core_explain.md`

注意：
- 不要只在聊天里给摘要
- 要产出成品文档
- 内容宁可详细，不要过度简略
- 重点是“运行逻辑、模块协同、数据流、控制流、接口意义”
