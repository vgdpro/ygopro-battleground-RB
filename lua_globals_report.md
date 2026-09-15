# Lua 全局变量分析报告（仅可变状态）

- 分析文件数: 13417
- 全局变量总数: 79906
- 过滤函数/常量/纯读取: 79552
- 保留可变状态: 354

> 过滤规则：纯函数（值类型全为 function）不需要克隆，战斗时重新映射参数即可；
> 纯常量（值类型全为 number/string 且无函数内赋值）无 per-field 语义；
> 纯读取（无定义无赋值，如方法名误报）一并过滤。

## 汇总

| 类别 | 数量 |
|---|---|
| 顶层全局变量 | 13 |
| Auxiliary.* 字段 | 16 |
| c{code}.* 字段 | 303 |
| 其他表字段 | 22 |

## 顶层全局变量（可变状态）

| 变量 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |
|---|---|---|---|---|---|
| `DOUBLE_DAMAGE` | constant.lua:766(def) | - | 0 | 40 |  |
| `HALF_DAMAGE` | constant.lua:767(def) | - | 0 | 45 |  |
| `NULL_VALUE` | utility.lua:6(def) | - | 0 | 9 |  |
| `Ritual_ReleaseRitualMaterial` | c12157563.lua:23(func) | Duel.ReleaseRitualMaterial | 1 | 1 | 🔶 |
| `_GetFusionMaterial` | c2344618.lua:39(func), c5370235.lua:32(func), c72064891.lua:35(func), c86758746.lua:31(func) | Duel.GetFusionMaterial | 4 | 4 | 🔶 |
| `_ReleaseRitualMaterial` | c20560620.lua:40(func) | Duel.ReleaseRitualMaterial | 1 | 1 | 🔶 |
| `_SendtoGrave` | c2344618.lua:46(func), c5370235.lua:39(func), c72064891.lua:42(func), c86758746.lua:38(func) | Duel.SendtoGrave | 4 | 8 | 🔶 |
| `announce_filter` | c10406322.lua:34(func), c10809984.lua:20(func), c15800838.lua:17(func), c18486927.lua:66(func), c18631392.lua:64(func), c22796548.lua:28(func), c24413299.lua:22(func), c28776350.lua:76(func), c33423043.lua:16(func), c39238953.lua:21(func), c39913299.lua:20(func), c41488249.lua:30(func), c49299410.lua:30(func), c56506740.lua:82(func), c65681983.lua:38(func), c72403299.lua:17(func), c74733322.lua:94(func), c78053598.lua:15(func), c92501449.lua:33(func), c98715423.lua:92(func) | af, table | 20 | 20 | 🔶 |
| `aux` | utility.lua:2(def) | Auxiliary | 0 | 0 |  |
| `filter1` | procedure.lua:1526(func), procedure.lua:1540(func), procedure.lua:1602(func), procedure.lua:1617(func) | params.filter | 4 | 0 | 🔶 |
| `filter2` | procedure.lua:1541(func), procedure.lua:1618(func) | mf | 2 | 0 | 🔶 |
| `get_fcheck` | procedure.lua:1528(func), procedure.lua:1543(func), procedure.lua:1604(func), procedure.lua:1620(func) | params.get_fcheck | 4 | 0 | 🔶 |
| `spsummon_nocheck` | procedure.lua:1529(func), procedure.lua:1605(func) | params.spsummon_nocheck | 2 | 0 | 🔶 |

## Auxiliary.* 字段（可变状态）

| 字段 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |
|---|---|---|---|---|---|
| `Auxiliary.ExtraDeckSummonCountLimit` | utility.lua:484(func), utility.lua:483(guard) | table | 1 | 0 | 🔶✅ |
| `Auxiliary.ExtraDeckSummonCountLimit.[]` | utility.lua:485(func), utility.lua:486(func), utility.lua:494(func), utility.lua:495(func) | number | 4 | 0 | 🔶 |
| `Auxiliary.FCheckAdditional` | procedure.lua:2(top), procedure.lua:1488(func), procedure.lua:1490(func), procedure.lua:1519(func), procedure.lua:1531(func), procedure.lua:1595(func), procedure.lua:1607(func), procedure.lua:1629(func), procedure.lua:1634(func), procedure.lua:1670(func), procedure.lua:1118(guard), procedure.lua:1158(guard), procedure.lua:1191(guard), procedure.lua:1371(guard) | nil, params.fcheck, params.get_fcheck() | 10 | 2 | 🔶✅ |
| `Auxiliary.FGoalCheckAdditional` | procedure.lua:3(top), procedure.lua:1524(func), procedure.lua:1548(func), procedure.lua:1600(func), procedure.lua:1672(func), procedure.lua:1111(guard) | nil, params.fgoalcheck | 5 | 2 | 🔶✅ |
| `Auxiliary.FMaterialBase` | procedure.lua:1481(func), procedure.lua:1518(func), procedure.lua:1547(func), procedure.lua:1594(func), procedure.lua:1673(func) | mgbase, nil | 5 | 0 | 🔶 |
| `Auxiliary.GCheckAdditional` | c18988396.lua:87(func), c18988396.lua:89(func), c18988396.lua:99(func), c18988396.lua:101(func), procedure.lua:571(func), procedure.lua:573(func), procedure.lua:623(func), procedure.lua:625(func), procedure.lua:755(func), procedure.lua:757(func), procedure.lua:786(func), procedure.lua:788(func), procedure.lua:807(func), procedure.lua:809(func), procedure.lua:1520(func), procedure.lua:1522(func), procedure.lua:1532(func), procedure.lua:1596(func), procedure.lua:1598(func), procedure.lua:1608(func), procedure.lua:1630(func), procedure.lua:1632(func), procedure.lua:1671(func), procedure.lua:1837(func), procedure.lua:1839(func), procedure.lua:1890(func), procedure.lua:1892(func), procedure.lua:2082(func), procedure.lua:2084(func), utility.lua:56(top), utility.lua:1198(guard), utility.lua:1209(guard), utility.lua:1237(guard), utility.lua:1238(guard), utility.lua:1319(guard) | Auxiliary.PendOperationCheck(), Auxiliary.RitualCheckAdditional(), Auxiliary.TuneMagicianCheckAdditionalXyz, function, nil, params.gcheck, params.get_gcheck() | 30 | 0 | 🔶✅ |
| `Auxiliary.MulcharmyGlobalFlag` | utility.lua:2043(func), utility.lua:2042(guard) | boolean | 1 | 0 | 🔶✅ |
| `Auxiliary.PendulumChecklist` | procedure.lua:1950(func), procedure.lua:1978(func), procedure.lua:2090(func), procedure.lua:1949(guard), procedure.lua:2008(guard) | Auxiliary.PendulumChecklist, number | 3 | 3 | 🔶✅ |
| `Auxiliary.SubGroupCaptured` | utility.lua:1195(func), utility.lua:1256(func) | Group.CreateGroup(), nil | 2 | 1 | 🔶 |
| `Auxiliary.disfilter1` | utility.lua:762(func) | Auxiliary.NegateAnyFilter | 1 | 0 | 🔶 |
| `Auxiliary.merge_single_effect_codes` | utility.lua:1683(func) | table | 1 | 1 | 🔶 |
| `Auxiliary.merge_single_effect_codes.[]` | utility.lua:1717(func) | g | 1 | 0 | 🔶 |
| `Auxiliary.merge_single_global_check` | utility.lua:1720(func), utility.lua:1719(guard) | boolean | 1 | 0 | 🔶✅ |
| `Auxiliary.quick_effect_filter` | utility.lua:1973(func) | table | 1 | 0 | 🔶 |
| `Auxiliary.quick_effect_filter.[]` | utility.lua:1974(func), utility.lua:1975(func) | Auxiliary.GoldenAllureQueenFilter, Auxiliary.OrcustratedBabelFilter | 2 | 1 | 🔶 |
| `Auxiliary.tdcfop` | procedure.lua:1747(func) | Auxiliary.ContactFusionSendToDeck | 1 | 0 | 🔶 |

## c{code}.* 字段（可变状态）

