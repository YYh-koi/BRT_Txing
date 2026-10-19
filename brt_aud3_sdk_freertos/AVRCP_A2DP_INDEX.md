# AVRCP / A2DP 分离代码索引

阅读日期：2026-09-07。范围：本目录源码，补充核对主工程 `amt630hv100-freertos/app/carlink/common/carlink_bt_wifi.c`。未修改 C/H 文件，未编译、烧录或实机验证。

## 结论

这份代码有“保留 AVRCP、禁用 A2DP”的运行时策略。核心是 `sdkprofile` 的 `PROFILE_BT_A2DP_SINK` 位，以及 `binded_profiles` 自动联动位，不是去掉 `CONFIG_A2DP` 编译宏。

策略包括：撤销 A2DP SDP 服务公告；清除 HFP→A2DP、A2DP→AVRCP 自动联动；在 GAP 授权回调中拒绝 AVDTP、保留 AVCTP；通过独立入口连接 AVRCP；无 A2DP 时仍轮询曲目信息和播放状态。

这里“只连 AVRCP”指相对 A2DP 而言，主工程的其他 profile（如 HFP）仍可保留。若要求整个设备只连接 AVRCP，不能直接套用主工程掩码，且需处理 HFP/HID 等自动连接行为。

## 模块导航

| 层次 | 文件 | 作用 |
| --- | --- | --- |
| 平台入口 | `brt_bt/brt_bt.c`、`brtbt_interface.h` | 软件配置、平台回调 |
| 编译开关 | `include/autoconf_bt.h` | AVRCP/A2DP 与绑定策略编译配置 |
| 命令解析 | `sdk/auto_at/cmd_process_ex.c` | PROFILE、AVRCPCONN、AVRCPDISC、SADENY 等 |
| 命令适配 | `sdk/auto_at/cmd_operations.c` | 字符串参数到 SDK 调用 |
| SDK API | `sdk/auto_app/bt_functions.c` | profile 配置、独立连接/断开 API |
| 连接策略 | `sdk/auto_app/bt_manager.c`、`.h` | profile 掩码、联动、回连、事件路由、轮询 |
| A2DP 应用层 | `sdk/auto_app/auto_a2dp.c`、`internal_a2dp.c` | 主动连接、SDP/SEP 注册、音频流事件 |
| AVRCP 应用层 | `sdk/auto_app/auto_avrcp.c`、`internal_avrcp.c` | 独立连接、遥控、媒体信息 |
| 入站授权 | `sdk/auto_app/internal_gap.c` | 按 L2CAP PSM 和 profile 掩码决定接受/拒绝 |
| 栈接口 | `stack/include/a2dp_ui.h`、`avrcp_ui.h`、`global.h` | 下层 API、PSM 常量；核心连接实现未在所检索 C 源码中找到 |

## 核心调用链与位置

### 1. PROFILE 配置入口

```text
AT+PROFILE=<十进制掩码>
  cmd_process_ex.c:149 ATGap（atoi，按十进制解析）
  cmd_operations.c:1079 do_set_sdk_profile
  bt_functions.c:907 bt_set_sdk_prfile（源码拼写如此）
  bt_manager.c:513 bt_manager_set_curr_profile
```

`bt_manager.h:56` 起定义掩码：A2DP Sink=0x0020，A2DP Source=0x0040，AVRCP CT=0x0080，AVRCP TG=0x0100，HFP=0x0008。

`bt_manager.c:490` 的初始 `sdkprofile=0xFFFF`；`bt_manager.c:261` 的初始绑定包含 HFP_A2DP、A2DP_AVRCP。故默认不是分离模式。

`bt_manager.c:608`：Sink 位开启时注册 A2DP SDP，并根据 HFP/AVRCP CT 位设置绑定；Sink 位关闭时执行 `hlp_a2dp_unregister_sdp()`，随后 `binded_profiles=0`。

注意：`sdkprofile` 是运行时 profile 掩码；`binded_profiles` 是自动联动策略，两者不是同一组位。`bt_manager_get_binded_profiles()` 判断的是两个 A2DP 相关绑定位的“任意一位”，不是只判断 A2DP_AVRCP。

### 2. 手机主动连入的拦截

`internal_gap.c:297` 注册 `GAP_IND_AUTHORIZATION_REQUEST` 回调。

`internal_gap.c:1667` 的 `hlp_gap_authorize_request_cbk()`：

- `PROT_L2CAP + PSM_AVDTP`：缺少 A2DP Sink 位则拒绝。
- `PROT_L2CAP + PSM_AVCTP`：缺少 AVRCP CT 位则拒绝。
- `internal_gap.c:1700` 返回 ACCEPT / REJECT。

`stack/include/global.h:93`：AVCTP=0x0017，AVDTP=0x0019，AVCTP Browsing=0x001B。该回调没有单独判断 Browsing PSM。

必须区分“代码中有拒绝分支”和“实机所有连接都经过该分支”：`autoconf_bt.h:85` 当前 undef 了 `CONFIG_CONNECTION_ATHENTICATE_REQUIRED`，而 A2DP/AVRCP 应用层的显式安全注册受此宏控制（`internal_a2dp.c:92`、`internal_avrcp.c:113`）。底层是否另有默认注册在当前源码范围内未确认，需要实机观察授权回调。

### 3. AVRCP 独立连接

