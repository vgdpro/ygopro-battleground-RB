#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分析 ygopro script/ 目录下所有 Lua 脚本的全局变量使用情况。

用法:
    python analyze_lua_globals.py [--limit N] [--output report.md] [--files f1 f2 ...]

输出:
    - 顶层全局变量定义（文件加载时执行）
    - 全局表字段赋值（Auxiliary.*, c{code}.* 等）
    - 函数内动态全局赋值（运行时执行）
    - 一次性守卫模式（if not X then ... X = ...）
    - 读取统计

原理: 词法分析（不执行 Lua），跟踪 local 作用域与函数嵌套深度，
      识别非 local 的赋值/读取。注释与字符串内容被跳过。
"""

import os
import re
import sys
from collections import Counter

SCRIPT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "script")

LUA_KEYWORDS = {
    'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for', 'function',
    'goto', 'if', 'in', 'local', 'nil', 'not', 'or', 'repeat', 'return',
    'then', 'true', 'until', 'while',
}


# ============ Lua 词法分析 ============

def lua_tokens(src):
    """将 Lua 源码切成 token 流。
    返回 [(type, value, line)]，type 为 'id'/'num'/'str'/'op'。
    跳过注释；字符串作为 'str' token 保留。
    """
    tokens = []
    i, n, line = 0, len(src), 1
    while i < n:
        c = src[i]
        if c == '\n':
            line += 1
            i += 1
            continue
        if c in ' \t\r':
            i += 1
            continue
        # 注释
        if c == '-' and i + 1 < n and src[i + 1] == '-':
            m = re.match(r'--\[(=*)\[', src[i:])
            if m:
                close = ']' + m.group(1) + ']'
                end = src.find(close, i + m.end())
                if end != -1:
                    line += src[i:end].count('\n')
                    i = end + len(close)
                    continue
            end = src.find('\n', i)
            if end == -1:
                break
            line += 1
            i = end + 1
            continue
        # 字符串 '...' "..."
        if c in ('"', "'"):
            q = c
            j = i + 1
            while j < n:
                if src[j] == '\\':
                    j += 2
                    continue
                if src[j] == q:
                    break
                if src[j] == '\n':
                    line += 1
                j += 1
            tokens.append(('str', src[i:j + 1], line))
            i = j + 1
            continue
        # 长字符串 [[...]]
        if c == '[':
            m = re.match(r'\[(=*)\[', src[i:])
            if m:
                close = ']' + m.group(1) + ']'
                end = src.find(close, i + m.end())
                if end != -1:
                    line += src[i:end].count('\n')
                    tokens.append(('str', src[i:end + len(close)], line))
                    i = end + len(close)
                    continue
        # 数字
        if c.isdigit() or (c == '.' and i + 1 < n and src[i + 1].isdigit()):
            m = re.match(r'0[xX][0-9a-fA-F]+|(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?', src[i:])
            if m:
                tokens.append(('num', m.group(0), line))
                i += m.end()
                continue
        # 标识符
        if c.isalpha() or c == '_':
            m = re.match(r'[A-Za-z_][A-Za-z0-9_]*', src[i:])
            tokens.append(('id', m.group(0), line))
            i += m.end()
            continue
        # 多字符运算符
        matched = False
        for op in ('~=', '==', '<=', '>=', '..', '...', '<<', '>>', '//'):
            if src.startswith(op, i):
                tokens.append(('op', op, line))
                i += len(op)
                matched = True
                break
        if matched:
            continue
        tokens.append(('op', c, line))
        i += 1
    return tokens


def peek_value(tokens, i):
    """推断赋值右侧值的类型。"""
    if i >= len(tokens):
        return '?'
    t = tokens[i]
    if t[0] == 'op' and t[1] == '{':
        return 'table'
    if t[0] == 'id':
        if t[1] == 'function':
            return 'function'
        if t[1] == 'nil':
            return 'nil'
        if t[1] in ('true', 'false'):
            return 'boolean'
        parts = [t[1]]
        j = i + 1
        while j < len(tokens) and tokens[j][0] == 'op' and tokens[j][1] == '.' \
                and j + 1 < len(tokens) and tokens[j + 1][0] == 'id':
            parts.append(tokens[j + 1][1])
            j += 2
        if j < len(tokens) and tokens[j][0] == 'op' and tokens[j][1] == '(':
            return '.'.join(parts) + '()'
        return '.'.join(parts)
    if t[0] == 'num':
        return 'number'
    if t[0] == 'str':
        return 'string'
    return t[1]


def skip_func_params(tokens, i, func_locals):
    """跳过函数参数列表 (a, b, ...)，把参数加入 func_locals[-1]。
    返回 ')' 之后的下标。"""
    if i >= len(tokens) or not (tokens[i][0] == 'op' and tokens[i][1] == '('):
        return i
    depth = 0
    while i < len(tokens):
        kind, val, _ = tokens[i]
        if kind == 'op' and val == '(':
            depth += 1
        elif kind == 'op' and val == ')':
            depth -= 1
            if depth == 0:
                return i + 1
        elif kind == 'id' and depth == 1:
            # 参数名（含 ... 可变参数）
            if val != '...':
                func_locals[-1].add(val)
        i += 1
    return i


# ============ 单文件分析 ============

def analyze_lua(src, path):
    tokens = lua_tokens(src)
    n = len(tokens)
    result = {
        'path': path,
        'top_assigns': [],    # (name, line, value_hint)
        'field_assigns': [],  # (path_str, line, value_hint, in_func)
        'func_assigns': [],   # (name, line, value_hint)
        'guards': [],         # (path_str, line, negated)
        'reads': Counter(),
        'func_defs': [],      # (path_str, line)
    }
    func_depth = 0
    file_locals = set()
    func_locals = [set()]  # 栈，[0] 为文件级
    block_stack = []       # 'function' | 'if' | 'for' | 'while' | 'do' | 'repeat'

    def is_local(name):
        return name in file_locals or any(name in s for s in func_locals[1:])

    i = 0
    while i < n:
        kind, val, line = tokens[i]
        if kind != 'id':
            i += 1
            continue
        if val == 'local':
            j = i + 1
            if j < n and tokens[j][0] == 'id' and tokens[j][1] == 'function':
                # local function f(params) ... end
                j += 1
                if j < n and tokens[j][0] == 'id':
                    func_locals[-1].add(tokens[j][1])
                    func_depth += 1
                    func_locals.append(set())
                    block_stack.append('function')
                    j = skip_func_params(tokens, j, func_locals)
                    i = j
                    continue
            # local a, b = ...
            while j < n and tokens[j][0] == 'id':
                func_locals[-1].add(tokens[j][1])
                j += 1
                if j < n and tokens[j][0] == 'op' and tokens[j][1] == ',':
                    j += 1
                else:
                    break
            i = j
            continue
        if val == 'function':
            func_depth += 1
            func_locals.append(set())
            block_stack.append('function')
            j = i + 1
            parts = []
            while j < n and tokens[j][0] == 'id':
                parts.append(tokens[j][1])
                j += 1
                if j < n and tokens[j][0] == 'op' and tokens[j][1] == '.':
                    j += 1
                    continue
                break
            if parts and not is_local(parts[0]):
                path_str = '.'.join(parts)
                result['func_defs'].append((path_str, line))
                if len(parts) > 1:
                    result['field_assigns'].append((path_str, line, 'function', func_depth > 1))
            j = skip_func_params(tokens, j, func_locals)
            i = j
            continue
        if val == 'if':
            block_stack.append('if')
            j = i + 1
            cond = []
            while j < n and not (tokens[j][0] == 'id' and tokens[j][1] == 'then'):
                cond.append(tokens[j])
                j += 1
            for k in range(len(cond)):
                if cond[k][0] == 'id' and cond[k][1] not in LUA_KEYWORDS and not is_local(cond[k][1]):
                    # 跳过字段访问 (x.y) 和方法调用 (x:y) 的后续 id
                    if k > 0 and cond[k - 1][0] == 'op' and cond[k - 1][1] in ('.', ':'):
                        continue
                    parts = [cond[k][1]]
                    m = k + 1
                    while m < len(cond) and cond[m][0] == 'op' and cond[m][1] == '.' \
                            and m + 1 < len(cond) and cond[m + 1][0] == 'id':
                        parts.append(cond[m + 1][1])
                        m += 2
                    has_neg = any(c[0] == 'id' and c[1] == 'not' for c in cond) or \
                              any(c[0] == 'op' and c[1] in ('~=', '==') for c in cond)
                    result['guards'].append(('.'.join(parts), line, has_neg))
                    break
            i = j
            continue
        if val in ('for', 'while', 'do'):
            block_stack.append(val)
            j = i + 1
            if val == 'for':
                # 收集循环变量：数值 for 的 '=' 前 / 泛型 for 的 'in' 前
                while j < n:
                    if tokens[j][0] == 'id' and tokens[j][1] in ('in', 'do'):
                        break
                    if tokens[j][0] == 'op' and tokens[j][1] == '=':
                        break
                    if tokens[j][0] == 'id' and tokens[j][1] not in LUA_KEYWORDS:
                        func_locals[-1].add(tokens[j][1])
                    j += 1
            i = j
            continue
        if val == 'repeat':
            block_stack.append('repeat')
            i += 1
            continue
        if val == 'until':
            if block_stack and block_stack[-1] == 'repeat':
                block_stack.pop()
            i += 1
            continue
        if val in ('elseif', 'else'):
            # 不改变块深度（if 的 end 只出现一次）
            i += 1
            continue
        if val == 'end':
            if block_stack:
                top = block_stack.pop()
                if top == 'function':
                    func_depth -= 1
                    func_locals.pop()
            i += 1
            continue
        # 赋值 / 读取
        name = val
        if name in LUA_KEYWORDS:
            i += 1
            continue
        j = i + 1
        parts = [name]
        while j < n:
            if tokens[j][0] == 'op' and tokens[j][1] == '.' and j + 1 < n and tokens[j + 1][0] == 'id':
                parts.append(tokens[j + 1][1])
                j += 2
                continue
            if tokens[j][0] == 'op' and tokens[j][1] == '[':
                depth = 1
                j += 1
                while j < n and depth > 0:
                    if tokens[j][0] == 'op' and tokens[j][1] == '[':
                        depth += 1
                    elif tokens[j][0] == 'op' and tokens[j][1] == ']':
                        depth -= 1
                    j += 1
                parts.append('[]')
                continue
            break
        # 方法调用 c:Method(...) —— 方法名不是全局变量
        if j < n and tokens[j][0] == 'op' and tokens[j][1] == ':':
            i = j + 1
            continue
        is_global = not is_local(name)
        if j < n and tokens[j][0] == 'op' and tokens[j][1] == '=':
            vh = peek_value(tokens, j + 1)
            if is_global:
                if len(parts) == 1:
                    if func_depth == 0:
                        result['top_assigns'].append((name, line, vh))
                    else:
                        result['func_assigns'].append((name, line, vh))
                else:
                    result['field_assigns'].append(('.'.join(parts), line, vh, func_depth > 0))
            i = j + 1
            continue
        if is_global:
            result['reads']['.'.join(parts)] += 1
        i = j
    return result


# ============ 报告生成 ============

# 过滤规则：
# - 纯函数：所有赋值都是 function → 函数不需要克隆，战斗时重新映射参数即可
# - 纯常量：所有赋值都是 number/string 且无函数内赋值 → 常量无 per-field 语义
# - 常量别名：值类型是标识符（如 Auxiliary.NegateAnyFilter、CATEGORY_HANDES_SELF）且无函数内赋值
# - 只读常量表：值类型是 table 且顶层定义、无函数内赋值、无守卫
# - 占位符：值类型是 nil 且无函数内赋值
# - 纯读取：无定义无赋值（方法名误报，如 Card.GetAttack）→ 过滤
def is_pure_function(v):
    return bool(v['value_types']) and all(t == 'function' for t in v['value_types'])

def is_pure_constant(v):
    if not v['value_types'] or v['in_func']:
        return False
    return all(t in ('number', 'string') for t in v['value_types'])

def is_constant_alias(v):
    if not v['value_types'] or v['in_func']:
        return False
    # 标识符别名：值类型含 '.' 或全大写（如 Auxiliary.NegateAnyFilter、CATEGORY_HANDES_SELF）
    return all(('.' in t or t.isupper() or t == 'nil') for t in v['value_types'])

def is_readonly_table(v):
    if not v['value_types'] or v['in_func'] or v['guards']:
        return False
    return all(t == 'table' for t in v['value_types'])

def is_read_only(v):
    return not v['defs'] and not v['assigns']

def should_keep(v):
    return not (is_pure_function(v) or is_pure_constant(v) or is_constant_alias(v)
                or is_readonly_table(v) or is_read_only(v))


def build_report(results):
    vars_info = {}

    def get_var(name):
        if name not in vars_info:
            vars_info[name] = {
                'defs': [], 'assigns': [], 'guards': [], 'reads': 0,
                'value_types': set(), 'in_func': False, 'files': set(),
            }
        return vars_info[name]

    for r in results:
        for name, line, vh in r['top_assigns']:
            v = get_var(name)
            v['defs'].append((r['path'], line))
            v['value_types'].add(vh)
            v['files'].add(r['path'])
        for path_str, line, vh, in_func in r['field_assigns']:
            v = get_var(path_str)
            v['assigns'].append((r['path'], line, in_func))
            v['value_types'].add(vh)
            v['in_func'] = v['in_func'] or in_func
            v['files'].add(r['path'])
        for name, line, vh in r['func_assigns']:
            v = get_var(name)
            v['assigns'].append((r['path'], line, True))
            v['value_types'].add(vh)
            v['in_func'] = True
            v['files'].add(r['path'])
        for path_str, line, _neg in r['guards']:
            v = get_var(path_str)
            v['guards'].append((r['path'], line))
            v['files'].add(r['path'])
        for path_str, count in r['reads'].items():
            v = get_var(path_str)
            v['reads'] += count
            v['files'].add(r['path'])

    # 过滤函数/常量/纯读取
    kept = {k: v for k, v in vars_info.items() if should_keep(v)}
    filtered = len(vars_info) - len(kept)

    aux_fields = {k: v for k, v in kept.items() if k.startswith('Auxiliary.')}
    card_fields = {k: v for k, v in kept.items() if re.match(r'^c\d+\.', k)}
    other_fields = {k: v for k, v in kept.items()
                    if '.' in k and k not in aux_fields and k not in card_fields}
    top_vars = {k: v for k, v in kept.items() if '.' not in k}

    L = []
    L.append('# Lua 全局变量分析报告（仅可变状态）')
    L.append('')
    L.append(f'- 分析文件数: {len(results)}')
    L.append(f'- 全局变量总数: {len(vars_info)}')
    L.append(f'- 过滤函数/常量/纯读取: {filtered}')
    L.append(f'- 保留可变状态: {len(kept)}')
    L.append('')
    L.append('> 过滤规则：纯函数（值类型全为 function）不需要克隆，战斗时重新映射参数即可；')
    L.append('> 纯常量（值类型全为 number/string 且无函数内赋值）无 per-field 语义；')
    L.append('> 纯读取（无定义无赋值，如方法名误报）一并过滤。')
    L.append('')
    L.append('## 汇总')
    L.append('')
    L.append('| 类别 | 数量 |')
    L.append('|---|---|')
    L.append(f'| 顶层全局变量 | {len(top_vars)} |')
    L.append(f'| Auxiliary.* 字段 | {len(aux_fields)} |')
    L.append(f'| c{{code}}.* 字段 | {len(card_fields)} |')
    L.append(f'| 其他表字段 | {len(other_fields)} |')
    L.append('')

    def fmt_var(name, v):
        locs = []
        for f, ln in v['defs']:
            locs.append(f'{f}:{ln}(def)')
        for f, ln, inf in v['assigns']:
            locs.append(f'{f}:{ln}({"func" if inf else "top"})')
        for f, ln in v['guards']:
            locs.append(f'{f}:{ln}(guard)')
        guard = '✅' if v['guards'] else ''
        dyn = '🔶' if v['in_func'] else ''
        return (f'| `{name}` | {", ".join(locs) or "-"} | '
                f'{", ".join(sorted(v["value_types"])) or "-"} | '
                f'{len(v["assigns"])} | {v["reads"]} | {dyn}{guard} |')

    L.append('## 顶层全局变量（可变状态）')
    L.append('')
    L.append('| 变量 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |')
    L.append('|---|---|---|---|---|---|')
    for name in sorted(top_vars):
        L.append(fmt_var(name, top_vars[name]))
    L.append('')

    L.append('## Auxiliary.* 字段（可变状态）')
    L.append('')
    L.append('| 字段 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |')
    L.append('|---|---|---|---|---|---|')
    for name in sorted(aux_fields):
        L.append(fmt_var(name, aux_fields[name]))
    L.append('')

    L.append('## c{code}.* 字段（可变状态）')
    L.append('')
    L.append('| 字段 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |')
    L.append('|---|---|---|---|---|---|')
    for name in sorted(card_fields):
        L.append(fmt_var(name, card_fields[name]))
    L.append('')

    L.append('## 其他表字段（可变状态）')
    L.append('')
    L.append('| 字段 | 位置 | 值类型 | 赋值次数 | 读取次数 | 动态/守卫 |')
    L.append('|---|---|---|---|---|---|')
    for name in sorted(other_fields):
        L.append(fmt_var(name, other_fields[name]))
    L.append('')

    # 一次性守卫清单（重点）
    guarded = {k: v for k, v in kept.items() if v['guards']}
    L.append('## 一次性守卫模式（重点：跨 field 需重置）')
    L.append('')
    L.append('| 变量 | 守卫位置 | 赋值位置 | 值类型 |')
    L.append('|---|---|---|---|')
    for name in sorted(guarded):
        v = guarded[name]
        g = ', '.join(f'{f}:{ln}' for f, ln in v['guards'])
        a = ', '.join(f'{f}:{ln}' for f, ln, _ in v['assigns']) or \
            ', '.join(f'{f}:{ln}' for f, ln in v['defs'])
        L.append(f'| `{name}` | {g} | {a} | {", ".join(sorted(v["value_types"])) or "-"} |')
    L.append('')

    return '\n'.join(L), kept


def main():
    import argparse
    parser = argparse.ArgumentParser(description='分析 ygopro Lua 脚本全局变量')
    parser.add_argument('--limit', type=int, default=0, help='只分析前 N 个文件（0=全部）')
    parser.add_argument('--output', default='lua_globals_report.md', help='输出报告路径')
    parser.add_argument('--files', nargs='*', help='只分析指定文件（相对 script/）')
    args = parser.parse_args()

    if args.files:
        files = [f if os.path.isabs(f) else os.path.join(SCRIPT_DIR, f) for f in args.files]
    else:
        files = sorted(os.path.join(SCRIPT_DIR, f) for f in os.listdir(SCRIPT_DIR) if f.endswith('.lua'))
        if args.limit:
            files = files[:args.limit]

    results = []
    for path in files:
        try:
            with open(path, 'r', encoding='utf-8', errors='replace') as fh:
                src = fh.read()
        except OSError as e:
            print(f'[ERROR] {os.path.basename(path)}: {e}', file=sys.stderr)
            continue
        results.append(analyze_lua(src, os.path.basename(path)))

    report, vars_info = build_report(results)
    with open(args.output, 'w', encoding='utf-8') as fh:
        fh.write(report)

    # 控制台摘要
    print(f'分析完成: {len(results)} 个文件, {len(vars_info)} 个全局变量')
    print(f'报告已写入: {args.output}')
    guarded = [k for k, v in vars_info.items() if v['guards']]
    print(f'保留可变状态: {len(vars_info)} 个')
    print(f'一次性守卫变量: {len(guarded)} 个')
    for g in sorted(guarded):
        print(f'  - {g}')


if __name__ == '__main__':
    main()