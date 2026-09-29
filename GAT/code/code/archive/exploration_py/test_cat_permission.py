#!/usr/bin/env python3
"""
Intel CAT (Cache Allocation Technology) Permission & Functionality Test.

Tests:
  1. Can we read resctrl?
  2. Can we create CLOS (resource groups)?
  3. Can we modify schemata (cache way allocation)?
  4. Can we assign tasks to CLOS?
  5. Measure actual cache isolation effect with a micro-benchmark.
"""

import os, sys, time, subprocess, struct

RESCTRL = "/sys/fs/resctrl"

def read_file(path):
    """读取文件内容，失败返回 None"""
    try:
        with open(path, 'r') as f:
            return f.read().strip()
    except Exception as e:
        return f"ERROR: {e}"

def write_file(path, content):
    """写入文件，返回 (success, message)"""
    try:
        with open(path, 'w') as f:
            f.write(content)
        return True, "OK"
    except Exception as e:
        return False, str(e)

def test_read_resctrl():
    """Test 1: 能否读取 resctrl 信息"""
    print("=" * 60)
    print("TEST 1: Read resctrl filesystem")
    print("=" * 60)

    if not os.path.exists(RESCTRL):
        print("  FAIL: /sys/fs/resctrl does not exist")
        print("  → resctrl 未挂载，需要 root: mount -t resctrl resctrl /sys/fs/resctrl")
        return False

    print(f"  /sys/fs/resctrl EXISTS ✓")

    # 读取关键文件
    files_to_check = ['schemata', 'cpus', 'cpus_list', 'tasks', 'mode']
    for fn in files_to_check:
        path = os.path.join(RESCTRL, fn)
        content = read_file(path)
        if content and not content.startswith("ERROR"):
            # 截断过长内容
            display = content[:200] + "..." if len(content) > 200 else content
            print(f"  {fn}: {display}")
        else:
            print(f"  {fn}: {content}")

    # 读取 info 目录
    info_dir = os.path.join(RESCTRL, "info")
    if os.path.exists(info_dir):
        print(f"\n  --- resctrl/info ---")
        for subdir in sorted(os.listdir(info_dir)):
            subpath = os.path.join(info_dir, subdir)
            if os.path.isdir(subpath):
                print(f"  [{subdir}]")
                for fn in sorted(os.listdir(subpath)):
                    content = read_file(os.path.join(subpath, fn))
                    if content and not content.startswith("ERROR"):
                        print(f"    {fn}: {content}")

    return True

def test_create_clos():
    """Test 2: 能否创建 CLOS (资源组)"""
    print("\n" + "=" * 60)
    print("TEST 2: Create CLOS (resource group)")
    print("=" * 60)

    test_dir = os.path.join(RESCTRL, "spmm_test_clos")

    # 清理可能残留的测试目录
    if os.path.exists(test_dir):
        try:
            os.rmdir(test_dir)
            print(f"  Cleaned up old test dir")
        except:
            pass

    try:
        os.mkdir(test_dir)
        print(f"  mkdir {test_dir}: SUCCESS ✓")
        print(f"  → 我们有权限创建 CLOS!")

        # 读取新 CLOS 的默认 schemata
        schemata = read_file(os.path.join(test_dir, "schemata"))
        print(f"  Default schemata: {schemata}")

        return True, test_dir
    except PermissionError as e:
        print(f"  mkdir {test_dir}: PERMISSION DENIED ✗")
        print(f"  Error: {e}")
        print(f"  → 需要 root 权限或管理员配置")
        return False, None
    except Exception as e:
        print(f"  mkdir {test_dir}: FAILED ✗")
        print(f"  Error: {e}")
        return False, None

def test_modify_schemata(test_dir):
    """Test 3: 能否修改 cache way 分配"""
    print("\n" + "=" * 60)
    print("TEST 3: Modify cache way allocation")
    print("=" * 60)

    if test_dir is None:
        print("  SKIP: No CLOS created")
        return False

    schemata_path = os.path.join(test_dir, "schemata")

    # 读取当前 info 确定有多少 way
    info_l3 = os.path.join(RESCTRL, "info", "L3")
    if os.path.exists(info_l3):
        cbm = read_file(os.path.join(info_l3, "cbm_mask"))
        min_cbm = read_file(os.path.join(info_l3, "min_cbm_bits"))
        num_closids = read_file(os.path.join(info_l3, "num_closids"))
        print(f"  L3 CBM mask: {cbm} (可用 way 的 bitmask)")
        print(f"  L3 min CBM bits: {min_cbm} (最少分配几个 way)")
        print(f"  L3 num CLOSIDs: {num_closids} (最多几个资源组)")

    # 尝试设置: 只用前 2 个 way (bitmask = 0x3)
    # 对双 socket: "L3:0=3;1=3"
    # 先读当前值确定 socket 数
    cur_schemata = read_file(schemata_path)
    print(f"  Current schemata: {cur_schemata}")

    # 解析 socket 数量
    if "L3:" in cur_schemata:
        parts = cur_schemata.split("L3:")[-1].strip().split(";")
        nsockets = len(parts)
        print(f"  Detected {nsockets} socket(s)")

        # 构造测试 schemata: 每个 socket 只用 way 0-1
        new_schema = "L3:" + ";".join([f"{i}=3" for i in range(nsockets)])
        print(f"  Attempting: {new_schema}")

        ok, msg = write_file(schemata_path, new_schema)
        if ok:
            verify = read_file(schemata_path)
            print(f"  Write SUCCESS ✓")
            print(f"  Verified: {verify}")

            # 恢复默认 (全部 way 开放)
            full_mask = parts[0].split("=")[1]
            restore = "L3:" + ";".join([f"{i}={full_mask}" for i in range(nsockets)])
            write_file(schemata_path, restore)
            print(f"  Restored to default")
            return True
        else:
            print(f"  Write FAILED ✗: {msg}")
            return False
    else:
        print(f"  Cannot parse schemata format")
        return False