```text
AT+AVRCPCONN=<12位蓝牙地址，无冒号>
  cmd_process_ex.c:842
  cmd_operations.c:363 do_avrcp_connect
  bt_functions.c:526 bt_avrcp_connect
  auto_avrcp.c:88 app_avrcp_connect（只检查 AVRCP READY）
  internal_avrcp.c:150 hlp_avrcp_connect
  AVRCP_ConnectReq
```

这条应用层路径不检查 A2DP 已连接，也不调用 `A2DP_Connect`。解析 MAC 时会倒序写入 SDK 的 6 字节地址数组（`cmd_operations.c:381`）。

无 A2DP 时还有自动入口：HFP 连接事件在 `bt_manager.c:1213` 检查 Sink 位关闭，延迟 1 秒调用 `hlp_manager_time_conn_avrcp_hid()`（`:772`），主动连接上次设备的 AVRCP，同时尝试 HID。因此这条自动路径依赖 HFP 事件，且不等于全设备仅 AVRCP。

### 4. 解除连接/断开联动

- HFP 主动连接成功后延迟 3 秒连接 A2DP：`bt_manager.c:1188`，受 HFP_A2DP 绑定控制。
- A2DP OPEN 后连接 AVRCP：`bt_manager.c:1404`，受 `bt_manager_get_binded_profiles()` 控制。
- A2DP 断开后，HFP 尚连接时最多尝试 3 次、每次延迟 700ms 重连：`bt_manager.c:1430`，受 HFP_A2DP 绑定控制。
- A2DP 断开时联动断开 AVRCP：`bt_manager.c:1461`，同样受绑定判断控制。

清空绑定后，以上 A2DP 联动路径被抑制；只调用 `bt_manager_bind_a2dp_avrcp(FALSE)` 不充分，因为部分位置检查的是任一 A2DP 相关绑定位。

### 5. 无 A2DP 时仍获取媒体信息

`bt_manager.c:1516`：AVRCP 连接完成且 Sink 位关闭时启动通知轮询。

`bt_manager.c:677`：无 A2DP 时，AVRCP 达到 CONNECTED 就允许轮询，不要求 PLAYING。`:691` 起根据配置调用 `app_avrcp_get_nowplaying_info()`、`app_avrcp_get_play_status()`，`:699` 设置 500ms 定时器。

`auto_avrcp.c:102` 的遥控命令入口仅要求 AVRCP 已连接。

## 主工程现状与使用条件

主工程 `amt630hv100-freertos/app/carlink/common/carlink_bt_wifi.c:1028` 当前发送：

```text
AT+PROFILE=50859    // 0xC6AB，包含 A2DP Sink、AVRCP CT
```

保留其余配置并清除 A2DP Sink/Source 位：`50859 & ~0x0060 = 50827 = 0xC68B`。Source 位原本已为 0，因此实际仅清除 Sink。

蓝牙相关模块已初始化、服务已就绪，且没有已有连接或待执行 A2DP 连接请求时，可用于验证的命令为（每条实际以 CRLF 结束）：

```text
AT+PROFILE=50827
AT+AVRCPCONN=001122334455
```

地址是格式示例，应替换为目标手机。这里仍保留 HFP/IAP/AAP 等原配置，不声称全设备只启用 AVRCP。应避免其他上层逻辑再次覆盖 PROFILE。

## 已确认的边界

1. **SADENY 不是当前完整分离方案。** `auto_a2dp.c:86` 的 deny 只拦主动连接；`internal_gap.c:1685` 的入站 deny 判断被注释，实际改用 profile 掩码。`bt_functions.c:476` 的公开 deny API 还要求当前存在绑定，否则报错。
2. **不要用无参数 AVRCPCONN 测试分离。** `bt_avrcp_connect_last()`（`bt_functions.c:504`）要求 `bt_manager_get_binded_profiles()` 为真，分离后可能什么都不做；带明确地址的连接入口没有此限制。
3. **修改 PROFILE 不会主动断开现有 A2DP。** `bt_manager_set_curr_profile()` 没有调用断开 API。在线切换需要先解除联动，再处理已有 A2DP 和异步状态。
4. **修改 PROFILE 不清理待执行的 A2DP 定时器。** `bt_manager_timer_connect_a2dp()`（`:642`）直接调用 `app_a2dp_connect()`；后者检查 deny 和自身状态，但没有检查 `sdkprofile`。不能认为清掉 Sink 位就封死所有主动调用入口。
5. **SDP 生命周期需核对。** `internal_a2dp.c:84` 的服务启动会无条件注册 A2DP SDP；`:128` 注销函数没有清空 `svr_hdl`，而 `:122` 注册函数以句柄是否 <=0 为条件。启动前设置 PROFILE、反复开关 profile 的效果需要结合调用时序验证。
6. **已注册授权回调，但入站拦截是否被实际触发仍需确认。** 见上文安全注册宏边界，不可仅凭撤销 SDP 就断言手机一定无法连入。

建议实机只做针对性验证：在日志中确认新掩码与绑定为 0；观察 channel=25（0x19）授权返回拒绝；确认 AVRCP 独立连接成功，A2DP 保持未连接；再试播放/暂停/切歌及曲目信息。重新开关蓝牙和在线切换另测时序。

## 索引状态

已建立 codebase-memory 项目：`D-Project-Outside-tianxing-amt630hV100-sdk-bc4661-brt_aud3_sdk_freertos`，fast 索引返回 559 节点、1053 边。但图只识别出 11 个函数，关键 C 函数没有进入符号搜索，不能当作完整调用图。本笔记的调用链均通过实际源码逐处核对补齐。后续查询可先使用本笔记，再按文件/符号定点搜索。