| 字段 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |
|---|---|---|---|---|---|
| `c10497636.global_check` | c10497636.lua:37(func), c10497636.lua:36(guard) | boolean | 1 | 0 | 🔶✅ |
| `c1050186.star_knight_summon_effect` | c1050186.lua:20(func) | e1 | 1 | 0 | 🔶 |
| `c11678191.old_union` | c11678191.lua:50(top) | boolean | 1 | 0 |  |
| `c12275533.global_check` | c12275533.lua:17(func), c12275533.lua:16(guard) | boolean | 1 | 0 | 🔶✅ |
| `c12289247.global_check` | c12289247.lua:38(func), c12289247.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c12958919.global_check` | c12958919.lua:30(func), c12958919.lua:29(guard) | boolean | 1 | 0 | 🔶✅ |
| `c12965761.old_union` | c12965761.lua:50(top) | boolean | 1 | 0 |  |
| `c13224603.global_check` | c13224603.lua:38(func), c13224603.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c13293158.dark_calling` | c13293158.lua:36(top) | boolean | 1 | 0 |  |
| `c13851202.star_knight_summon_effect` | c13851202.lua:18(func) | e1 | 1 | 0 | 🔶 |
| `c14318794.[]` | c14318794.lua:23(func), c14318794.lua:24(func), c14318794.lua:25(func), c14318794.lua:26(func), c14318794.lua:42(func), c14318794.lua:46(func), c14318794.lua:47(func) | c14318794, number | 7 | 3 | 🔶 |
| `c14318794.global_check` | c14318794.lua:22(func), c14318794.lua:21(guard) | boolean | 1 | 0 | 🔶✅ |
| `c14759024.star_knight_summon_effect` | c14759024.lua:16(func) | e1 | 1 | 0 | 🔶 |
| `c14934922.global_check` | c14934922.lua:15(func), c14934922.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c15871676.star_knight_summon_effect` | c15871676.lua:12(func) | e2 | 1 | 0 | 🔶 |
| `c16051717.treat_itself_tuner` | c16051717.lua:31(top) | boolean | 1 | 0 |  |
| `c16832845.[]` | c16832845.lua:15(func), c16832845.lua:16(func), c16832845.lua:33(func), c16832845.lua:39(func), c16832845.lua:40(func) | boolean | 5 | 2 | 🔶 |
| `c16832845.global_check` | c16832845.lua:14(func), c16832845.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c16906241.star_knight_summon_effect` | c16906241.lua:12(func) | e2 | 1 | 0 | 🔶 |
| `c17994645.treat_itself_tuner` | c17994645.lua:30(top) | boolean | 1 | 0 |  |
| `c1801154.global_check` | c1801154.lua:20(func), c1801154.lua:19(guard) | boolean | 1 | 0 | 🔶✅ |
| `c18114794.[]` | c18114794.lua:18(func), c18114794.lua:19(func), c18114794.lua:40(func), c18114794.lua:41(func), c18114794.lua:42(func), c18114794.lua:54(func) | Duel.GetTurnCount(), c18114794, number | 6 | 1 | 🔶 |
| `c18114794.global_check` | c18114794.lua:17(func), c18114794.lua:16(guard) | boolean | 1 | 0 | 🔶✅ |
| `c18558867.global_check` | c18558867.lua:29(func), c18558867.lua:28(guard) | boolean | 1 | 0 | 🔶✅ |
| `c18969888.global_check` | c18969888.lua:48(func), c18969888.lua:47(guard) | boolean | 1 | 0 | 🔶✅ |
| `c19086954.old_union` | c19086954.lua:50(top) | boolean | 1 | 0 |  |
| `c1966438.global_check` | c1966438.lua:47(func), c1966438.lua:46(guard) | boolean | 1 | 0 | 🔶✅ |
| `c197042.global_check` | c197042.lua:17(func), c197042.lua:16(guard) | boolean | 1 | 0 | 🔶✅ |
| `c19974890.global_check` | c19974890.lua:13(func), c19974890.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c20057949.[]` | c20057949.lua:14(func), c20057949.lua:15(func), c20057949.lua:32(func), c20057949.lua:38(func), c20057949.lua:39(func) | boolean | 5 | 1 | 🔶 |
| `c20057949.global_check` | c20057949.lua:13(func), c20057949.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c20318029.discard_effect` | c20318029.lua:18(func) | e1 | 1 | 0 | 🔶 |
| `c20501450.[]` | c20501450.lua:17(func), c20501450.lua:18(func), c20501450.lua:34(func), c20501450.lua:39(func), c20501450.lua:40(func) | c20501450, number | 5 | 3 | 🔶 |
| `c20501450.global_check` | c20501450.lua:16(func), c20501450.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c20788863.global_check` | c20788863.lua:37(func), c20788863.lua:36(guard) | boolean | 1 | 0 | 🔶✅ |
| `c20799347.treat_itself_tuner` | c20799347.lua:24(top) | boolean | 1 | 0 |  |
| `c20822520.global_check` | c20822520.lua:16(func), c20822520.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c21123811.cosmic_quasar_dragon_summon` | c21123811.lua:59(top) | boolean | 1 | 0 |  |
| `c21364070.global_check` | c21364070.lua:73(func), c21364070.lua:72(guard) | boolean | 1 | 0 | 🔶✅ |
| `c21862633.global_check` | c21862633.lua:36(func), c21862633.lua:35(guard) | boolean | 1 | 0 | 🔶✅ |
| `c21947653.dark_calling` | c21947653.lua:27(top) | boolean | 1 | 0 |  |
| `c22125101.global_check` | c22125101.lua:32(func), c22125101.lua:31(guard) | boolean | 1 | 0 | 🔶✅ |
| `c22160245.dark_calling` | c22160245.lua:32(top) | boolean | 1 | 0 |  |
| `c22617205.star_knight_summon_effect` | c22617205.lua:31(func) | e3 | 1 | 0 | 🔶 |
| `c2273734.star_knight_summon_effect` | c2273734.lua:20(func) | e1 | 1 | 0 | 🔶 |
| `c22804644.[]` | c22804644.lua:56(func) | e2 | 1 | 0 | 🔶 |
| `c23338098.treat_itself_tuner` | c23338098.lua:40(top) | boolean | 1 | 0 |  |
| `c23931679.[]` | c23931679.lua:49(top), c23931679.lua:50(top), c23931679.lua:84(func), c23931679.lua:85(func), c23931679.lua:93(func), c23931679.lua:102(func), c23931679.lua:104(func), c23931679.lua:113(func) | number | 8 | 1 | 🔶 |
| `c23931679.global_check` | c23931679.lua:39(func), c23931679.lua:38(guard) | boolean | 1 | 0 | 🔶✅ |
| `c24207889.[]` | c24207889.lua:35(func), c24207889.lua:36(func) | table | 2 | 0 | 🔶 |
| `c24207889.[].[]` | c24207889.lua:39(func), c24207889.lua:41(func) | Group.CreateGroup() | 2 | 0 | 🔶 |
| `c24207889.global_check` | c24207889.lua:33(func), c24207889.lua:32(guard) | boolean | 1 | 0 | 🔶✅ |
| `c24207889.is_empty` | c24207889.lua:34(func), c24207889.lua:73(func), c24207889.lua:77(func), c24207889.lua:66(guard) | boolean | 3 | 0 | 🔶✅ |
| `c24635329.shadoll_flip_effect` | c24635329.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c2547033.global_check` | c2547033.lua:33(func), c2547033.lua:32(guard) | boolean | 1 | 0 | 🔶✅ |
| `c26057276.star_knight_summon_effect` | c26057276.lua:20(func) | e1 | 1 | 0 | 🔶 |
| `c26268488.cosmic_quasar_dragon_summon` | c26268488.lua:47(top) | boolean | 1 | 0 |  |
| `c26285788.global_check` | c26285788.lua:13(func), c26285788.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c26400609.Dragon_Ruler_handes_effect` | c26400609.lua:48(func) | e3 | 1 | 0 | 🔶 |
| `c26773909.global_check` | c26773909.lua:16(func), c26773909.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c27103517.treat_itself_tuner` | c27103517.lua:22(top) | boolean | 1 | 0 |  |
| `c27204311.global_check` | c27204311.lua:17(func), c27204311.lua:16(guard) | boolean | 1 | 0 | 🔶✅ |
| `c27769400.[]` | c27769400.lua:26(func), c27769400.lua:28(func), c27769400.lua:44(func) | Group.CreateGroup(), c27769400, number | 3 | 0 | 🔶 |
| `c27769400.global_check` | c27769400.lua:25(func), c27769400.lua:24(guard) | boolean | 1 | 0 | 🔶✅ |
| `c27770341.[]` | c27770341.lua:13(func), c27770341.lua:14(func), c27770341.lua:33(func), c27770341.lua:34(func), c27770341.lua:42(func) | c27770341, number | 5 | 2 | 🔶 |
| `c27770341.counter` | c27770341.lua:12(func), c27770341.lua:11(guard) | boolean | 1 | 0 | 🔶✅ |
| `c29307554.global_check` | c29307554.lua:13(func), c29307554.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c29596581.discard_effect` | c29596581.lua:14(func) | e1 | 1 | 0 | 🔶 |
| `c29724053.[]` | c29724053.lua:27(func), c29724053.lua:28(func), c29724053.lua:45(func), c29724053.lua:46(func), c29724053.lua:53(func) | c29724053, number | 5 | 24 | 🔶 |
| `c29724053.global_check` | c29724053.lua:26(func), c29724053.lua:25(guard) | boolean | 1 | 0 | 🔶✅ |
| `c29948294.global_check` | c29948294.lua:27(func), c29948294.lua:26(guard) | boolean | 1 | 0 | 🔶✅ |
| `c30328508.shadoll_flip_effect` | c30328508.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c30353551.[]` | c30353551.lua:38(func), c30353551.lua:41(func), c30353551.lua:45(func), c30353551.lua:46(func) | c30353551, number | 4 | 4 | 🔶 |
| `c30353551.global_check` | c30353551.lua:21(func), c30353551.lua:20(guard) | boolean | 1 | 0 | 🔶✅ |
| `c30537973.global_check` | c30537973.lua:38(func), c30537973.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c30765615.global_check` | c30765615.lua:25(func), c30765615.lua:24(guard) | boolean | 1 | 0 | 🔶✅ |
| `c31458630.fusion_effect` | c31458630.lua:26(top) | boolean | 1 | 0 |  |
| `c31472884.global_check` | c31472884.lua:16(func), c31472884.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c31699677.global_check` | c31699677.lua:32(func), c31699677.lua:31(guard) | boolean | 1 | 0 | 🔶✅ |
| `c31768112.old_union` | c31768112.lua:51(top) | boolean | 1 | 0 |  |
| `c31786629.discard_effect` | c31786629.lua:13(func) | e1 | 1 | 0 | 🔶 |
| `c32247099.global_check` | c32247099.lua:35(func), c32247099.lua:34(guard) | boolean | 1 | 0 | 🔶✅ |
| `c32825095.treat_itself_tuner` | c32825095.lua:19(top) | boolean | 1 | 0 |  |
| `c33103459.global_check` | c33103459.lua:28(func), c33103459.lua:27(guard) | boolean | 1 | 0 | 🔶✅ |
| `c33327029.global_check` | c33327029.lua:25(func), c33327029.lua:24(guard) | boolean | 1 | 0 | 🔶✅ |
| `c33545259.global_check` | c33545259.lua:43(func), c33545259.lua:42(guard) | boolean | 1 | 0 | 🔶✅ |
| `c33776734.global_check` | c33776734.lua:38(func), c33776734.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c34620088.global_check` | c34620088.lua:22(func), c34620088.lua:21(guard) | boolean | 1 | 0 | 🔶✅ |
| `c34800281.global_check` | c34800281.lua:30(func), c34800281.lua:29(guard) | boolean | 1 | 0 | 🔶✅ |
| `c35027493.[]` | c35027493.lua:58(func) | e2 | 1 | 0 | 🔶 |
| `c35268887.[]` | c35268887.lua:11(func), c35268887.lua:12(func), c35268887.lua:27(func), c35268887.lua:31(func), c35268887.lua:32(func) | c35268887, number | 5 | 3 | 🔶 |
| `c35268887.global_check` | c35268887.lua:10(func), c35268887.lua:9(guard) | boolean | 1 | 0 | 🔶✅ |
| `c35756798.global_check` | c35756798.lua:15(func), c35756798.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c3576031.[]` | c3576031.lua:35(func), c3576031.lua:36(func), c3576031.lua:54(func), c3576031.lua:60(func), c3576031.lua:61(func) | c3576031, number | 5 | 5 | 🔶 |
| `c3576031.global_check` | c3576031.lua:34(func), c3576031.lua:33(guard) | boolean | 1 | 0 | 🔶✅ |
| `c35952884.cosmic_quasar_dragon_summon` | c35952884.lua:45(top) | boolean | 1 | 0 |  |
| `c36458063.global_check` | c36458063.lua:20(func), c36458063.lua:19(guard) | boolean | 1 | 0 | 🔶✅ |
| `c3717252.shadoll_flip_effect` | c3717252.lua:27(func) | e1 | 1 | 0 | 🔶 |
| `c37241623.[]` | c37241623.lua:15(func), c37241623.lua:26(func) | Duel.GetChainInfo(), nil | 2 | 1 | 🔶 |
| `c37241623.global_check` | c37241623.lua:14(func), c37241623.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c37445295.shadoll_flip_effect` | c37445295.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c38049934.check` | c38049934.lua:14(top), c38049934.lua:19(func), c38049934.lua:28(func), c38049934.lua:31(func), c38049934.lua:27(guard) | boolean | 4 | 0 | 🔶✅ |
| `c38318146.[]` | c38318146.lua:28(func), c38318146.lua:45(func) | boolean | 2 | 0 | 🔶 |
| `c38667773.star_knight_summon_effect` | c38667773.lua:20(func) | e1 | 1 | 0 | 🔶 |
| `c38783169.treat_itself_tuner` | c38783169.lua:15(top) | boolean | 1 | 0 |  |
| `c3972721.global_check` | c3972721.lua:15(func), c3972721.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c39853199.global_check` | c39853199.lua:36(func), c39853199.lua:35(guard) | boolean | 1 | 0 | 🔶✅ |
| `c40143123.star_knight_summon_effect` | c40143123.lua:12(func) | e2 | 1 | 0 | 🔶 |
| `c4064925.global_check` | c4064925.lua:26(func), c4064925.lua:25(guard) | boolean | 1 | 0 | 🔶✅ |
| `c40737112.global_check` | c40737112.lua:29(func), c40737112.lua:28(guard) | boolean | 1 | 0 | 🔶✅ |
| `c41269771.star_knight_summon_effect` | c41269771.lua:12(func) | e2 | 1 | 0 | 🔶 |
| `c41850466.global_check` | c41850466.lua:15(func), c41850466.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c42228966.global_check` | c42228966.lua:30(func), c42228966.lua:29(guard) | boolean | 1 | 0 | 🔶✅ |
| `c42391240.star_knight_summon_effect` | c42391240.lua:16(func) | e1 | 1 | 0 | 🔶 |
| `c42589641.global_check` | c42589641.lua:54(func), c42589641.lua:53(guard) | boolean | 1 | 0 | 🔶✅ |
| `c42822433.star_knight_summon_effect` | c42822433.lua:19(func) | e1 | 1 | 0 | 🔶 |
| `c43383478.globle_check` | c43383478.lua:15(func), c43383478.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c43513897.star_knight_summon_effect` | c43513897.lua:14(func) | e1 | 1 | 0 | 🔶 |
| `c44190146.[]` | c44190146.lua:45(func), c44190146.lua:46(func), c44190146.lua:61(func), c44190146.lua:71(func), c44190146.lua:72(func) | table | 5 | 0 | 🔶 |
| `c44190146.[].[]` | c44190146.lua:57(func), c44190146.lua:66(func) | rc | 2 | 2 | 🔶 |
| `c44190146.global_check` | c44190146.lua:29(func), c44190146.lua:28(guard) | boolean | 1 | 0 | 🔶✅ |
| `c47228077.old_union` | c47228077.lua:51(top) | boolean | 1 | 0 |  |
| `c47415292.old_union` | c47415292.lua:55(top) | boolean | 1 | 0 |  |
| `c47459126.treat_itself_tuner` | c47459126.lua:13(top) | boolean | 1 | 0 |  |
| `c47693640.old_union` | c47693640.lua:64(top) | boolean | 1 | 0 |  |
| `c48568432.old_union` | c48568432.lua:51(top) | boolean | 1 | 0 |  |
| `c49249907.global_check` | c49249907.lua:24(func), c49249907.lua:23(guard) | boolean | 1 | 0 | 🔶✅ |
| `c49296203.treat_itself_tuner` | c49296203.lua:28(top) | boolean | 1 | 0 |  |
| `c4931121.[]` | c4931121.lua:58(func) | e2 | 1 | 0 | 🔶 |
| `c4939890.shadoll_flip_effect` | c4939890.lua:27(func) | e1 | 1 | 0 | 🔶 |
| `c49430782.counter` | c49430782.lua:43(func), c49430782.lua:64(func), c49430782.lua:78(func), c49430782.lua:42(guard) | c49430782.counter, number | 3 | 5 | 🔶✅ |
| `c49587034.[]` | c49587034.lua:36(func) | e1 | 1 | 0 | 🔶 |
| `c49930315.treat_itself_tuner` | c49930315.lua:25(top) | boolean | 1 | 0 |  |
| `c49980185.[]` | c49980185.lua:16(func), c49980185.lua:30(func), c49980185.lua:33(func) | c49980185, number | 3 | 5 | 🔶 |
| `c49980185.global_check` | c49980185.lua:15(func), c49980185.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c50282757.dark_calling` | c50282757.lua:37(top) | boolean | 1 | 0 |  |
| `c51023024.shadoll_flip_effect` | c51023024.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c51194046.[]` | c51194046.lua:29(func), c51194046.lua:30(func), c51194046.lua:61(func), c51194046.lua:69(func), c51194046.lua:70(func) | c51194046, number | 5 | 5 | 🔶 |
| `c51194046.global_check` | c51194046.lua:28(func), c51194046.lua:27(guard) | boolean | 1 | 0 | 🔶✅ |
| `c51339637.global_check` | c51339637.lua:28(func), c51339637.lua:27(guard) | boolean | 1 | 0 | 🔶✅ |
| `c52155219.global_check` | c52155219.lua:13(func), c52155219.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c52445243.treat_itself_tuner` | c52445243.lua:24(top) | boolean | 1 | 0 |  |
| `c52551211.shadoll_flip_effect` | c52551211.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c52900379.[]` | c52900379.lua:24(func), c52900379.lua:25(func), c52900379.lua:46(func), c52900379.lua:52(func), c52900379.lua:53(func) | c52900379, number | 5 | 2 | 🔶 |
| `c52900379.global_check` | c52900379.lua:23(func), c52900379.lua:22(guard) | boolean | 1 | 0 | 🔶✅ |
| `c53039326.global_check` | c53039326.lua:31(func), c53039326.lua:30(guard) | boolean | 1 | 0 | 🔶✅ |
| `c53251824.global_check` | c53251824.lua:14(func), c53251824.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c53334471.[]` | c53334471.lua:44(top), c53334471.lua:45(top), c53334471.lua:48(func), c53334471.lua:49(func), c53334471.lua:85(func), c53334471.lua:100(func), c53334471.lua:102(func), c53334471.lua:117(func) | att, number | 8 | 2 | 🔶 |
| `c53389254.treat_itself_tuner` | c53389254.lua:45(top) | boolean | 1 | 0 |  |
| `c53541822.fusion_effect` | c53541822.lua:31(top) | boolean | 1 | 0 |  |
| `c53804307.Dragon_Ruler_handes_effect` | c53804307.lua:49(func) | e3 | 1 | 0 | 🔶 |
| `c54109233.global_check` | c54109233.lua:21(func), c54109233.lua:20(guard) | boolean | 1 | 0 | 🔶✅ |
| `c54564198.globle_check` | c54564198.lua:16(func) | boolean | 1 | 0 | 🔶 |
| `c54974237.[]` | c54974237.lua:66(func) | e2 | 1 | 0 | 🔶 |
| `c55273560.global_check` | c55273560.lua:40(func), c55273560.lua:39(guard) | boolean | 1 | 0 | 🔶✅ |
| `c55795155.global_check` | c55795155.lua:42(func), c55795155.lua:41(guard) | boolean | 1 | 0 | 🔶✅ |
| `c5614808.treat_itself_tuner` | c5614808.lua:42(top) | boolean | 1 | 0 |  |
| `c56713174.discard_effect` | c56713174.lua:16(func) | e1 | 1 | 0 | 🔶 |
| `c56824871.treat_itself_tuner` | c56824871.lua:25(top) | boolean | 1 | 0 |  |
| `c57062206.old_union` | c57062206.lua:46(top) | boolean | 1 | 0 |  |
| `c57995165.global_check` | c57995165.lua:14(func), c57995165.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c58062306.treat_itself_tuner` | c58062306.lua:25(top) | boolean | 1 | 0 |  |
| `c58092907.global_check` | c58092907.lua:30(func), c58092907.lua:29(guard) | boolean | 1 | 0 | 🔶✅ |
| `c58132856.set_as_spell` | c58132856.lua:36(top) | boolean | 1 | 0 |  |
| `c58242947.[]` | c58242947.lua:25(func) | te | 1 | 2 | 🔶 |
| `c58332301.dark_calling` | c58332301.lua:30(top) | boolean | 1 | 0 |  |
| `c58775978.[]` | c58775978.lua:38(func) | e1 | 1 | 0 | 🔶 |
| `c59160188.re_activated` | c15717011.lua:46(func), c15717011.lua:49(func), c15717011.lua:55(func), c15717011.lua:59(func), c52101615.lua:46(func), c52101615.lua:49(func), c52101615.lua:55(func), c52101615.lua:59(func), c88696724.lua:46(func), c88696724.lua:49(func), c88696724.lua:55(func), c88696724.lua:59(func) | boolean | 12 | 1 | 🔶 |
| `c59364406.old_union` | c59364406.lua:51(top) | boolean | 1 | 0 |  |
| `c59419719.global_check` | c59419719.lua:26(func), c59419719.lua:25(guard) | boolean | 1 | 0 | 🔶✅ |
| `c59695933.global_check` | c59695933.lua:14(func), c59695933.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c59957503.[]` | c59957503.lua:14(func), c59957503.lua:25(func) | Duel.GetChainInfo(), nil | 2 | 1 | 🔶 |
| `c59957503.global_check` | c59957503.lua:13(func), c59957503.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c60187739.[]` | c60187739.lua:24(func), c60187739.lua:25(func), c60187739.lua:39(func), c60187739.lua:42(func), c60187739.lua:43(func) | boolean | 5 | 1 | 🔶 |
| `c60187739.global_check` | c60187739.lua:23(func), c60187739.lua:22(guard) | boolean | 1 | 0 | 🔶✅ |
| `c60406591.[]` | c60406591.lua:14(func), c60406591.lua:15(func), c60406591.lua:16(func), c60406591.lua:30(func), c60406591.lua:31(func), c60406591.lua:32(func), c60406591.lua:35(func), c60406591.lua:36(func), c60406591.lua:37(func) | eg, ep, math.floor(), nil | 9 | 4 | 🔶 |
| `c60406591.global_check` | c60406591.lua:13(func), c60406591.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c60621361.global_check` | c60621361.lua:36(func), c60621361.lua:35(guard) | boolean | 1 | 0 | 🔶✅ |
| `c60930169.global_check` | c60930169.lua:13(func), c60930169.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c60950180.global_check` | c60950180.lua:28(func), c60950180.lua:27(guard) | boolean | 1 | 0 | 🔶✅ |
| `c62161698.global_check` | c62161698.lua:15(func), c62161698.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c62370023.global_check` | c62370023.lua:16(func), c62370023.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c63274863.star_knight_summon_effect` | c63274863.lua:20(func) | e1 | 1 | 0 | 🔶 |
| `c63676256.old_union` | c63676256.lua:60(top) | boolean | 1 | 0 |  |
| `c63731062.treat_itself_tuner` | c63731062.lua:31(top) | boolean | 1 | 0 |  |
| `c64178424.global_check` | c64178424.lua:31(func), c64178424.lua:30(guard) | boolean | 1 | 0 | 🔶✅ |
| `c65056481.star_knight_summon_effect` | c65056481.lua:20(func) | e1 | 1 | 0 | 🔶 |
| `c65398390.treat_itself_tuner` | c65398390.lua:30(top) | boolean | 1 | 0 |  |
| `c65685470.old_union` | c65685470.lua:65(top) | boolean | 1 | 0 |  |
| `c66078354.global_check` | c66078354.lua:32(func), c66078354.lua:31(guard) | boolean | 1 | 0 | 🔶✅ |
| `c66150724.global_check` | c66150724.lua:26(func), c66150724.lua:25(guard) | boolean | 1 | 0 | 🔶✅ |
| `c66675911.shadoll_flip_effect` | c66675911.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c67045174.global_check` | c67045174.lua:13(func), c67045174.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c67100549.global_check` | c67100549.lua:37(func), c67100549.lua:36(guard) | boolean | 1 | 0 | 🔶✅ |
| `c67630339.[]` | c67630339.lua:12(func), c67630339.lua:13(func), c67630339.lua:34(func), c67630339.lua:37(func), c67630339.lua:44(func), c67630339.lua:48(func), c67630339.lua:49(func) | Duel.GetAttacker(), c67630339, number | 7 | 5 | 🔶 |
| `c67630339.global_check` | c67630339.lua:11(func), c67630339.lua:10(guard) | boolean | 1 | 0 | 🔶✅ |
| `c67712104.global_check` | c67712104.lua:38(func), c67712104.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c67901914.global_check` | c67901914.lua:16(func), c67901914.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c6850209.[]` | c6850209.lua:13(func), c6850209.lua:14(func), c6850209.lua:31(func), c6850209.lua:37(func), c6850209.lua:38(func) | boolean | 5 | 1 | 🔶 |
| `c6850209.global_check` | c6850209.lua:12(func), c6850209.lua:11(guard) | boolean | 1 | 0 | 🔶✅ |
| `c6909330.global_check` | c6909330.lua:27(func), c6909330.lua:26(guard) | boolean | 1 | 0 | 🔶✅ |
| `c69145169.global_check` | c69145169.lua:16(func), c69145169.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c69456283.old_union` | c69456283.lua:52(top) | boolean | 1 | 0 |  |
| `c69811710.treat_itself_tuner` | c69811710.lua:29(top) | boolean | 1 | 0 |  |
| `c70335319.[]` | c70335319.lua:61(func), c70335319.lua:62(func), c70335319.lua:78(func), c70335319.lua:81(func), c70335319.lua:84(func), c70335319.lua:85(func), c70335319.lua:86(func) | at, nil, number | 7 | 1 | 🔶 |
| `c70335319.global_check` | c70335319.lua:60(func), c70335319.lua:59(guard) | boolean | 1 | 0 | 🔶✅ |
| `c70456282.treat_itself_tuner` | c70456282.lua:23(top) | boolean | 1 | 0 |  |
| `c71095768.[]` | c71095768.lua:41(func), c71095768.lua:42(func), c71095768.lua:43(func), c71095768.lua:54(func), c71095768.lua:55(func), c71095768.lua:64(func) | Duel.GetChainInfo(), nil, seq | 6 | 1 | 🔶 |
| `c71095768.global_check` | c71095768.lua:40(func), c71095768.lua:39(guard) | boolean | 1 | 0 | 🔶✅ |
| `c71386411.[]` | c71386411.lua:29(func), c71386411.lua:30(func), c71386411.lua:47(func), c71386411.lua:51(func), c71386411.lua:52(func) | boolean | 5 | 1 | 🔶 |
| `c71386411.global_check` | c71386411.lua:28(func), c71386411.lua:27(guard) | boolean | 1 | 0 | 🔶✅ |
| `c71612253.global_check` | c71612253.lua:33(func), c71612253.lua:32(guard) | boolean | 1 | 0 | 🔶✅ |
| `c71645242.global_check` | c71645242.lua:30(func), c71645242.lua:29(guard) | boolean | 1 | 0 | 🔶✅ |
| `c72302403.[]` | c72302403.lua:43(func) | e1 | 1 | 0 | 🔶 |
| `c72554862.global_check` | c72554862.lua:25(func), c72554862.lua:24(guard) | boolean | 1 | 0 | 🔶✅ |
| `c73193552.global_check` | c73193552.lua:43(func), c73193552.lua:42(guard) | boolean | 1 | 0 | 🔶✅ |
| `c7369217.old_union` | c7369217.lua:46(top) | boolean | 1 | 0 |  |
| `c75782277.global_check` | c75782277.lua:33(func), c75782277.lua:32(guard) | boolean | 1 | 0 | 🔶✅ |
| `c75878039.star_knight_summon_effect` | c75878039.lua:20(func) | e1 | 1 | 0 | 🔶 |
| `c75906310.global_flag` | c75906310.lua:38(func), c75906310.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c76224717.[]` | c76224717.lua:21(func), c76224717.lua:22(func), c76224717.lua:43(func), c76224717.lua:47(func), c76224717.lua:48(func) | c76224717, number | 5 | 4 | 🔶 |
| `c76224717.global_check` | c76224717.lua:20(func), c76224717.lua:19(guard) | boolean | 1 | 0 | 🔶✅ |
| `c76552147.global_check` | c76552147.lua:15(func), c76552147.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c76794549.global_check` | c76794549.lua:38(func), c76794549.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c7714344.global_check` | c7714344.lua:25(func), c7714344.lua:24(guard) | boolean | 1 | 0 | 🔶✅ |
| `c77723643.shadoll_flip_effect` | c77723643.lua:27(func) | e1 | 1 | 0 | 🔶 |
| `c78229193.treat_itself_tuner` | c78229193.lua:30(top) | boolean | 1 | 0 |  |
| `c78486968.star_knight_summon_effect` | c78486968.lua:12(func) | e1 | 1 | 0 | 🔶 |
| `c79210531.star_knight_summon_effect` | c79210531.lua:31(func) | e2 | 1 | 0 | 🔶 |
| `c80402389.global_check` | c80402389.lua:20(func), c80402389.lua:19(guard) | boolean | 1 | 0 | 🔶✅ |
| `c80773359.treat_itself_tuner` | c80773359.lua:39(top) | boolean | 1 | 0 |  |
| `c80949182.[]` | c80949182.lua:52(func), c80949182.lua:53(func), c80949182.lua:123(func) | -, Duel.GetTurnCount() | 3 | 1 | 🔶 |
| `c80949182.global_check` | c80949182.lua:51(func), c80949182.lua:50(guard) | boolean | 1 | 0 | 🔶✅ |
| `c81167171.[]` | c81167171.lua:13(func), c81167171.lua:14(func), c81167171.lua:31(func), c81167171.lua:37(func), c81167171.lua:38(func) | boolean | 5 | 1 | 🔶 |
| `c81167171.global_check` | c81167171.lua:12(func), c81167171.lua:11(guard) | boolean | 1 | 0 | 🔶✅ |
| `c81193865.[]` | c81193865.lua:42(func), c81193865.lua:43(func), c81193865.lua:59(func), c81193865.lua:64(func), c81193865.lua:65(func) | c81193865, number | 5 | 2 | 🔶 |
| `c81193865.global_check` | c81193865.lua:41(func), c81193865.lua:40(guard) | boolean | 1 | 0 | 🔶✅ |
| `c81275309.[]` | c81275309.lua:11(top), c81275309.lua:15(func) | id, number | 2 | 0 | 🔶 |
| `c8129306.global_check` | c8129306.lua:27(func), c8129306.lua:26(guard) | boolean | 1 | 0 | 🔶✅ |
| `c82052602.[]` | c82052602.lua:15(func), c82052602.lua:16(func), c82052602.lua:34(func), c82052602.lua:40(func), c82052602.lua:41(func) | boolean | 5 | 1 | 🔶 |
| `c82052602.global_check` | c82052602.lua:14(func), c82052602.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c82570174.global_check` | c82570174.lua:32(func), c82570174.lua:31(guard) | boolean | 1 | 0 | 🔶✅ |
| `c82670878.[]` | c82670878.lua:25(func), c82670878.lua:26(func), c82670878.lua:47(func), c82670878.lua:49(func), c82670878.lua:59(func), c82670878.lua:63(func), c82670878.lua:64(func) | c82670878, number, tc | 7 | 5 | 🔶 |
| `c82670878.global_check` | c82670878.lua:24(func), c82670878.lua:23(guard) | boolean | 1 | 0 | 🔶✅ |
| `c82760689.[]` | c82760689.lua:12(func), c82760689.lua:13(func), c82760689.lua:41(func), c82760689.lua:48(func) | Group.CreateGroup(), boolean | 4 | 2 | 🔶 |
| `c82760689.global_check` | c82760689.lua:11(func), c82760689.lua:10(guard) | boolean | 1 | 0 | 🔶✅ |
| `c83107873.discard_effect` | c83107873.lua:14(func) | e1 | 1 | 0 | 🔶 |
| `c83236601.global_check` | c83236601.lua:32(func), c83236601.lua:31(guard) | boolean | 1 | 0 | 🔶✅ |
| `c83725008.[]` | c83725008.lua:17(func), c83725008.lua:18(func), c83725008.lua:32(func), c83725008.lua:33(func), c83725008.lua:42(func) | boolean | 5 | 1 | 🔶 |
| `c83725008.discard` | c83725008.lua:16(func), c83725008.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c83957459.[]` | c83957459.lua:12(func), c83957459.lua:13(func), c83957459.lua:30(func), c83957459.lua:33(func), c83957459.lua:34(func) | c83957459, number | 5 | 2 | 🔶 |
| `c83957459.global_check` | c83957459.lua:11(func), c83957459.lua:10(guard) | boolean | 1 | 0 | 🔶✅ |
| `c84211599.gf` | c84211599.lua:14(func), c84211599.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c84274024.global_check` | c84274024.lua:38(func), c84274024.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c84313685.old_union` | c84313685.lua:51(top) | boolean | 1 | 0 |  |
| `c84339249.global_check` | c84339249.lua:38(func), c84339249.lua:37(guard) | boolean | 1 | 0 | 🔶✅ |
| `c84814897.old_union` | c84814897.lua:62(top) | boolean | 1 | 0 |  |
| `c85359414.old_union` | c85359414.lua:51(top) | boolean | 1 | 0 |  |
| `c85360035.global_check` | c85360035.lua:27(func), c85360035.lua:26(guard) | boolean | 1 | 0 | 🔶✅ |
| `c85555787.[]` | c85555787.lua:69(func) | e2 | 1 | 0 | 🔶 |
| `c85602018.global_check` | c85602018.lua:11(func), c85602018.lua:10(guard) | boolean | 1 | 0 | 🔶✅ |
| `c85862791.global_check` | c85862791.lua:15(func), c85862791.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c86165817.dark_calling` | c86165817.lua:36(top) | boolean | 1 | 0 |  |
| `c86196216.globle_check` | c86196216.lua:14(func), c86196216.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c86277379.global_check` | c86277379.lua:31(func), c86277379.lua:30(guard) | boolean | 1 | 0 | 🔶✅ |
| `c86466163.star_knight_summon_effect` | c86466163.lua:19(func) | e1 | 1 | 0 | 🔶 |
| `c86541496.global_check` | c86541496.lua:13(func), c86541496.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c86676862.dark_calling` | c86676862.lua:38(top) | boolean | 1 | 0 |  |
| `c8700633.global_check` | c8700633.lua:17(func), c8700633.lua:16(guard) | boolean | 1 | 0 | 🔶✅ |
| `c8736823.treat_itself_tuner` | c8736823.lua:34(top) | boolean | 1 | 0 |  |
| `c87564935.old_union` | c87564935.lua:53(top) | boolean | 1 | 0 |  |
| `c87798440.old_union` | c87798440.lua:51(top) | boolean | 1 | 0 |  |
| `c8785161.global_check` | c8785161.lua:14(func), c8785161.lua:13(guard) | boolean | 1 | 0 | 🔶✅ |
| `c88513608.global_check` | c88513608.lua:15(func), c88513608.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c88540324.global_check` | c88540324.lua:41(func), c88540324.lua:40(guard) | boolean | 1 | 0 | 🔶✅ |
| `c88851326.global_check` | c88851326.lua:54(func), c88851326.lua:53(guard) | boolean | 1 | 0 | 🔶✅ |
| `c89399912.Dragon_Ruler_handes_effect` | c89399912.lua:48(func) | e3 | 1 | 0 | 🔶 |
| `c89792713.global_check` | c89792713.lua:13(func), c89792713.lua:12(guard) | boolean | 1 | 0 | 🔶✅ |
| `c90411554.Dragon_Ruler_handes_effect` | c90411554.lua:49(func) | e3 | 1 | 0 | 🔶 |
| `c90448279.global_check` | c90448279.lua:31(func), c90448279.lua:30(guard) | boolean | 1 | 0 | 🔶✅ |
| `c90846359.[]` | c90846359.lua:44(top), c90846359.lua:45(top), c90846359.lua:48(func), c90846359.lua:49(func), c90846359.lua:85(func), c90846359.lua:100(func), c90846359.lua:102(func), c90846359.lua:117(func) | number, rac | 8 | 2 | 🔶 |
| `c91269402.global_check` | c91269402.lua:15(func), c91269402.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c91392974.global_check` | c91392974.lua:16(func), c91392974.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c92079625.shadoll_flip_effect` | c92079625.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c93211810.[]` | c93211810.lua:17(func), c93211810.lua:18(func), c93211810.lua:34(func), c93211810.lua:39(func), c93211810.lua:40(func) | c93211810, number | 5 | 3 | 🔶 |
| `c93211810.global_check` | c93211810.lua:16(func), c93211810.lua:15(guard) | boolean | 1 | 0 | 🔶✅ |
| `c93238626.global_check` | c93238626.lua:34(func), c93238626.lua:33(guard) | boolean | 1 | 0 | 🔶✅ |
| `c93368494.global_check` | c93368494.lua:54(func), c93368494.lua:53(guard) | boolean | 1 | 0 | 🔶✅ |
| `c9348522.global_check` | c9348522.lua:26(func), c9348522.lua:25(guard) | boolean | 1 | 0 | 🔶✅ |
| `c94145021.global_check` | c94145021.lua:15(func), c94145021.lua:14(guard) | boolean | 1 | 0 | 🔶✅ |
| `c94303232.global_check` | c94303232.lua:41(func), c94303232.lua:40(guard) | boolean | 1 | 0 | 🔶✅ |
| `c94585852.global_check` | c94585852.lua:29(func), c94585852.lua:28(guard) | boolean | 1 | 0 | 🔶✅ |
| `c95134948.global_check` | c95134948.lua:31(func), c95134948.lua:30(guard) | boolean | 1 | 0 | 🔶✅ |
| `c95308449.[]` | c95308449.lua:28(func) | e1 | 1 | 0 | 🔶 |
| `c95448692.[]` | c95448692.lua:18(top), c95448692.lua:19(top), c95448692.lua:21(func), c95448692.lua:35(func) | number | 4 | 0 | 🔶 |
| `c96223501.star_knight_summon_effect` | c96223501.lua:31(func) | e3 | 1 | 0 | 🔶 |
| `c96345188.global_check` | c96345188.lua:24(func), c96345188.lua:23(guard) | boolean | 1 | 0 | 🔶✅ |
| `c97403510.global_check` | c97403510.lua:51(func), c97403510.lua:50(guard) | boolean | 1 | 0 | 🔶✅ |
| `c97518132.shadoll_flip_effect` | c97518132.lua:25(func) | e1 | 1 | 0 | 🔶 |
| `c98864751.[]` | c98864751.lua:32(func), c98864751.lua:33(func), c98864751.lua:49(func), c98864751.lua:54(func), c98864751.lua:55(func) | c98864751, number | 5 | 2 | 🔶 |
| `c98864751.global_check` | c98864751.lua:31(func), c98864751.lua:30(guard) | boolean | 1 | 0 | 🔶✅ |
| `c99668578.star_knight_summon_effect` | c99668578.lua:20(func) | e3 | 1 | 0 | 🔶 |
| `c99748883.global_check` | c99748883.lua:27(func), c99748883.lua:26(guard) | boolean | 1 | 0 | 🔶✅ |

## 其他表字段（可变状态）

| 字段 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |
|---|---|---|---|---|---|
| `aux.ExtraDeckSummonCountLimit.[]` | c23812568.lua:96(func), c23812568.lua:99(func), c38783169.lua:60(func), c38783169.lua:63(func), c4606229.lua:109(func), c4606229.lua:112(func), c50073633.lua:97(func), c50073633.lua:100(func), c92345028.lua:81(func), c92345028.lua:84(func) | aux.ExtraDeckSummonCountLimit | 10 | 19 | 🔶 |
| `aux.FCheckAdditional` | c10833828.lua:94(func), c10833828.lua:105(func), c10833828.lua:115(func), c10833828.lua:145(func), c15717011.lua:45(func), c15717011.lua:48(func), c15717011.lua:54(func), c15717011.lua:58(func), c17725109.lua:71(func), c17725109.lua:73(func), c17725109.lua:90(func), c17725109.lua:92(func), c17725109.lua:109(func), c17725109.lua:111(func), c24220368.lua:41(func), c24220368.lua:45(func), c24220368.lua:102(func), c24220368.lua:106(func), c24220368.lua:126(func), c24220368.lua:130(func), c25861589.lua:80(func), c25861589.lua:85(func), c25861589.lua:112(func), c25861589.lua:116(func), c25861589.lua:136(func), c25861589.lua:140(func), c29062925.lua:44(func), c29062925.lua:49(func), c29062925.lua:76(func), c29062925.lua:80(func), c29062925.lua:99(func), c29062925.lua:103(func), c3078380.lua:59(func), c3078380.lua:70(func), c3078380.lua:78(func), c3078380.lua:108(func), c31444249.lua:74(func), c31444249.lua:79(func), c31444249.lua:106(func), c31444249.lua:110(func), c31444249.lua:129(func), c31444249.lua:133(func), c34813443.lua:61(func), c34813443.lua:72(func), c34813443.lua:84(func), c34813443.lua:120(func), c34813545.lua:104(func), c34813545.lua:106(func), c34813545.lua:124(func), c34813545.lua:126(func), c34813545.lua:143(func), c34813545.lua:145(func), c34933456.lua:42(func), c34933456.lua:53(func), c34933456.lua:69(func), c34933456.lua:99(func), c34995106.lua:66(func), c34995106.lua:77(func), c34995106.lua:90(func), c34995106.lua:123(func), c35098357.lua:38(func), c35098357.lua:40(func), c35098357.lua:57(func), c35098357.lua:59(func), c35098357.lua:76(func), c35098357.lua:78(func), c35705817.lua:42(func), c35705817.lua:46(func), c35705817.lua:74(func), c35705817.lua:78(func), c35705817.lua:98(func), c35705817.lua:102(func), c36484016.lua:32(func), c36484016.lua:34(func), c36484016.lua:85(func), c36484016.lua:99(func), c37517035.lua:42(func), c37517035.lua:53(func), c37517035.lua:61(func), c37517035.lua:101(func), c38129297.lua:33(func), c38129297.lua:46(func), c38129297.lua:81(func), c38129297.lua:111(func), c38984832.lua:54(func), c38984832.lua:59(func), c38984832.lua:86(func), c38984832.lua:90(func), c38984832.lua:110(func), c38984832.lua:114(func), c41940225.lua:34(func), c41940225.lua:36(func), c41940225.lua:86(func), c41940225.lua:100(func), c42577802.lua:38(func), c42577802.lua:43(func), c42577802.lua:70(func), c42577802.lua:74(func), c42577802.lua:94(func), c42577802.lua:98(func), c44362883.lua:42(func), c44362883.lua:44(func), c44362883.lua:92(func), c44362883.lua:94(func), c44886582.lua:78(func), c48144509.lua:40(func), c48144509.lua:45(func), c48144509.lua:72(func), c48144509.lua:76(func), c48144509.lua:96(func), c48144509.lua:100(func), c52101615.lua:45(func), c52101615.lua:48(func), c52101615.lua:54(func), c52101615.lua:58(func), c52947044.lua:33(func), c52947044.lua:44(func), c52947044.lua:55(func), c52947044.lua:97(func), c53541822.lua:66(func), c53541822.lua:77(func), c53541822.lua:89(func), c53541822.lua:127(func), c55704856.lua:22(func), c55704856.lua:24(func), c55704856.lua:74(func), c55704856.lua:107(func), c57425061.lua:44(func), c57425061.lua:49(func), c57425061.lua:76(func), c57425061.lua:80(func), c57425061.lua:100(func), c57425061.lua:104(func), c58549532.lua:34(func), c58549532.lua:46(func), c58549532.lua:56(func), c58549532.lua:87(func), c58570206.lua:44(func), c58570206.lua:47(func), c58570206.lua:71(func), c58570206.lua:90(func), c58570206.lua:93(func), c58570206.lua:120(func), c59332125.lua:48(func), c59332125.lua:52(func), c59332125.lua:77(func), c59332125.lua:81(func), c59332125.lua:100(func), c59332125.lua:104(func), c59514116.lua:30(func), c59514116.lua:41(func), c59514116.lua:76(func), c59514116.lua:106(func), c6172122.lua:41(func), c6172122.lua:43(func), c6172122.lua:90(func), c6172122.lua:111(func), c63136489.lua:42(func), c63136489.lua:53(func), c63136489.lua:61(func), c63136489.lua:93(func), c64061284.lua:38(func), c64061284.lua:41(func), c64061284.lua:60(func), c64061284.lua:63(func), c64061284.lua:80(func), c64061284.lua:82(func), c65037172.lua:46(func), c65037172.lua:57(func), c65037172.lua:68(func), c65037172.lua:98(func), c65801012.lua:42(func), c65801012.lua:47(func), c65801012.lua:74(func), c65801012.lua:78(func), c65801012.lua:98(func), c65801012.lua:102(func), c66518509.lua:85(func), c66518509.lua:96(func), c66518509.lua:109(func), c66518509.lua:145(func), c68468459.lua:53(func), c68468459.lua:64(func), c68468459.lua:79(func), c68468459.lua:109(func), c69270537.lua:36(func), c69270537.lua:43(func), c71143015.lua:26(func), c71143015.lua:28(func), c71143015.lua:77(func), c71143015.lua:103(func), c75047173.lua:24(func), c75047173.lua:26(func), c75047173.lua:51(func), c75047173.lua:53(func), c76647978.lua:47(func), c76647978.lua:59(func), c76647978.lua:71(func), c76647978.lua:105(func), c87532344.lua:137(func), c87532344.lua:148(func), c87532344.lua:160(func), c87532344.lua:190(func), c87931906.lua:40(func), c87931906.lua:45(func), c87931906.lua:72(func), c87931906.lua:76(func), c87931906.lua:96(func), c87931906.lua:100(func), c88696724.lua:45(func), c88696724.lua:48(func), c88696724.lua:54(func), c88696724.lua:58(func), c92058902.lua:22(func), c92058902.lua:24(func), c92058902.lua:69(func), c92058902.lua:81(func), c96687733.lua:35(func), c96687733.lua:37(func), c96687733.lua:88(func), c96687733.lua:90(func), c99161253.lua:41(func), c99161253.lua:43(func), c99161253.lua:61(func), c99161253.lua:102(func), c99426088.lua:48(func), c99426088.lua:53(func), c99426088.lua:92(func), c99426088.lua:96(func), c99426088.lua:140(func), c99426088.lua:144(func), c99941223.lua:21(func), c99941223.lua:23(func), c99941223.lua:83(func), c99941223.lua:85(func), c12381100.lua:66(guard), c32467459.lua:118(guard), c33166263.lua:44(guard), c50907446.lua:128(guard), c93053159.lua:64(guard) | c.branded_fusion_check, c.cyber_fusion_check, c.destruction_swordsman_fusion_check, c.hero_fusion_check, c.red_eyes_fusion_check, c.synchro_fusion_check, c.ultimate_fusion_check, c10833828.fcheck1(), c15717011.fcheck, c29062925.frcheck, c3078380.fcheck, c31444249.fcheck, c34813545.fcheck, c34933456.fcheck, c34995106.fcheck, c35098357.fcheck, c35705817.fcheck, c42577802.fcheck, c48144509.fcheck, c52101615.fcheck, c53541822.fcheck, c57425061.fcheck, c58549532.fcheck, c59332125.fcheck, c59514116.fcheck, c64061284.fcheck, c65037172.fcheck, c65801012.fcheck, c68468459.fcheck(), c69270537.fcheck(), c76647978.fcheck, c87931906.fcheck, c88696724.fcheck, c99426088.frcheck, nil, s.check, s.fcheck, s.fcheck1(), tc.branded_fusion_check, tc.cyber_fusion_check, tc.destruction_swordsman_fusion_check, tc.hero_fusion_check, tc.red_eyes_fusion_check, tc.synchro_fusion_check, tc.ultimate_fusion_check | 235 | 0 | 🔶✅ |
| `aux.FGoalCheckAdditional` | c58570206.lua:45(func), c58570206.lua:48(func), c58570206.lua:72(func), c58570206.lua:91(func), c58570206.lua:94(func), c58570206.lua:121(func), c7614732.lua:23(func), c7614732.lua:34(func), c7614732.lua:43(func), c7614732.lua:103(func), c78420796.lua:49(func), c78420796.lua:61(func), c78420796.lua:71(func), c78420796.lua:102(func), c8148322.lua:46(func), c8148322.lua:48(func), c8148322.lua:67(func), c8148322.lua:69(func), c8148322.lua:86(func), c8148322.lua:88(func) | c7614732.fcheck, c78420796.fcheck, c8148322.fcheck, nil, s.fcheck2 | 20 | 0 | 🔶 |
| `aux.GCheckAdditional` | c11398951.lua:43(func), c11398951.lua:45(func), c11489642.lua:66(func), c11489642.lua:68(func), c12215894.lua:73(func), c12215894.lua:75(func), c13386407.lua:29(func), c13386407.lua:31(func), c13386407.lua:83(func), c13386407.lua:85(func), c14386013.lua:60(func), c14386013.lua:62(func), c16329071.lua:28(func), c16329071.lua:30(func), c16494704.lua:51(func), c16494704.lua:53(func), c17888577.lua:50(func), c17888577.lua:52(func), c17956906.lua:46(func), c17956906.lua:48(func), c18738846.lua:75(func), c18738846.lua:77(func), c20071842.lua:48(func), c20071842.lua:50(func), c20994205.lua:45(func), c20994205.lua:47(func), c20994205.lua:59(func), c20994205.lua:61(func), c22398665.lua:71(func), c22398665.lua:73(func), c22398665.lua:123(func), c22398665.lua:125(func), c2266498.lua:18(func), c2266498.lua:20(func), c2266498.lua:54(func), c2266498.lua:56(func), c23187256.lua:59(func), c23187256.lua:61(func), c23288411.lua:51(func), c23288411.lua:53(func), c23288411.lua:59(func), c23288411.lua:61(func), c24166324.lua:102(func), c24166324.lua:104(func), c24166324.lua:154(func), c24166324.lua:156(func), c24220368.lua:42(func), c24220368.lua:46(func), c24220368.lua:103(func), c24220368.lua:107(func), c24220368.lua:127(func), c24220368.lua:131(func), c25861589.lua:81(func), c25861589.lua:86(func), c25861589.lua:113(func), c25861589.lua:117(func), c25861589.lua:137(func), c25861589.lua:141(func), c28429121.lua:56(func), c28429121.lua:58(func), c28770951.lua:53(func), c28770951.lua:55(func), c29062925.lua:45(func), c29062925.lua:50(func), c29062925.lua:77(func), c29062925.lua:81(func), c29062925.lua:100(func), c29062925.lua:104(func), c30270176.lua:62(func), c30270176.lua:64(func), c30581601.lua:78(func), c30581601.lua:80(func), c30983281.lua:63(func), c30983281.lua:65(func), c31002402.lua:57(func), c31002402.lua:59(func), c31444249.lua:75(func), c31444249.lua:80(func), c31444249.lua:107(func), c31444249.lua:111(func), c31444249.lua:130(func), c31444249.lua:134(func), c33166263.lua:54(func), c33166263.lua:57(func), c33166263.lua:64(func), c33166263.lua:70(func), c33166263.lua:81(func), c33781156.lua:46(func), c33781156.lua:48(func), c33955120.lua:59(func), c33955120.lua:61(func), c3434362.lua:44(func), c3434362.lua:47(func), c3434362.lua:52(func), c34813545.lua:56(func), c34813545.lua:58(func), c35546670.lua:68(func), c35546670.lua:70(func), c35705817.lua:43(func), c35705817.lua:47(func), c35705817.lua:75(func), c35705817.lua:79(func), c35705817.lua:99(func), c35705817.lua:103(func), c36328300.lua:45(func), c36328300.lua:47(func), c36350300.lua:37(func), c36350300.lua:39(func), c36608728.lua:81(func), c36608728.lua:83(func), c36849933.lua:88(func), c36849933.lua:90(func), c36920182.lua:45(func), c36920182.lua:48(func), c36982581.lua:63(func), c36982581.lua:65(func), c38129297.lua:124(func), c38129297.lua:127(func), c38784726.lua:65(func), c38784726.lua:67(func), c38984832.lua:55(func), c38984832.lua:60(func), c38984832.lua:87(func), c38984832.lua:91(func), c38984832.lua:111(func), c38984832.lua:115(func), c39114494.lua:87(func), c39114494.lua:89(func), c41371602.lua:100(func), c41371602.lua:102(func), c42158279.lua:54(func), c42158279.lua:56(func), c42577802.lua:39(func), c42577802.lua:44(func), c42577802.lua:71(func), c42577802.lua:75(func), c42577802.lua:95(func), c42577802.lua:99(func), c45148985.lua:57(func), c45148985.lua:59(func), c45148985.lua:65(func), c45148985.lua:67(func), c45675980.lua:37(func), c45675980.lua:39(func), c4575541.lua:66(func), c4575541.lua:68(func), c46033517.lua:65(func), c46033517.lua:67(func), c46033517.lua:73(func), c46033517.lua:75(func), c46033517.lua:125(func), c46033517.lua:127(func), c46052429.lua:44(func), c46052429.lua:46(func), c48144509.lua:41(func), c48144509.lua:46(func), c48144509.lua:73(func), c48144509.lua:77(func), c48144509.lua:97(func), c48144509.lua:101(func), c4837861.lua:46(func), c4837861.lua:48(func), c50596425.lua:63(func), c50596425.lua:65(func), c51124303.lua:107(func), c51124303.lua:109(func), c51510279.lua:99(func), c51510279.lua:101(func), c52020510.lua:45(func), c52020510.lua:47(func), c55795155.lua:151(func), c55795155.lua:153(func), c57425061.lua:45(func), c57425061.lua:50(func), c57425061.lua:77(func), c57425061.lua:81(func), c57425061.lua:101(func), c57425061.lua:105(func), c58549532.lua:35(func), c58549532.lua:47(func), c58549532.lua:57(func), c58549532.lua:88(func), c59332125.lua:49(func), c59332125.lua:53(func), c59332125.lua:78(func), c59332125.lua:82(func), c59332125.lua:101(func), c59332125.lua:105(func), c59514116.lua:121(func), c59514116.lua:124(func), c59934749.lua:112(func), c59934749.lua:114(func), c60830240.lua:69(func), c60830240.lua:71(func), c63056220.lua:79(func), c63056220.lua:81(func), c63526052.lua:74(func), c63526052.lua:76(func), c63526052.lua:78(func), c63526052.lua:91(func), c63526052.lua:93(func), c63526052.lua:95(func), c63526052.lua:118(func), c63526052.lua:120(func), c63679166.lua:50(func), c63679166.lua:52(func), c65801012.lua:43(func), c65801012.lua:48(func), c65801012.lua:75(func), c65801012.lua:79(func), c65801012.lua:99(func), c65801012.lua:103(func), c69003792.lua:60(func), c69003792.lua:62(func), c69003792.lua:87(func), c69003792.lua:89(func), c69815951.lua:109(func), c69815951.lua:111(func), c69815951.lua:114(func), c69815951.lua:117(func), c7337976.lua:69(func), c7337976.lua:71(func), c73881652.lua:57(func), c73881652.lua:59(func), c74431740.lua:70(func), c74431740.lua:72(func), c76647978.lua:48(func), c76647978.lua:60(func), c76647978.lua:72(func), c76647978.lua:106(func), c77075360.lua:68(func), c77075360.lua:70(func), c77783947.lua:40(func), c77783947.lua:42(func), c78420796.lua:50(func), c78420796.lua:62(func), c78420796.lua:72(func), c78420796.lua:103(func), c78785392.lua:53(func), c78785392.lua:55(func), c79407975.lua:43(func), c79407975.lua:45(func), c7986397.lua:56(func), c7986397.lua:58(func), c81560239.lua:76(func), c81560239.lua:78(func), c81560239.lua:105(func), c81560239.lua:107(func), c81677154.lua:66(func), c81677154.lua:68(func), c82197831.lua:70(func), c82197831.lua:72(func), c82197831.lua:82(func), c82197831.lua:85(func), c82428674.lua:40(func), c82428674.lua:42(func), c82997779.lua:74(func), c82997779.lua:76(func), c8428836.lua:40(func), c8428836.lua:42(func), c8428836.lua:66(func), c8428836.lua:68(func), c84425220.lua:55(func), c84425220.lua:58(func), c84425220.lua:63(func), c84546257.lua:97(func), c84546257.lua:99(func), c84546257.lua:110(func), c84546257.lua:112(func), c85327820.lua:59(func), c85327820.lua:61(func), c85704698.lua:69(func), c85704698.lua:71(func), c86133013.lua:21(func), c86133013.lua:23(func), c86133013.lua:62(func), c86133013.lua:64(func), c87804365.lua:35(func), c87804365.lua:38(func), c87804365.lua:50(func), c87804365.lua:58(func), c87931906.lua:41(func), c87931906.lua:46(func), c87931906.lua:73(func), c87931906.lua:77(func), c87931906.lua:97(func), c87931906.lua:101(func), c8805651.lua:94(func), c8805651.lua:96(func), c90207654.lua:67(func), c90207654.lua:69(func), c90303227.lua:66(func), c90303227.lua:69(func), c90444325.lua:79(func), c90444325.lua:81(func), c91588074.lua:39(func), c91588074.lua:41(func), c91588074.lua:47(func), c91588074.lua:49(func), c93053159.lua:51(func), c93053159.lua:53(func), c93053159.lua:112(func), c93053159.lua:115(func), c93754402.lua:41(func), c93754402.lua:43(func), c95209656.lua:107(func), c95209656.lua:109(func), c95209656.lua:120(func), c95209656.lua:123(func), c96142517.lua:53(func), c96142517.lua:55(func), c96142517.lua:59(func), c96142517.lua:61(func), c97051536.lua:62(func), c97051536.lua:64(func), c99426088.lua:49(func), c99426088.lua:54(func), c99426088.lua:93(func), c99426088.lua:97(func), c99426088.lua:141(func), c99426088.lua:145(func), c99426088.lua:170(func), c99426088.lua:172(func) | aux.PendOperationCheck(), aux.RitualCheckAdditional(), aux.SynGroupCheckLevelAddition(), aux.dabcheck, aux.dlvcheck, aux.dncheck, aux.drkcheck, c20994205.gcheck(), c22398665.RitualCheckAdditional(), c29062925.gcheck, c31444249.gcheck, c35705817.gcheck, c42577802.gcheck, c46033517.hspgcheck, c46033517.spcheck, c48144509.gcheck, c51124303.RitualCheckAdditional(), c52020510.spcheck, c57425061.gcheck, c58549532.gcheck, c59332125.gcheck(), c65801012.gcheck, c69815951.gcheck(), c76647978.gcheck, c78420796.gcheck, c82197831.gcheck, c84425220.gcheck, c84546257.gcheck, c86133013.gcheck(), c87931906.gcheck, c90207654.gcheck(), c95209656.gcheck, c96142517.gcheck, c99426088.gcheck, function, nil, s.RitualCheckAdditional(), s.gcheck, s.gcheck(), s.lncheck | 323 | 0 | 🔶 |
| `aux.PendulumChecklist` | c31531170.lua:14(func), c31531170.lua:99(func), c31531170.lua:14(guard), c31531170.lua:41(guard) | aux.PendulumChecklist, number | 2 | 3 | 🔶✅ |
| `aux.RCheckAdditional` | c36849933.lua:62(func), c36849933.lua:64(func), c36849933.lua:75(func), c36849933.lua:92(func), c36849933.lua:101(func), c38129297.lua:47(func), c38129297.lua:51(func), c38129297.lua:115(func), c38129297.lua:129(func), c38129297.lua:138(func), c59514116.lua:43(func), c59514116.lua:46(func), c59514116.lua:110(func), c59514116.lua:126(func), c59514116.lua:135(func), c63056220.lua:53(func), c63056220.lua:55(func), c63056220.lua:66(func), c63056220.lua:83(func), c63056220.lua:92(func), c7986397.lua:29(func), c7986397.lua:32(func), c7986397.lua:43(func), c7986397.lua:60(func), c7986397.lua:87(func), c81560239.lua:30(func), c81560239.lua:33(func), c81560239.lua:89(func), c81560239.lua:109(func), c81560239.lua:119(func), c8805651.lua:65(func), c8805651.lua:67(func), c8805651.lua:79(func), c8805651.lua:90(func), c8805651.lua:98(func), c8805651.lua:107(func), c90444325.lua:53(func), c90444325.lua:55(func), c90444325.lua:66(func), c90444325.lua:83(func), c90444325.lua:92(func), c99426088.lua:70(func), c99426088.lua:73(func), c99426088.lua:112(func), c99426088.lua:115(func), c99426088.lua:160(func), c99426088.lua:174(func), c99426088.lua:188(func) | c36849933.rcheck(), c59514116.rcheck, c63056220.rcheck(), c7986397.rcheck, c90444325.rcheck(), c99426088.frcheck, nil, s.rcheck, s.rcheck() | 48 | 4 | 🔶 |
| `aux.RGCheckAdditional` | c7986397.lua:30(func), c7986397.lua:33(func), c7986397.lua:44(func), c7986397.lua:61(func), c7986397.lua:88(func), c81560239.lua:31(func), c81560239.lua:34(func), c81560239.lua:90(func), c81560239.lua:110(func), c81560239.lua:120(func), c99426088.lua:71(func), c99426088.lua:74(func), c99426088.lua:113(func), c99426088.lua:116(func), c99426088.lua:161(func), c99426088.lua:175(func), c99426088.lua:189(func) | c7986397.rgcheck, c99426088.gcheck, nil, s.rgcheck | 17 | 12 | 🔶 |
| `aux.SushipMentionsTable` | c83008724.lua:5(func) | aux.SushipMentionsTable | 1 | 2 | 🔶 |
| `aux.fus_mat_hack_check` | c2344618.lua:35(func), c5370235.lua:28(func), c72064891.lua:31(func), c86758746.lua:27(func), c2344618.lua:34(guard), c5370235.lua:27(guard), c72064891.lua:30(guard), c86758746.lua:26(guard) | boolean | 4 | 0 | 🔶✅ |
| `aux.rit_mat_hack_check` | c20560620.lua:27(func), c20560620.lua:26(guard) | boolean | 1 | 0 | 🔶✅ |
| `s.[]` | c57847269.lua:34(func), c57847269.lua:79(func), c57847269.lua:84(func), c7903368.lua:31(func), c7903368.lua:32(func), c7903368.lua:46(func), c7903368.lua:49(func), c7903368.lua:50(func) | bit.bor(), number | 8 | 4 | 🔶 |
| `s.dark_calling` | c13708888.lua:51(top), c59893882.lua:41(top), c86282581.lua:39(top) | boolean | 3 | 0 |  |
| `s.fusion_effect` | c10218411.lua:15(top), c20934683.lua:29(top), c23738096.lua:27(top), c25861589.lua:28(top), c26434972.lua:25(top), c34813443.lua:35(top), c3496543.lua:28(top), c35614780.lua:39(top), c38648860.lua:38(top), c42201897.lua:29(top), c49867899.lua:28(top), c50042011.lua:52(top), c58570206.lua:14(top), c62002838.lua:38(top), c62091148.lua:29(top), c63181559.lua:26(top), c66518509.lua:36(top), c71939275.lua:53(top), c73714736.lua:49(top), c77124096.lua:15(top), c9102835.lua:45(top), c94845588.lua:25(top), c98567237.lua:27(top), c98828338.lua:26(top), c99599062.lua:27(top) | boolean | 25 | 0 |  |
| `s.global_check` | c10113611.lua:22(func), c10529441.lua:15(func), c11155484.lua:27(func), c13076804.lua:42(func), c13243124.lua:33(func), c17269895.lua:16(func), c19271881.lua:29(func), c20904475.lua:39(func), c22916418.lua:46(func), c25388971.lua:28(func), c27275398.lua:29(func), c27308231.lua:27(func), c27822206.lua:25(func), c30432463.lua:43(func), c3048768.lua:47(func), c31149212.lua:28(func), c35405755.lua:34(func), c37617348.lua:42(func), c44466810.lua:24(func), c4472318.lua:29(func), c49565413.lua:39(func), c50042011.lua:18(func), c5063379.lua:45(func), c51869363.lua:16(func), c53545926.lua:33(func), c53792930.lua:28(func), c54475145.lua:39(func), c55421040.lua:17(func), c57232301.lua:36(func), c57847269.lua:33(func), c64626565.lua:27(func), c66518509.lua:27(func), c69053263.lua:35(func), c7020743.lua:55(func), c72880377.lua:35(func), c75787708.lua:45(func), c76504386.lua:39(func), c76636978.lua:34(func), c77894049.lua:36(func), c78905039.lua:30(func), c7903368.lua:30(func), c8152834.lua:16(func), c83319154.lua:27(func), c85523502.lua:33(func), c86809440.lua:50(func), c8778267.lua:17(func), c89948817.lua:43(func), c93039339.lua:8(func), c93509766.lua:42(func), c95515789.lua:39(func), c99707692.lua:25(func), c10113611.lua:21(guard), c10529441.lua:14(guard), c11155484.lua:26(guard), c13076804.lua:41(guard), c13243124.lua:32(guard), c17269895.lua:15(guard), c19271881.lua:28(guard), c20904475.lua:38(guard), c22916418.lua:45(guard), c25388971.lua:27(guard), c27275398.lua:28(guard), c27308231.lua:26(guard), c27822206.lua:24(guard), c30432463.lua:42(guard), c3048768.lua:46(guard), c31149212.lua:27(guard), c35405755.lua:33(guard), c37617348.lua:41(guard), c44466810.lua:23(guard), c4472318.lua:28(guard), c49565413.lua:38(guard), c50042011.lua:17(guard), c5063379.lua:44(guard), c51869363.lua:15(guard), c53545926.lua:32(guard), c53792930.lua:27(guard), c54475145.lua:38(guard), c55421040.lua:16(guard), c57232301.lua:35(guard), c57847269.lua:32(guard), c64626565.lua:26(guard), c66518509.lua:26(guard), c69053263.lua:34(guard), c7020743.lua:54(guard), c72880377.lua:34(guard), c75787708.lua:44(guard), c76504386.lua:38(guard), c76636978.lua:33(guard), c77894049.lua:35(guard), c78905039.lua:29(guard), c7903368.lua:29(guard), c8152834.lua:15(guard), c83319154.lua:26(guard), c85523502.lua:32(guard), c86809440.lua:49(guard), c8778267.lua:16(guard), c89948817.lua:42(guard), c93039339.lua:7(guard), c93509766.lua:41(guard), c95515789.lua:38(guard), c99707692.lua:24(guard) | boolean | 51 | 0 | 🔶✅ |
| `s.global_flag` | c65541655.lua:53(func), c84544192.lua:46(func), c65541655.lua:52(guard), c84544192.lua:45(guard) | boolean | 2 | 0 | 🔶✅ |
| `s.globle_check` | c12157563.lua:22(func), c12157563.lua:21(guard) | boolean | 1 | 0 | 🔶✅ |
| `s.killer_tune_be_material_effect` | c16387555.lua:36(func), c16509007.lua:39(func), c17209452.lua:36(func), c42781164.lua:39(func), c43904702.lua:38(func), c89392810.lua:39(func) | e3, e4 | 6 | 0 | 🔶 |
| `s.set_as_spell` | c65504487.lua:35(top), c69925461.lua:34(top) | boolean | 2 | 0 |  |
| `s.shadoll_flip_effect` | c95072744.lua:27(func) | e1 | 1 | 0 | 🔶 |
| `s.star_knight_summon_effect` | c33302589.lua:21(func), c60700283.lua:21(func) | e1 | 2 | 0 | 🔶 |
| `s.toss_coin` | c12686296.lua:35(top), c30913809.lua:47(top), c3376703.lua:30(top), c39761418.lua:32(top), c84079032.lua:33(top), c86304179.lua:16(top) | boolean | 6 | 0 |  |
| `s.treat_itself_tuner` | c31114334.lua:33(top), c46956301.lua:31(top), c52596406.lua:24(top), c98684051.lua:24(top) | boolean | 4 | 0 |  |

## 一次性守卫模式（重点：跨 field 需重置）

| 变量 | 守卫位置 | 赋值位置 | 值类型 |
|---|---|---|---|
| `Auxiliary.ExtraDeckSummonCountLimit` | utility.lua:483 | utility.lua:484 | table |
| `Auxiliary.FCheckAdditional` | procedure.lua:1118, procedure.lua:1158, procedure.lua:1191, procedure.lua:1371 | procedure.lua:2, procedure.lua:1488, procedure.lua:1490, procedure.lua:1519, procedure.lua:1531, procedure.lua:1595, procedure.lua:1607, procedure.lua:1629, procedure.lua:1634, procedure.lua:1670 | nil, params.fcheck, params.get_fcheck() |
| `Auxiliary.FGoalCheckAdditional` | procedure.lua:1111 | procedure.lua:3, procedure.lua:1524, procedure.lua:1548, procedure.lua:1600, procedure.lua:1672 | nil, params.fgoalcheck |
| `Auxiliary.GCheckAdditional` | utility.lua:1198, utility.lua:1209, utility.lua:1237, utility.lua:1238, utility.lua:1319 | c18988396.lua:87, c18988396.lua:89, c18988396.lua:99, c18988396.lua:101, procedure.lua:571, procedure.lua:573, procedure.lua:623, procedure.lua:625, procedure.lua:755, procedure.lua:757, procedure.lua:786, procedure.lua:788, procedure.lua:807, procedure.lua:809, procedure.lua:1520, procedure.lua:1522, procedure.lua:1532, procedure.lua:1596, procedure.lua:1598, procedure.lua:1608, procedure.lua:1630, procedure.lua:1632, procedure.lua:1671, procedure.lua:1837, procedure.lua:1839, procedure.lua:1890, procedure.lua:1892, procedure.lua:2082, procedure.lua:2084, utility.lua:56 | Auxiliary.PendOperationCheck(), Auxiliary.RitualCheckAdditional(), Auxiliary.TuneMagicianCheckAdditionalXyz, function, nil, params.gcheck, params.get_gcheck() |
| `Auxiliary.MulcharmyGlobalFlag` | utility.lua:2042 | utility.lua:2043 | boolean |
| `Auxiliary.PendulumChecklist` | procedure.lua:1949, procedure.lua:2008 | procedure.lua:1950, procedure.lua:1978, procedure.lua:2090 | Auxiliary.PendulumChecklist, number |
| `Auxiliary.merge_single_global_check` | utility.lua:1719 | utility.lua:1720 | boolean |
| `aux.FCheckAdditional` | c12381100.lua:66, c32467459.lua:118, c33166263.lua:44, c50907446.lua:128, c93053159.lua:64 | c10833828.lua:94, c10833828.lua:105, c10833828.lua:115, c10833828.lua:145, c15717011.lua:45, c15717011.lua:48, c15717011.lua:54, c15717011.lua:58, c17725109.lua:71, c17725109.lua:73, c17725109.lua:90, c17725109.lua:92, c17725109.lua:109, c17725109.lua:111, c24220368.lua:41, c24220368.lua:45, c24220368.lua:102, c24220368.lua:106, c24220368.lua:126, c24220368.lua:130, c25861589.lua:80, c25861589.lua:85, c25861589.lua:112, c25861589.lua:116, c25861589.lua:136, c25861589.lua:140, c29062925.lua:44, c29062925.lua:49, c29062925.lua:76, c29062925.lua:80, c29062925.lua:99, c29062925.lua:103, c3078380.lua:59, c3078380.lua:70, c3078380.lua:78, c3078380.lua:108, c31444249.lua:74, c31444249.lua:79, c31444249.lua:106, c31444249.lua:110, c31444249.lua:129, c31444249.lua:133, c34813443.lua:61, c34813443.lua:72, c34813443.lua:84, c34813443.lua:120, c34813545.lua:104, c34813545.lua:106, c34813545.lua:124, c34813545.lua:126, c34813545.lua:143, c34813545.lua:145, c34933456.lua:42, c34933456.lua:53, c34933456.lua:69, c34933456.lua:99, c34995106.lua:66, c34995106.lua:77, c34995106.lua:90, c34995106.lua:123, c35098357.lua:38, c35098357.lua:40, c35098357.lua:57, c35098357.lua:59, c35098357.lua:76, c35098357.lua:78, c35705817.lua:42, c35705817.lua:46, c35705817.lua:74, c35705817.lua:78, c35705817.lua:98, c35705817.lua:102, c36484016.lua:32, c36484016.lua:34, c36484016.lua:85, c36484016.lua:99, c37517035.lua:42, c37517035.lua:53, c37517035.lua:61, c37517035.lua:101, c38129297.lua:33, c38129297.lua:46, c38129297.lua:81, c38129297.lua:111, c38984832.lua:54, c38984832.lua:59, c38984832.lua:86, c38984832.lua:90, c38984832.lua:110, c38984832.lua:114, c41940225.lua:34, c41940225.lua:36, c41940225.lua:86, c41940225.lua:100, c42577802.lua:38, c42577802.lua:43, c42577802.lua:70, c42577802.lua:74, c42577802.lua:94, c42577802.lua:98, c44362883.lua:42, c44362883.lua:44, c44362883.lua:92, c44362883.lua:94, c44886582.lua:78, c48144509.lua:40, c48144509.lua:45, c48144509.lua:72, c48144509.lua:76, c48144509.lua:96, c48144509.lua:100, c52101615.lua:45, c52101615.lua:48, c52101615.lua:54, c52101615.lua:58, c52947044.lua:33, c52947044.lua:44, c52947044.lua:55, c52947044.lua:97, c53541822.lua:66, c53541822.lua:77, c53541822.lua:89, c53541822.lua:127, c55704856.lua:22, c55704856.lua:24, c55704856.lua:74, c55704856.lua:107, c57425061.lua:44, c57425061.lua:49, c57425061.lua:76, c57425061.lua:80, c57425061.lua:100, c57425061.lua:104, c58549532.lua:34, c58549532.lua:46, c58549532.lua:56, c58549532.lua:87, c58570206.lua:44, c58570206.lua:47, c58570206.lua:71, c58570206.lua:90, c58570206.lua:93, c58570206.lua:120, c59332125.lua:48, c59332125.lua:52, c59332125.lua:77, c59332125.lua:81, c59332125.lua:100, c59332125.lua:104, c59514116.lua:30, c59514116.lua:41, c59514116.lua:76, c59514116.lua:106, c6172122.lua:41, c6172122.lua:43, c6172122.lua:90, c6172122.lua:111, c63136489.lua:42, c63136489.lua:53, c63136489.lua:61, c63136489.lua:93, c64061284.lua:38, c64061284.lua:41, c64061284.lua:60, c64061284.lua:63, c64061284.lua:80, c64061284.lua:82, c65037172.lua:46, c65037172.lua:57, c65037172.lua:68, c65037172.lua:98, c65801012.lua:42, c65801012.lua:47, c65801012.lua:74, c65801012.lua:78, c65801012.lua:98, c65801012.lua:102, c66518509.lua:85, c66518509.lua:96, c66518509.lua:109, c66518509.lua:145, c68468459.lua:53, c68468459.lua:64, c68468459.lua:79, c68468459.lua:109, c69270537.lua:36, c69270537.lua:43, c71143015.lua:26, c71143015.lua:28, c71143015.lua:77, c71143015.lua:103, c75047173.lua:24, c75047173.lua:26, c75047173.lua:51, c75047173.lua:53, c76647978.lua:47, c76647978.lua:59, c76647978.lua:71, c76647978.lua:105, c87532344.lua:137, c87532344.lua:148, c87532344.lua:160, c87532344.lua:190, c87931906.lua:40, c87931906.lua:45, c87931906.lua:72, c87931906.lua:76, c87931906.lua:96, c87931906.lua:100, c88696724.lua:45, c88696724.lua:48, c88696724.lua:54, c88696724.lua:58, c92058902.lua:22, c92058902.lua:24, c92058902.lua:69, c92058902.lua:81, c96687733.lua:35, c96687733.lua:37, c96687733.lua:88, c96687733.lua:90, c99161253.lua:41, c99161253.lua:43, c99161253.lua:61, c99161253.lua:102, c99426088.lua:48, c99426088.lua:53, c99426088.lua:92, c99426088.lua:96, c99426088.lua:140, c99426088.lua:144, c99941223.lua:21, c99941223.lua:23, c99941223.lua:83, c99941223.lua:85 | c.branded_fusion_check, c.cyber_fusion_check, c.destruction_swordsman_fusion_check, c.hero_fusion_check, c.red_eyes_fusion_check, c.synchro_fusion_check, c.ultimate_fusion_check, c10833828.fcheck1(), c15717011.fcheck, c29062925.frcheck, c3078380.fcheck, c31444249.fcheck, c34813545.fcheck, c34933456.fcheck, c34995106.fcheck, c35098357.fcheck, c35705817.fcheck, c42577802.fcheck, c48144509.fcheck, c52101615.fcheck, c53541822.fcheck, c57425061.fcheck, c58549532.fcheck, c59332125.fcheck, c59514116.fcheck, c64061284.fcheck, c65037172.fcheck, c65801012.fcheck, c68468459.fcheck(), c69270537.fcheck(), c76647978.fcheck, c87931906.fcheck, c88696724.fcheck, c99426088.frcheck, nil, s.check, s.fcheck, s.fcheck1(), tc.branded_fusion_check, tc.cyber_fusion_check, tc.destruction_swordsman_fusion_check, tc.hero_fusion_check, tc.red_eyes_fusion_check, tc.synchro_fusion_check, tc.ultimate_fusion_check |
| `aux.PendulumChecklist` | c31531170.lua:14, c31531170.lua:41 | c31531170.lua:14, c31531170.lua:99 | aux.PendulumChecklist, number |
| `aux.fus_mat_hack_check` | c2344618.lua:34, c5370235.lua:27, c72064891.lua:30, c86758746.lua:26 | c2344618.lua:35, c5370235.lua:28, c72064891.lua:31, c86758746.lua:27 | boolean |
| `aux.rit_mat_hack_check` | c20560620.lua:26 | c20560620.lua:27 | boolean |
| `c10497636.global_check` | c10497636.lua:36 | c10497636.lua:37 | boolean |
| `c12275533.global_check` | c12275533.lua:16 | c12275533.lua:17 | boolean |
| `c12289247.global_check` | c12289247.lua:37 | c12289247.lua:38 | boolean |
| `c12958919.global_check` | c12958919.lua:29 | c12958919.lua:30 | boolean |
| `c13224603.global_check` | c13224603.lua:37 | c13224603.lua:38 | boolean |
| `c14318794.global_check` | c14318794.lua:21 | c14318794.lua:22 | boolean |
| `c14934922.global_check` | c14934922.lua:14 | c14934922.lua:15 | boolean |
| `c16832845.global_check` | c16832845.lua:13 | c16832845.lua:14 | boolean |
| `c1801154.global_check` | c1801154.lua:19 | c1801154.lua:20 | boolean |
| `c18114794.global_check` | c18114794.lua:16 | c18114794.lua:17 | boolean |
| `c18558867.global_check` | c18558867.lua:28 | c18558867.lua:29 | boolean |
| `c18969888.global_check` | c18969888.lua:47 | c18969888.lua:48 | boolean |
| `c1966438.global_check` | c1966438.lua:46 | c1966438.lua:47 | boolean |
| `c197042.global_check` | c197042.lua:16 | c197042.lua:17 | boolean |
| `c19974890.global_check` | c19974890.lua:12 | c19974890.lua:13 | boolean |
| `c20057949.global_check` | c20057949.lua:12 | c20057949.lua:13 | boolean |
| `c20501450.global_check` | c20501450.lua:15 | c20501450.lua:16 | boolean |
| `c20788863.global_check` | c20788863.lua:36 | c20788863.lua:37 | boolean |
| `c20822520.global_check` | c20822520.lua:15 | c20822520.lua:16 | boolean |
| `c21364070.global_check` | c21364070.lua:72 | c21364070.lua:73 | boolean |
| `c21862633.global_check` | c21862633.lua:35 | c21862633.lua:36 | boolean |
| `c22125101.global_check` | c22125101.lua:31 | c22125101.lua:32 | boolean |
| `c23931679.global_check` | c23931679.lua:38 | c23931679.lua:39 | boolean |
| `c24207889.global_check` | c24207889.lua:32 | c24207889.lua:33 | boolean |
| `c24207889.is_empty` | c24207889.lua:66 | c24207889.lua:34, c24207889.lua:73, c24207889.lua:77 | boolean |
| `c2547033.global_check` | c2547033.lua:32 | c2547033.lua:33 | boolean |
| `c26285788.global_check` | c26285788.lua:12 | c26285788.lua:13 | boolean |
| `c26773909.global_check` | c26773909.lua:15 | c26773909.lua:16 | boolean |
| `c27204311.global_check` | c27204311.lua:16 | c27204311.lua:17 | boolean |
| `c27769400.global_check` | c27769400.lua:24 | c27769400.lua:25 | boolean |
| `c27770341.counter` | c27770341.lua:11 | c27770341.lua:12 | boolean |
| `c29307554.global_check` | c29307554.lua:12 | c29307554.lua:13 | boolean |
| `c29724053.global_check` | c29724053.lua:25 | c29724053.lua:26 | boolean |
| `c29948294.global_check` | c29948294.lua:26 | c29948294.lua:27 | boolean |
| `c30353551.global_check` | c30353551.lua:20 | c30353551.lua:21 | boolean |
| `c30537973.global_check` | c30537973.lua:37 | c30537973.lua:38 | boolean |
| `c30765615.global_check` | c30765615.lua:24 | c30765615.lua:25 | boolean |
| `c31472884.global_check` | c31472884.lua:15 | c31472884.lua:16 | boolean |
| `c31699677.global_check` | c31699677.lua:31 | c31699677.lua:32 | boolean |
| `c32247099.global_check` | c32247099.lua:34 | c32247099.lua:35 | boolean |
| `c33103459.global_check` | c33103459.lua:27 | c33103459.lua:28 | boolean |
| `c33327029.global_check` | c33327029.lua:24 | c33327029.lua:25 | boolean |
| `c33545259.global_check` | c33545259.lua:42 | c33545259.lua:43 | boolean |
| `c33776734.global_check` | c33776734.lua:37 | c33776734.lua:38 | boolean |
| `c34620088.global_check` | c34620088.lua:21 | c34620088.lua:22 | boolean |
| `c34800281.global_check` | c34800281.lua:29 | c34800281.lua:30 | boolean |
| `c35268887.global_check` | c35268887.lua:9 | c35268887.lua:10 | boolean |
| `c35756798.global_check` | c35756798.lua:14 | c35756798.lua:15 | boolean |
| `c3576031.global_check` | c3576031.lua:33 | c3576031.lua:34 | boolean |
| `c36458063.global_check` | c36458063.lua:19 | c36458063.lua:20 | boolean |
| `c37241623.global_check` | c37241623.lua:13 | c37241623.lua:14 | boolean |
| `c38049934.check` | c38049934.lua:27 | c38049934.lua:14, c38049934.lua:19, c38049934.lua:28, c38049934.lua:31 | boolean |
| `c3972721.global_check` | c3972721.lua:14 | c3972721.lua:15 | boolean |
| `c39853199.global_check` | c39853199.lua:35 | c39853199.lua:36 | boolean |
| `c4064925.global_check` | c4064925.lua:25 | c4064925.lua:26 | boolean |
| `c40737112.global_check` | c40737112.lua:28 | c40737112.lua:29 | boolean |
| `c41850466.global_check` | c41850466.lua:14 | c41850466.lua:15 | boolean |
| `c42228966.global_check` | c42228966.lua:29 | c42228966.lua:30 | boolean |
| `c42589641.global_check` | c42589641.lua:53 | c42589641.lua:54 | boolean |
| `c43383478.globle_check` | c43383478.lua:14 | c43383478.lua:15 | boolean |
| `c44190146.global_check` | c44190146.lua:28 | c44190146.lua:29 | boolean |
| `c49249907.global_check` | c49249907.lua:23 | c49249907.lua:24 | boolean |
| `c49430782.counter` | c49430782.lua:42 | c49430782.lua:43, c49430782.lua:64, c49430782.lua:78 | c49430782.counter, number |
| `c49980185.global_check` | c49980185.lua:14 | c49980185.lua:15 | boolean |
| `c51194046.global_check` | c51194046.lua:27 | c51194046.lua:28 | boolean |
| `c51339637.global_check` | c51339637.lua:27 | c51339637.lua:28 | boolean |
| `c52155219.global_check` | c52155219.lua:12 | c52155219.lua:13 | boolean |
| `c52900379.global_check` | c52900379.lua:22 | c52900379.lua:23 | boolean |
| `c53039326.global_check` | c53039326.lua:30 | c53039326.lua:31 | boolean |
| `c53251824.global_check` | c53251824.lua:13 | c53251824.lua:14 | boolean |
| `c54109233.global_check` | c54109233.lua:20 | c54109233.lua:21 | boolean |
| `c55273560.global_check` | c55273560.lua:39 | c55273560.lua:40 | boolean |
| `c55795155.global_check` | c55795155.lua:41 | c55795155.lua:42 | boolean |
| `c57995165.global_check` | c57995165.lua:13 | c57995165.lua:14 | boolean |
| `c58092907.global_check` | c58092907.lua:29 | c58092907.lua:30 | boolean |
| `c59419719.global_check` | c59419719.lua:25 | c59419719.lua:26 | boolean |
| `c59695933.global_check` | c59695933.lua:13 | c59695933.lua:14 | boolean |
| `c59957503.global_check` | c59957503.lua:12 | c59957503.lua:13 | boolean |
| `c60187739.global_check` | c60187739.lua:22 | c60187739.lua:23 | boolean |
| `c60406591.global_check` | c60406591.lua:12 | c60406591.lua:13 | boolean |
| `c60621361.global_check` | c60621361.lua:35 | c60621361.lua:36 | boolean |
| `c60930169.global_check` | c60930169.lua:12 | c60930169.lua:13 | boolean |
| `c60950180.global_check` | c60950180.lua:27 | c60950180.lua:28 | boolean |
| `c62161698.global_check` | c62161698.lua:14 | c62161698.lua:15 | boolean |
| `c62370023.global_check` | c62370023.lua:15 | c62370023.lua:16 | boolean |
| `c64178424.global_check` | c64178424.lua:30 | c64178424.lua:31 | boolean |
| `c66078354.global_check` | c66078354.lua:31 | c66078354.lua:32 | boolean |
| `c66150724.global_check` | c66150724.lua:25 | c66150724.lua:26 | boolean |
| `c67045174.global_check` | c67045174.lua:12 | c67045174.lua:13 | boolean |
| `c67100549.global_check` | c67100549.lua:36 | c67100549.lua:37 | boolean |
| `c67630339.global_check` | c67630339.lua:10 | c67630339.lua:11 | boolean |
| `c67712104.global_check` | c67712104.lua:37 | c67712104.lua:38 | boolean |
| `c67901914.global_check` | c67901914.lua:15 | c67901914.lua:16 | boolean |
| `c6850209.global_check` | c6850209.lua:11 | c6850209.lua:12 | boolean |
| `c6909330.global_check` | c6909330.lua:26 | c6909330.lua:27 | boolean |
| `c69145169.global_check` | c69145169.lua:15 | c69145169.lua:16 | boolean |
| `c70335319.global_check` | c70335319.lua:59 | c70335319.lua:60 | boolean |
| `c71095768.global_check` | c71095768.lua:39 | c71095768.lua:40 | boolean |
| `c71386411.global_check` | c71386411.lua:27 | c71386411.lua:28 | boolean |
| `c71612253.global_check` | c71612253.lua:32 | c71612253.lua:33 | boolean |
| `c71645242.global_check` | c71645242.lua:29 | c71645242.lua:30 | boolean |
| `c72554862.global_check` | c72554862.lua:24 | c72554862.lua:25 | boolean |
| `c73193552.global_check` | c73193552.lua:42 | c73193552.lua:43 | boolean |
| `c75782277.global_check` | c75782277.lua:32 | c75782277.lua:33 | boolean |
| `c75906310.global_flag` | c75906310.lua:37 | c75906310.lua:38 | boolean |
| `c76224717.global_check` | c76224717.lua:19 | c76224717.lua:20 | boolean |
| `c76552147.global_check` | c76552147.lua:14 | c76552147.lua:15 | boolean |
| `c76794549.global_check` | c76794549.lua:37 | c76794549.lua:38 | boolean |
| `c7714344.global_check` | c7714344.lua:24 | c7714344.lua:25 | boolean |
| `c80402389.global_check` | c80402389.lua:19 | c80402389.lua:20 | boolean |
| `c80949182.global_check` | c80949182.lua:50 | c80949182.lua:51 | boolean |
| `c81167171.global_check` | c81167171.lua:11 | c81167171.lua:12 | boolean |
| `c81193865.global_check` | c81193865.lua:40 | c81193865.lua:41 | boolean |
| `c8129306.global_check` | c8129306.lua:26 | c8129306.lua:27 | boolean |
| `c82052602.global_check` | c82052602.lua:13 | c82052602.lua:14 | boolean |
| `c82570174.global_check` | c82570174.lua:31 | c82570174.lua:32 | boolean |
| `c82670878.global_check` | c82670878.lua:23 | c82670878.lua:24 | boolean |
| `c82760689.global_check` | c82760689.lua:10 | c82760689.lua:11 | boolean |
| `c83236601.global_check` | c83236601.lua:31 | c83236601.lua:32 | boolean |
| `c83725008.discard` | c83725008.lua:15 | c83725008.lua:16 | boolean |
| `c83957459.global_check` | c83957459.lua:10 | c83957459.lua:11 | boolean |
| `c84211599.gf` | c84211599.lua:13 | c84211599.lua:14 | boolean |
| `c84274024.global_check` | c84274024.lua:37 | c84274024.lua:38 | boolean |
| `c84339249.global_check` | c84339249.lua:37 | c84339249.lua:38 | boolean |
| `c85360035.global_check` | c85360035.lua:26 | c85360035.lua:27 | boolean |
| `c85602018.global_check` | c85602018.lua:10 | c85602018.lua:11 | boolean |
| `c85862791.global_check` | c85862791.lua:14 | c85862791.lua:15 | boolean |
| `c86196216.globle_check` | c86196216.lua:13 | c86196216.lua:14 | boolean |
| `c86277379.global_check` | c86277379.lua:30 | c86277379.lua:31 | boolean |
| `c86541496.global_check` | c86541496.lua:12 | c86541496.lua:13 | boolean |
| `c8700633.global_check` | c8700633.lua:16 | c8700633.lua:17 | boolean |
| `c8785161.global_check` | c8785161.lua:13 | c8785161.lua:14 | boolean |
| `c88513608.global_check` | c88513608.lua:14 | c88513608.lua:15 | boolean |
| `c88540324.global_check` | c88540324.lua:40 | c88540324.lua:41 | boolean |
| `c88851326.global_check` | c88851326.lua:53 | c88851326.lua:54 | boolean |
| `c89792713.global_check` | c89792713.lua:12 | c89792713.lua:13 | boolean |
| `c90448279.global_check` | c90448279.lua:30 | c90448279.lua:31 | boolean |
| `c91269402.global_check` | c91269402.lua:14 | c91269402.lua:15 | boolean |
| `c91392974.global_check` | c91392974.lua:15 | c91392974.lua:16 | boolean |
| `c93211810.global_check` | c93211810.lua:15 | c93211810.lua:16 | boolean |
| `c93238626.global_check` | c93238626.lua:33 | c93238626.lua:34 | boolean |
| `c93368494.global_check` | c93368494.lua:53 | c93368494.lua:54 | boolean |
| `c9348522.global_check` | c9348522.lua:25 | c9348522.lua:26 | boolean |
| `c94145021.global_check` | c94145021.lua:14 | c94145021.lua:15 | boolean |
| `c94303232.global_check` | c94303232.lua:40 | c94303232.lua:41 | boolean |
| `c94585852.global_check` | c94585852.lua:28 | c94585852.lua:29 | boolean |
| `c95134948.global_check` | c95134948.lua:30 | c95134948.lua:31 | boolean |
| `c96345188.global_check` | c96345188.lua:23 | c96345188.lua:24 | boolean |
| `c97403510.global_check` | c97403510.lua:50 | c97403510.lua:51 | boolean |
| `c98864751.global_check` | c98864751.lua:30 | c98864751.lua:31 | boolean |
| `c99748883.global_check` | c99748883.lua:26 | c99748883.lua:27 | boolean |
| `s.global_check` | c10113611.lua:21, c10529441.lua:14, c11155484.lua:26, c13076804.lua:41, c13243124.lua:32, c17269895.lua:15, c19271881.lua:28, c20904475.lua:38, c22916418.lua:45, c25388971.lua:27, c27275398.lua:28, c27308231.lua:26, c27822206.lua:24, c30432463.lua:42, c3048768.lua:46, c31149212.lua:27, c35405755.lua:33, c37617348.lua:41, c44466810.lua:23, c4472318.lua:28, c49565413.lua:38, c50042011.lua:17, c5063379.lua:44, c51869363.lua:15, c53545926.lua:32, c53792930.lua:27, c54475145.lua:38, c55421040.lua:16, c57232301.lua:35, c57847269.lua:32, c64626565.lua:26, c66518509.lua:26, c69053263.lua:34, c7020743.lua:54, c72880377.lua:34, c75787708.lua:44, c76504386.lua:38, c76636978.lua:33, c77894049.lua:35, c78905039.lua:29, c7903368.lua:29, c8152834.lua:15, c83319154.lua:26, c85523502.lua:32, c86809440.lua:49, c8778267.lua:16, c89948817.lua:42, c93039339.lua:7, c93509766.lua:41, c95515789.lua:38, c99707692.lua:24 | c10113611.lua:22, c10529441.lua:15, c11155484.lua:27, c13076804.lua:42, c13243124.lua:33, c17269895.lua:16, c19271881.lua:29, c20904475.lua:39, c22916418.lua:46, c25388971.lua:28, c27275398.lua:29, c27308231.lua:27, c27822206.lua:25, c30432463.lua:43, c3048768.lua:47, c31149212.lua:28, c35405755.lua:34, c37617348.lua:42, c44466810.lua:24, c4472318.lua:29, c49565413.lua:39, c50042011.lua:18, c5063379.lua:45, c51869363.lua:16, c53545926.lua:33, c53792930.lua:28, c54475145.lua:39, c55421040.lua:17, c57232301.lua:36, c57847269.lua:33, c64626565.lua:27, c66518509.lua:27, c69053263.lua:35, c7020743.lua:55, c72880377.lua:35, c75787708.lua:45, c76504386.lua:39, c76636978.lua:34, c77894049.lua:36, c78905039.lua:30, c7903368.lua:30, c8152834.lua:16, c83319154.lua:27, c85523502.lua:33, c86809440.lua:50, c8778267.lua:17, c89948817.lua:43, c93039339.lua:8, c93509766.lua:42, c95515789.lua:39, c99707692.lua:25 | boolean |
| `s.global_flag` | c65541655.lua:52, c84544192.lua:45 | c65541655.lua:53, c84544192.lua:46 | boolean |
| `s.globle_check` | c12157563.lua:21 | c12157563.lua:22 | boolean |
