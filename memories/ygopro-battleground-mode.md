# YGOPRO 战旗模式记忆摘要

> 状态日期：2026-09-11。本文件仅用于快速恢复上下文，不是设计、任务或实现的权威来源。

## 权威文档

- 玩法规则与 `BAL-*` 参数：`knowledge-base/battleground-game-design.md`
- 架构决策与关键约束：`knowledge-base/battleground-design.md`
- 里程碑、任务、依赖、状态和验收：`knowledge-base/battleground-roadmap.md`
- 接口、实现文件、风险和验证证据：`knowledge-base/battleground-tech.md`
- 编号格式与旧编号映射：`knowledge-base/task-numbering.md`

## 当前状态

- M1 双场独立运行：✅ 已完成。
- M2 战斗场合并：🔜 进行中；M2.1、M2.3、M2.5、M2.6、M2.9、M2.11、M2.12 已完成，M2.2、M2.7 进行中。
- M5 四人模式：✅ 已完成（2026-08-29）；M5.1-M5.17 全部验收通过。
- 当前入口：M2.7 家园场结果回写。
- M2.2 卡牌剩余属性克隆部分完成（2026-09-02，M2.2 整体进行中）：新增 `duel::clone_card_state`（A/C/D/E 组运行时状态，怪兽与超量素材共用）与 `duel::remap_card_references`（B 组指针引用重映射 + `unique_function` 经 `clone_function_ref` 复制）。`merge_field_to_bp` 调用链更新为 `clone_field_cards → clone_effects_to_field → clone_groups_to_field → remap_effect_references → remap_card_references → adjust_all()`。`fieldid`/`fieldid_r`/`activate_count_id`/`turnid`/`unique_fieldid` 覆盖为家园场值是设计决策（同一张卡语义，`infos.field_id` 全 duel 唯一）。编译 0 error 0 warning、ocgcore-tests 20/20 PASS、服务器启动正常。effect 启用与统一调整的完整验证待 M2.8 端到端联调。
- M2.11 已完成 card/effect/group 双向绑定与 group 克隆（2026-09-02）：`card::home_clone`↔`home_origin` 双向（普通卡与超量素材卡）；`effect::home_origin`/`home_clone` 双向；`group::home_origin`/`home_clone` 双向；`clone_groups_to_field` 全量克隆 group（含 `sgroups`），container 经 `card_map` 映射、`is_readonly` 一致；`reset_field` 通过 `clear_home_clone` 清理反向指针无悬垂。ocgcore-tests 20/20 PASS、服务器启动正常。
- M2.12 已完成 effect 额外属性重映射（2026-09-02）：新增 `duel::remap_effect_references`，在 `merge_field_to_bp` 中于 `clone_groups_to_field` 之后调用。重映射四项：`label_object`（CARD 经 card_map、EFFECT/GROUP 经 home_clone）、`required_handorset_effects`（经 home_clone）、`active_handler`（经 card_map）、`last_handler`（经 card_map）。编译 0 error、ocgcore-tests 20/20 PASS、服务器启动正常。`label_object` 接线重映射已完成，M2.7 永久状态写回可依赖此基础设施。
- **M2.13 卡牌安全卸载完成（2026-09-09）**：新增 `field::delete_cards(card_set)` 和 `Duel.DeleteCards` Lua 绑定。7 步安全销毁流程（zone 移除→12 种交叉引用修复→home 双向解绑→effect 删除→group 清理→Lua 反注册+delete→refresh_player_info）。用于家园场回合结束清理主卡组/墓地/除外区。编译 0 error、ocgcore-tests 20/20 PASS、服务器启动正常。交叉引用修复和 Lua 端到端未自动化测试。
- M2.9 已完成超量素材同步与 `home_origin` 引用：克隆 MZONE 怪兽时同步克隆 `xyz_materials`，素材 `overlay_target`/`LOCATION_OVERLAY`/`sequence`/`owner` 正确；克隆卡 `home_origin` 指向家园场原卡供回写定位；`field::clear()` 通过 `list_mzone` 的 `xyz_materials` 收集回收素材。
- M5.14 已完成 `return_field_to_main` LP 写回守卫：尸体侧不写回、死血写回 23333、家园 `player[1]` 不被触碰。
- M5.15 已完成冻结 LP：`not_corpse` 字段统一尸体判定，`damage`/`recover`/`pay_lp_cost`/`SetLP` 对冻结玩家跳过。
- M5.16 已完成额外卡组 field 归属修复：`load_home`/`load` 前 `set_active_field`，effect 注册到正确 field。
- M5.17 已完成 per-field 灵摆标记：`set_active_field` 同步 `Auxiliary.PendulumChecklist` 到 per-field，跨 field 互不覆盖。
- M2.1 已完成效果克隆（2026-08-31）：`clone_effects_to_field` 遍历家园场效果执行 clone + owner/handler 重绑 + add_effect；temp_card 效果映射到战斗场 temp_card；`EFFECT_FLAG_UNCOPYABLE` 跳过；`new_card(code, false)` 跳过 initial_effect 但保留 Lua 注册；`field::clear()` 修复 temp_card 效果泄漏；`register_card` 增加 `register_with_lua` 参数；Lua 栈泄漏已修复。编译 0/0、ocgcore-tests 18/18 PASS、服务器启动正常。
- **Lua 环境隔离（M2.10，方案 A 全部完成 2026-09-11）**：`c{code}.*` 守卫字段（8 个字段名，~120 条目）已通过 `guard_states` map 实现 per-field 隔离（2026-09-07）。`Auxiliary.*` 5 字段已完成（2026-09-11）：`MulcharmyGlobalFlag`/`merge_single_global_check` 布尔快照/恢复（`aux_bool_values[2][6]`）；`ExtraDeckSummonCountLimit`/`merge_single_effect_codes`/`SubGroupCaptured` 表/对象 registry ref 快照/恢复（`aux_table_refs[3][6]`）+ `merge_field_to_bp` 重置 + `field::clear()` 清理。新增 interpreter 方法：`save_global_ref`/`restore_global_ref`/`free_global_ref`/`set_global_nil`/`set_global_empty_table`。编译 0/0、ocgcore-tests 20/20 PASS、服务器启动正常。方案 B/D/E 已排除。`GCheckAdditional`/`FCheckAdditional`/`FGoalCheckAdditional`/`FMaterialBase`/`disfilter1`/`quick_effect_filter`/`tdcfop` 经审核确认不需要隔离。
- M2.6 已完成双方七类区域到战斗场的卡牌克隆。
- M2.7 已有按战斗场绑定关系执行的纯 LP 回写 API；processor 发出 `MSG_CHANGE_FIELD`，仍缺永久卡牌状态写回及完整验证。
- M2.4 per-field 计时器尚未完成；M2.8 双客户端端到端联调待规划。
- 四人 `1v1v1v1` 由 `TagDuel` 承载；M5.3-M5.12 已覆盖 field-aware 协调、配对、尸体、统一结算与端到端回归。
- **random_card_reader**：已完成回调模式实现。DataManager 构建 `random_card_pool` 和 `capability_pools`（64 桶），`RandomCardReader` 用 Floyd 采样按 capability 随机选卡。四个 duel 模式（single_duel/tag_duel/replay/single_mode）均已注册。special_victory 卡牌改为手动 `lflist.txt` 管理。详见 `knowledge-base/battleground-tech.md` 随机卡牌读取器章节。