def test_assign_task(test_dir):
    """Test 4: 能否把当前进程分配到 CLOS"""
    print("\n" + "=" * 60)
    print("TEST 4: Assign task to CLOS")
    print("=" * 60)

    if test_dir is None:
        print("  SKIP: No CLOS created")
        return False

    tasks_path = os.path.join(test_dir, "tasks")
    my_pid = str(os.getpid())
    print(f"  My PID: {my_pid}")

    ok, msg = write_file(tasks_path, my_pid)
    if ok:
        tasks = read_file(tasks_path)
        print(f"  Assign SUCCESS ✓")
        print(f"  Tasks in CLOS: {tasks[:200]}")

        # 移回默认组
        default_tasks = os.path.join(RESCTRL, "tasks")
        write_file(default_tasks, my_pid)
        print(f"  Moved back to default group")
        return True
    else:
        print(f"  Assign FAILED ✗: {msg}")
        return False

def test_l2_cat():
    """Test 5: L2 CAT 是否也可用"""
    print("\n" + "=" * 60)
    print("TEST 5: L2 CAT availability")
    print("=" * 60)

    info_l2 = os.path.join(RESCTRL, "info", "L2")
    if os.path.exists(info_l2):
        print(f"  L2 CAT info EXISTS ✓")
        for fn in sorted(os.listdir(info_l2)):
            content = read_file(os.path.join(info_l2, fn))
            print(f"    {fn}: {content}")
        return True
    else:
        print(f"  L2 CAT info NOT FOUND (L2 分区不可用或未启用)")
        return False

def test_mba():
    """Test 6: MBA (Memory Bandwidth Allocation) 是否可用"""
    print("\n" + "=" * 60)
    print("TEST 6: MBA (Memory Bandwidth Allocation)")
    print("=" * 60)

    info_mb = os.path.join(RESCTRL, "info", "MB")
    if os.path.exists(info_mb):
        print(f"  MBA info EXISTS ✓")
        for fn in sorted(os.listdir(info_mb)):
            content = read_file(os.path.join(info_mb, fn))
            print(f"    {fn}: {content}")
        return True
    else:
        print(f"  MBA info NOT FOUND")
        return False

def cleanup(test_dir):
    """清理测试用的 CLOS"""
    if test_dir and os.path.exists(test_dir):
        try:
            os.rmdir(test_dir)
            print(f"\n  Cleanup: removed {test_dir} ✓")
        except Exception as e:
            print(f"\n  Cleanup failed: {e}")

def print_summary(results):
    """打印总结"""
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    for name, status in results.items():
        icon = "✓" if status else "✗"
        print(f"  [{icon}] {name}")

    print("\n  --- Implications for SpMM ---")
    if all(results.values()):
        print("  ★★★★★ FULL CAT SUPPORT!")
        print("  We can:")
        print("    1. Create dedicated L3 partition for hot B-rows")
        print("    2. Pin preload thread to hot partition")
        print("    3. Isolate AMX/fallback from evicting hot data")
        print("    4. Potentially use L2 CAT for per-core isolation")
        print("    5. Use MBA to control memory bandwidth allocation")
    elif results.get("Read resctrl"):
        if not results.get("Create CLOS"):
            print("  resctrl visible but no write permission")
            print("  → Ask admin to: chmod g+w /sys/fs/resctrl")
            print("  → Or run with: sudo or CAP_SYS_ADMIN capability")
        else:
            print("  Partial support — some features available")
    else:
        print("  No CAT support or resctrl not mounted")
        print("  → Alternative: rely on panel reordering + hardware LRU")

if __name__ == "__main__":
    print("=== Intel CAT Permission & Functionality Test ===")
    print(f"Node: {os.uname().nodename}, PID: {os.getpid()}")
    print(f"Time: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print()

    results = {}

    # Test 1: Read
    results["Read resctrl"] = test_read_resctrl()

    # Test 2: Create CLOS
    can_create, test_dir = test_create_clos()
    results["Create CLOS"] = can_create

    # Test 3: Modify schemata
    results["Modify schemata"] = test_modify_schemata(test_dir)

    # Test 4: Assign tasks
    results["Assign tasks"] = test_assign_task(test_dir)

    # Test 5: L2 CAT
    results["L2 CAT"] = test_l2_cat()

    # Test 6: MBA
    results["MBA"] = test_mba()

    # Cleanup
    cleanup(test_dir)

    # Summary
    print_summary(results)
    print("\n=== Test Complete ===")
