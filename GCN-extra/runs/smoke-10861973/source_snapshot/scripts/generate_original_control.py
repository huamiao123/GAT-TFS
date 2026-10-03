#!/usr/bin/env python3
import hashlib, pathlib
root=pathlib.Path(__file__).resolve().parents[1]
src=(root/'original/amx_tfs_v3.cpp').read_bytes()
assert hashlib.sha256(src).hexdigest()=='4e574786055816df80beb6e68181fe19a8ee02cf4be1d43717bc1e79b88cab29'
text=src.decode().replace('\r\n','\n')
function=text[text.index('static void tfs_v3('):text.index('static void ref_twostep(')]
function=function[:function.rfind('/*')]
needle='    memset(C, 0, (size_t)N * K_OUT * sizeof(float));\n'
assert function.count(needle)==1
function=function.replace('static void tfs_v3(', 'static void tfs_v3_nozero(',1).replace(needle,'    // Redundant full-C memset removed; every row/panel is stored below.\n',1)
(root/'src/original_nozero.hpp').write_text('// Generated from unchanged Original TFS; only C memset removed.\n'+function)
print(root/'src/original_nozero.hpp')