## 核心架构

- 当前 duel 已创建 4 个家园 field 和 2 个战斗 field；六 field 生命周期和战斗 field 独立重建已有自动化测试。
- ocgcore 克隆与 LP 回写已支持 `fields[4..5]` 动态绑定任意两个家园场；`TagDuel` 已接入四家园初始化、四人/两人配对、双战启动和完成屏障。
- 双人模式由 `SingleDuel` 承载，四人模式由 `TagDuel` 改造承载；M2 后续共享能力同时验收两种模式。
- 任意时刻只有一个活动 field；家园阶段由服务端交替推进，不并发执行 ocgcore。
- 家园场内部主人均是本地 `player[0]`；1v1 使用固定映射，四人模式按每个战斗场的当轮配对映射到全局玩家。
- 战斗场是工作副本；只把玩法明确列出的永久结果写回家园场。
- 客户端显示视角固定按 global P0/P1，不能随战斗先行动方翻转。
- 四人模式目标为单 duel 管理 6 个 field：`fields[0..3]` 是四个家园场，`fields[4..5]` 是当轮两组配对的战斗场。
- 四人模式仍只允许一个活动 field；两场战斗交替推进，并在两场均完成后统一写回和淘汰。
- 尸体使用淘汰前最后一次锁定阵容的只读快照，不占额外常驻 field，尸体一侧结果不写回；实现上通过 `not_corpse` 标记（`set_player_corpse`）+ `CORPSE_PLAYER_LP` 占位 + `playerop` 自动回复达成。`corpse_mask` 已全面替换为 `not_corpse`。

## 近期变更（2026-09-02）

