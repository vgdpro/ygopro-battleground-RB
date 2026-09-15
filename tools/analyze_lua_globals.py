#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分析 ygopro script/ 目录下所有 Lua 脚本的全局变量使用情况。

目标：枚举所有全局变量（含动态创建），分类统计，供 Lua 环境隔离方案决策使用。

方法：词法近似分析（不执行 Lua，避免副作用）。
- 识别 local 声明 / 函数参数 / for 循环变量，排除局部变量
- 识别全局赋值：Auxiliary.xxx / aux.xxx / c{code}.xxx / _G.xxx / 裸标识符
- 识别一次性守卫模式：if not Auxiliary.xxx then ... end
- 统计读取次数（出现次数 - 赋值次数）

用法：
    python tools/analyze_lua_globals.py [--script-dir PATH] [--out PATH] [--top N]
"""

import argparse
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

# ---------------------------------------------------------------------------
# 正则
# ---------------------------------------------------------------------------

# 赋值语句 LHS：捕获 前缀.字段 或 裸标识符（含数组下标形式）
ASSIGN_RE = re.compile(
    r"^\s*"
    r"(?P<lhs>"
    r"(?:[A-Za-z_]\w*)(?:\s*\.\s*[A-Za-z_]\w*|\s*\[\s*[^\]]+\s*\])*"
    r")"
    r"\s*=\s*(?P<rhs>.*)$"
)

# local 声明：local a, b = ... 或 local a
LOCAL_RE = re.compile(r"^\s*local\s+(?P<names>[A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)")

# 函数定义：function X.y(...) / function x(...) / local function x(...)
FUNCTION_DEF_RE = re.compile(
    r"^\s*(?:local\s+)?function\s+(?P<name>[A-Za-z_]\w*(?:\s*\.\s*[A-Za-z_]\w*)*)\s*\("
)

# 函数参数：function f(a, b, ...)
FUNC_PARAMS_RE = re.compile(r"^\s*(?:local\s+)?function\s+[A-Za-z_]\w*(?:\s*\.\s*[A-Za-z_]\w*)*\s*\((?P<params>[^)]*)\)")

# for 循环变量：for i = 1, n do / for k, v in pairs(...) do
FOR_RE = re.compile(r"^\s*for\s+(?P<names>[A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)\s*(?:=|in)\b")

# 一次性守卫模式
GUARD_RE = re.compile(
    r"^\s*if\s+(?:not\s+)?(?P<var>[A-Za-z_]\w*(?:\s*\.\s*[A-Za-z_]\w*)*)\s*"
    r"(?:~=\s*nil|==\s*nil|then|is\s+not\s+nil)?"
)

# 全局表前缀分类
AUX_PREFIX = ("Auxiliary", "aux")
CARD_PREFIX_RE = re.compile(r"^c\d+$")

# 值类型判断（看 RHS 首 token）
RHS_TYPE_RE = re.compile(r"^\s*(?P<tok>function|\{|-?\d|true|false|nil|['\"])")


# ---------------------------------------------------------------------------
# 数据结构
# ---------------------------------------------------------------------------

class GlobalVar:
    """一个全局变量的聚合信息。"""

    __slots__ = ("name", "kind", "assign_count", "read_count", "files",
                 "in_function", "value_types", "guarded", "examples")

    def __init__(self, name, kind):
        self.name = name
        self.kind = kind          # aux / card / bare / _G / other_table
        self.assign_count = 0
        self.read_count = 0
        self.files = set()        # 出现过的文件
        self.in_function = 0      # 函数内赋值次数
        self.value_types = Counter()
        self.guarded = False      # 是否有一性守卫模式
        self.examples = []        # 示例行

    def add_assign(self, file, rhs, in_function):
        self.assign_count += 1
        self.files.add(file)
        if in_function:
            self.in_function += 1
        m = RHS_TYPE_RE.match(rhs)
        if m:
            tok = m.group("tok")
            if tok == "function":
                self.value_types["function"] += 1
            elif tok == "{":
                self.value_types["table"] += 1
            elif tok == "true" or tok == "false":
                self.value_types["boolean"] += 1
            elif tok == "nil":
                self.value_types["nil"] += 1
            elif tok in ("'", '"'):
                self.value_types["string"] += 1
            else:
                self.value_types["number"] += 1
        else:
            self.value_types["expression"] += 1
        if len(self.examples) < 3:
            self.examples.append(f"{file}: {rhs.strip()[:80]}")

    def add_read(self, file):
        self.read_count += 1
        self.files.add(file)

    def mark_guarded(self):
        self.guarded = True


# ---------------------------------------------------------------------------
# 分析器
# ---------------------------------------------------------------------------

def classify_lhs(lhs):
    """把 LHS 分类为 (kind, 变量名)。"""
    parts = [p.strip() for p in lhs.split(".")]
    head = parts[0]
    if head in AUX_PREFIX:
        return "aux", ".".join(parts)
    if CARD_PREFIX_RE.match(head):
        return "card", ".".join(parts)
    if head == "_G":
        return "_G", ".".join(parts)
    if len(parts) > 1:
        return "other_table", ".".join(parts)
    return "bare", head


def analyze_file(path):
    """分析单个 Lua 文件，返回 (globals_dict, 统计)。"""
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return {}, Counter()

    locals_set = set()          # 已声明的局部变量（简化：不随作用域弹出）
    file_globals = {}           # name -> GlobalVar
    stats = Counter()

    def get_var(name, kind):
        if name not in file_globals:
            file_globals[name] = GlobalVar(name, kind)
        return file_globals[name]

    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not line or line.startswith("--"):
            continue
        # 去掉行尾注释（粗略：不处理字符串内的 --）
        code = re.sub(r"--.*$", "", line).strip()
        if not code:
            continue

        # 判断是否在函数内（缩进 > 0 近似）
        in_function = raw_line[:len(raw_line) - len(raw_line.lstrip())] != ""

        # local 声明
        m = LOCAL_RE.match(code)
        if m:
            for name in m.group("names").split(","):
                locals_set.add(name.strip())
            continue

        # 函数定义
        m = FUNCTION_DEF_RE.match(code)
        if m:
            fname = m.group("name")
            if "." in fname:
                kind, var = classify_lhs(fname)
                get_var(var, kind).add_assign(path.name, "function", in_function)
            else:
                if fname not in locals_set:
                    get_var(fname, "bare").add_assign(path.name, "function", in_function)
            pm = FUNC_PARAMS_RE.match(code)
            if pm and pm.group("params").strip():
                for p in pm.group("params").split(","):
                    p = p.strip()
                    if p and p != "...":
                        locals_set.add(p)
            continue

        # for 循环变量
        m = FOR_RE.match(code)
        if m:
            for name in m.group("names").split(","):
                locals_set.add(name.strip())
            continue

        # 赋值语句
        m = ASSIGN_RE.match(code)
        if m:
            lhs, rhs = m.group("lhs"), m.group("rhs")
            kind, var = classify_lhs(lhs)
            if kind == "bare" and var in locals_set:
                continue  # 局部变量
            gv = get_var(var, kind)
            gv.add_assign(path.name, rhs, in_function)
            continue

        # 一次性守卫模式：if not Auxiliary.xxx then
        m = GUARD_RE.match(code)
        if m:
            var = m.group("var")
            kind, name = classify_lhs(var)
            if kind in ("aux", "card", "other_table", "_G"):
                gv = get_var(name, kind)
                gv.mark_guarded()
            continue

        # 读取：统计全局表字段出现次数（粗略）
        for m in re.finditer(r"\b(?:Auxiliary|aux)\s*\.\s*([A-Za-z_]\w*)", code):
            get_var(f"Auxiliary.{m.group(1)}", "aux").add_read(path.name)
        for m in re.finditer(r"\b(c\d+)\s*\.\s*([A-Za-z_]\w*)", code):
            get_var(f"{m.group(1)}.{m.group(2)}", "card").add_read(path.name)

    return file_globals, stats


# ---------------------------------------------------------------------------
# 主流程
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="分析 Lua 全局变量")
    parser.add_argument("--script-dir", default=None, help="script 目录路径")
    parser.add_argument("--out", default=None, help="输出 Markdown 报告路径")
    parser.add_argument("--top", type=int, default=60, help="每个类别最多列出 N 个")
    args = parser.parse_args()

    script_dir = Path(args.script_dir) if args.script_dir else Path(__file__).resolve().parent.parent / "script"
    if not script_dir.is_dir():
        print(f"错误：script 目录不存在：{script_dir}", file=sys.stderr)
        sys.exit(1)

    all_globals = {}   # name -> GlobalVar
    file_count = 0

    for path in sorted(script_dir.glob("*.lua")):
        file_count += 1
        fg, _ = analyze_file(path)
        for name, gv in fg.items():
            if name not in all_globals:
                all_globals[name] = gv
            else:
                agg = all_globals[name]
                agg.assign_count += gv.assign_count
                agg.read_count += gv.read_count
                agg.files |= gv.files
                agg.in_function += gv.in_function
                agg.value_types += gv.value_types
                agg.guarded = agg.guarded or gv.guarded
                if len(agg.examples) < 3 and gv.examples:
                    agg.examples.extend(gv.examples[: 3 - len(agg.examples)])

    # 分类
    by_kind = defaultdict(list)
    for gv in all_globals.values():
        by_kind[gv.kind].append(gv)

    # 排序：按赋值次数降序
    for kind in by_kind:
        by_kind[kind].sort(key=lambda g: (-g.assign_count, g.name))

    # 生成报告
    lines = []
    lines.append("# Lua 全局变量分析报告")
    lines.append("")
    lines.append(f"- 分析文件数：{file_count}")
    lines.append(f"- 全局变量总数：{len(all_globals)}")
    lines.append("")
    lines.append("## 汇总")
    lines.append("")
    lines.append("| 类别 | 数量 | 说明 |")
    lines.append("|---|---|---|")
    lines.append(f"| aux | {len(by_kind['aux'])} | `Auxiliary.xxx` / `aux.xxx` 表字段 |")
    lines.append(f"| card | {len(by_kind['card'])} | `c{{code}}.xxx` 卡牌表字段 |")
    lines.append(f"| bare | {len(by_kind['bare'])} | 裸全局变量（非 local 赋值） |")
    lines.append(f"| _G | {len(by_kind['_G'])} | `_G.xxx` 显式全局 |")
    lines.append(f"| other_table | {len(by_kind['other_table'])} | 其他全局表字段 |")
    lines.append("")

    kind_titles = {
        "aux": "Auxiliary 表字段（per-field 语义候选）",
        "card": "卡牌表字段 c{code}.xxx",
        "bare": "裸全局变量（潜在污染源）",
        "_G": "_G 显式全局",
        "other_table": "其他全局表字段",
    }

    for kind in ("aux", "card", "bare", "_G", "other_table"):
        items = by_kind[kind]
        lines.append(f"## {kind_titles[kind]}（{len(items)} 个，显示前 {args.top}）")
        lines.append("")
        lines.append("| 变量 | 赋值次数 | 函数内赋值 | 读取次数 | 守卫 | 值类型 | 文件数 | 示例 |")
        lines.append("|---|---|---|---|---|---|---|---|")
        for gv in items[: args.top]:
            vtypes = ", ".join(f"{k}:{v}" for k, v in gv.value_types.most_common(4))
            guard = "✅" if gv.guarded else ""
            example = gv.examples[0] if gv.examples else ""
            lines.append(
                f"| `{gv.name}` | {gv.assign_count} | {gv.in_function} | {gv.read_count} "
                f"| {guard} | {vtypes} | {len(gv.files)} | `{example}` |"
            )
        lines.append("")

    # 一次性守卫变量（重点：这些是"只注册一次"的标记）
    guarded = [gv for gv in all_globals.values() if gv.guarded]
    guarded.sort(key=lambda g: (-g.assign_count, g.name))
    lines.append("## 一次性守卫变量（`if not X then X = ...` 模式）")
    lines.append("")
    lines.append("> 这些变量在战斗场克隆后不会重新触发注册，是隔离方案的关键对象。")
    lines.append("")
    lines.append("| 变量 | 类别 | 赋值次数 | 读取次数 | 文件数 |")
    lines.append("|---|---|---|---|---|")
    for gv in guarded:
        lines.append(f"| `{gv.name}` | {gv.kind} | {gv.assign_count} | {gv.read_count} | {len(gv.files)} |")
    lines.append("")

    # 函数内动态赋值（运行时创建的状态）
    dynamic = [gv for gv in all_globals.values() if gv.in_function > 0]
    dynamic.sort(key=lambda g: (-g.in_function, g.name))
    lines.append("## 函数内动态赋值（运行时创建/修改的全局状态）")
    lines.append("")
    lines.append("> 这些是效果执行期间写入的全局变量，静态分析无法预知，是隔离方案的最大盲区。")
    lines.append("")
    lines.append("| 变量 | 类别 | 函数内赋值 | 总赋值 | 读取次数 | 文件数 |")
    lines.append("|---|---|---|---|---|---|")
    for gv in dynamic[: args.top]:
        lines.append(f"| `{gv.name}` | {gv.kind} | {gv.in_function} | {gv.assign_count} | {gv.read_count} | {len(gv.files)} |")
    lines.append("")

    report = "\n".join(lines)

    if args.out:
        out_path = Path(args.out)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(report, encoding="utf-8")
        print(f"报告已写入：{out_path}")
    else:
        print(report)

    # 控制台摘要
    print("\n===== 摘要 =====")
    print(f"文件数: {file_count}, 全局变量总数: {len(all_globals)}")
    for kind in ("aux", "card", "bare", "_G", "other_table"):
        print(f"  {kind}: {len(by_kind[kind])}")
    print(f"一次性守卫变量: {len(guarded)}")
    print(f"函数内动态赋值变量: {len(dynamic)}")


if __name__ == "__main__":
    main()