- **M2.11 card/effect/group 双向绑定与 group 克隆完成**：`card.h` 新增 `card* home_clone{}`（原卡→克隆卡，与 `home_origin` 双向）；`effect.h` 新增 `effect* home_origin{}`/`home_clone{}`；`group.h` 新增 `group* home_origin{}`/`home_clone{}`。`clone_card_to_field` 设置 `src->home_clone = pcard`（怪兽与素材均设置）；`clone_effects_to_field` 设置 `ceffect->home_origin = pe; pe->home_clone = ceffect`；新增 `duel::clone_groups_to_field` 全量克隆 `groups`（含 `sgroups`），container 经 `card_map` 映射、`is_readonly` 一致、双向绑定；`merge_field_to_bp` 在 `clone_effects_to_field` 后调用。`reset_field` 在 `old_field->clear()` 前通过匿名命名空间模板 `clear_home_clone<T>()` 清理 `cards`/`effects`/`groups` 反向指针。编译 0 error、ocgcore-tests 20/20 PASS（新增 2 个测试）、服务器启动 7911 监听正常、debug.log 无异常。`label_object` 接线重映射已由 M2.12 完成。

## 近期变更（2026-08-31）

- **M2.1 效果克隆完成**：`clone_effects_to_field` 遍历 `pduel->effects` 将家园场效果克隆到战斗场。每条 effect 执行 `pe->clone()` 后重绑 `owner`/`handler`（普通卡牌通过 `card_map` 映射，temp_card 映射到战斗场 temp_card），`EFFECT_FLAG_UNCOPYABLE` 跳过。`new_card` 新增 `register_with_lua` 参数（默认 true），战斗场克隆使用 `new_card(code, false)` 保留 Lua 注册但跳过 `initial_effect`。`register_card` 同步增加参数，`add_param` 必须在 `if(register_with_lua)` 块内避免 Lua 栈泄漏。`field::clear()` 效果删除条件扩展 `|| pe->owner == temp_card || pe->handler == temp_card`。`clone_field_cards` 增加 `card_map` 输出参数。`merge_field_to_bp` 增加 `clone_effects_to_field` + `adjust_all()` 调用。编译 0 error 0 warning、ocgcore-tests 18/18 PASS、服务器 mode=0 7911 监听正常、debug.log 无异常。

## 近期变更（2026-08-29）

- **M5 全量验收完成**：M5.11 统一写回、淘汰与胜负、M5.12 四人端到端与双人回归、M5.13 MSG_WIN 淘汰/终局分流均经用户手动四客户端验证通过。唯一存活者获胜、同轮全灭无赢家、尸体侧不写回均符合验收标准；`alive_count < 2` 时直接结算并 EndDuel（不进入 MSG_CHANGE_FIELD，Abort 分支为防御性正确行为）。自动化测试未覆盖配对随机性、双战完成顺序、四客户端流程。
- **M5.13-M5.17 完成（部分）**：`corpse_mask` 全面替换为 `not_corpse`（`field.h:378`）；`merge_field_to_bp` 改 5 参（去 corpse_mask），尸体标记从家园场 `not_corpse[0]` 继承；新增 `set_player_corpse` 运行时标记尸体；`return_field_to_main` 只写家园 `player[0]`、死血写回 23333；`damage`/`recover`/`pay_lp_cost`/`SetLP` 对冻结玩家跳过；`load_home`/`load` 前 `set_active_field` 修复额外卡组 field 归属；`set_active_field` 同步 `Auxiliary.PendulumChecklist` 到 per-field `pendulum_checklist`（灵摆跨 field 隔离，无需改 Lua 卡牌脚本）。编译 0 error、ocgcore-tests 18/18 PASS、四人模式启动正常。`Process()` 的 `stop==3` 合成 MSG_CHANGE_FIELD 已移除，改为尸体 `playerop` 自动回复直到回合自然结束。
- **M2.9 超量素材同步与 `home_origin` 引用完成**：`clone_card_to_field` 遍历 `src->xyz_materials` 克隆素材，设置 `owner`/`home_origin`/`LOCATION_OVERLAY`/`sequence`/`overlay_target`；`card.h` 新增 `card* home_origin{}` 单向引用家园场原卡。编译 0 error、ocgcore-tests 18/18 PASS、四人模式启动正常。素材 `xmaterial_effect` 未克隆（属 M2.1/M2.2）；装备卡同步未实现。
- **M5.6 `TagDuel` `MSG_FIELD_READY` 全量刷新完成**：`tag_duel.cpp:923-1076` 对齐 `single_duel` 同 handler 行为。非战斗场对手数据填 0 只发家园主；战斗场通过 `players[battle_pairs[battle_id][p]]` 映射双方。全量刷新 12 区域 + `query_field_info` + `MSG_REVERSE_DECK` + `send_deck_top`。`home_field_ready` 成员变量已删除（只写不读）。编译 0 error、四人模式启动正常。端到端四客户端视觉验证未执行。
- **战斗场 `MSG_START` 座位反转修复**：`tag_duel.cpp:966-969` / `single_duel.cpp:2085-2087` 用 `turn_player_from_engine` 同时交换 `playertype` 值与发送目标，结果等价于固定座位（P0 恒下/P1 恒上）。先手交替由 `battle_turn_counter` 独立驱动，不受影响。编译 0 error、ocgcore-tests 18/18 PASS、四人模式启动正常。端到端四客户端视觉验证未执行。
- **`playertype` 语义澄清**：`MSG_START` 第 2 字节是座位（0=下/1=上），先手由 `MSG_NEW_TURN` 的 `turn_player` 字节独立表达。两条通道独立，固定座位不破坏先手交替。
- **已知隐患记录**：`processor.cpp` 用 `write_buffer8` 写 LP、gframe 用 `Read<uint8_t>` 读回，LP 截断为 8 位（8000→32），待单独排期。

## 近期变更（2026-08-28）

- **Refresh* 重映射编译修复**：默认实参 `int field = active_field` 非法（非静态成员），改为哨兵值 `-1`。函数体内 `if (field == -1) field = active_field;`。`RefreshSingle` 同理。
- **WriteUpdateData 签名对齐**：头文件声明补 `int field` 参数与 cpp 定义一致（6 参）。
- **BcastObs 战斗场索引越界修复**：`battle_pairs[active_field]`（active_field 为 4/5）改为 `battle_pairs[battle_id]`（`battle_id = active_field - HOME_FIELD_COUNT`）。
- **remap_field_player 边界防御**：新增 `active_field >= FIELD_COUNT || player ∉ {0,1}` 返回 -1。
- **RefreshSzone 死变量清理**：删除 `int pid` 与死注释 `// ReSendToPlayer(players[pid + 1])`。
- **非 SERVER_MODE 分支已废弃**：客户端不由 server 分支编译，已废弃不再维护。
- **四人战旗服务器正确启动参数**：12 参数形式，`mode=2`（`MODE_TAG`）。`ygopro.exe server` 非有效参数。

## 当前主要风险

- **Lua 全局状态跨 field 共享（M2.10）**：`ExtraDeckSummonCountLimit` 等 per-field 语义变量在战斗场克隆后共享，两场战斗互相污染；一次性守卫标记（`MulcharmyGlobalFlag` 等）在战斗场克隆后不重新注册。方案 A 已定待实现。
- **`label_object` 接线已完成（M2.12）**：`remap_effect_references` 在 card/effect/group 全部克隆后统一重映射 `label_object`（CARD/EFFECT/GROUP 三种）、`required_handorset_effects`、`active_handler`、`last_handler`。编译 0 error、ocgcore-tests 20/20 PASS、服务器启动正常。运行时重映射未自动化测试（需 Lua 脚本配合）。
- per-field 计时器未完成，超时状态可能跨 field 覆盖。
- M2.1 效果克隆已完成（2026-08-31），effect 克隆和 temp_card 清理已验收。
- M2.7 永久状态写回边界尚未实现和验证。
- gframe 消息路由与 Refresh 缺少自动化测试。
- 复杂区域重定向和战斗场销毁仍需边界验证。
- `TagDuel` 的 `MSG_FIELD_READY` 全量刷新已完成验收（2026-08-29）；LP 截断隐患待单独排期。
- 双战屏障缺少完成顺序自动化与四客户端验证；三人局尸体战端到端与"不连续匹配尸体"尚未自动化验证。
- `BcastObs` 战斗场索引已修正但运行时广播到达未经端到端（含三人尸体战）验证；非 SERVER_MODE 分支已废弃不再维护。
- `Process()` 的 `stop==3` 合成 MSG_CHANGE_FIELD 已移除，改为尸体 `playerop` 自动回复直到回合自然结束；该接管在全部 `select_*` 场景未自动化验证，依赖用户对 processor 调度的修改。
- 投降路径（`Surrender()`）剩余玩家 ≤2 时整局结束、>2 时淘汰+AI 接管，重入未验证。
- M5 已全量验收（2026-08-29），上述 M5 相关风险为"未自动化测试"性质，不阻塞验收；M2.7 仍为当前入口。