#!/usr/bin/env python3
"""Check Intel CAT (Cache Allocation Technology) availability."""
import subprocess, os, sys

print("=== Intel CAT / RDT Feature Check ===\n")

# 1. Check CPU model
print("--- CPU Info ---")
try:
    r = subprocess.run(['lscpu'], capture_output=True, text=True)
    for line in r.stdout.split('\n'):
        if any(k in line.lower() for k in ['model name','cpu(s)','socket','cache','numa']):
            print(f"  {line.strip()}")
except: print("  lscpu failed")

# 2. Check /proc/cpuinfo for cat_l3, mba, cqm flags
print("\n--- RDT CPU Flags ---")
try:
    with open('/proc/cpuinfo','r') as f:
        for line in f:
            if line.startswith('flags'):
                flags = line.split(':')[1].strip().split()
                rdt_flags = [f for f in flags if any(k in f for k in
                    ['cat_l3','cat_l2','mba','cqm','rdt','cdp_l3'])]
                if rdt_flags:
                    print(f"  RDT flags found: {' '.join(rdt_flags)}")
                else:
                    print("  No RDT flags found in cpuinfo")
                    # Check broader
                    possible = [f for f in flags if 'cat' in f or 'rdt' in f or 'mba' in f]
                    if possible:
                        print(f"  Possible related: {' '.join(possible)}")
                break
except: print("  Cannot read /proc/cpuinfo")

# 3. Check resctrl filesystem
print("\n--- resctrl Filesystem ---")
resctrl = '/sys/fs/resctrl'
if os.path.exists(resctrl):
    print(f"  {resctrl} EXISTS (CAT likely available!)")
    try:
        for item in os.listdir(resctrl):
            p = os.path.join(resctrl, item)
            if os.path.isfile(p):
                with open(p,'r') as f:
                    content = f.read().strip()[:200]
                print(f"  {item}: {content}")
    except Exception as e:
        print(f"  Cannot read resctrl: {e}")
else:
    print(f"  {resctrl} NOT FOUND")
    print("  Try: mount -t resctrl resctrl /sys/fs/resctrl")

# 4. Check L3 cache info
print("\n--- L3 Cache Details ---")
try:
    cache_dir = '/sys/devices/system/cpu/cpu0/cache'
    if os.path.exists(cache_dir):
        for idx in os.listdir(cache_dir):
            idx_path = os.path.join(cache_dir, idx)
            if os.path.isdir(idx_path):
                try:
                    level = open(os.path.join(idx_path,'level')).read().strip()
                    ctype = open(os.path.join(idx_path,'type')).read().strip()
                    size = open(os.path.join(idx_path,'size')).read().strip()
                    ways = open(os.path.join(idx_path,'ways_of_associativity')).read().strip()
                    print(f"  {idx}: L{level} {ctype}, {size}, {ways}-way")
                except: pass
except: print("  Cannot read cache info")

# 5. Check pqos tool
print("\n--- pqos Tool ---")
try:
    r = subprocess.run(['which','pqos'], capture_output=True, text=True)
    if r.returncode == 0:
        print(f"  pqos found: {r.stdout.strip()}")
        r2 = subprocess.run(['pqos','-d'], capture_output=True, text=True, timeout=5)
        print(f"  pqos -d output (first 500 chars):")
        print(f"  {r2.stdout[:500]}")
    else:
        print("  pqos not found")
except Exception as e:
    print(f"  pqos check failed: {e}")

# 6. Check rdtset tool
print("\n--- rdtset Tool ---")
try:
    r = subprocess.run(['which','rdtset'], capture_output=True, text=True)
    if r.returncode == 0:
        print(f"  rdtset found: {r.stdout.strip()}")
    else:
        print("  rdtset not found")
except: print("  rdtset check failed")

# 7. Check perf support for cache events
print("\n--- perf Cache Events ---")
try:
    r = subprocess.run(['perf','list'], capture_output=True, text=True, timeout=5)
    cache_events = [l.strip() for l in r.stdout.split('\n')
                    if 'cache' in l.lower() and ('miss' in l.lower() or 'hit' in l.lower())]
    print(f"  Cache events available: {len(cache_events)}")
    for e in cache_events[:10]:
        print(f"    {e}")
except: print("  perf list failed")

print("\n=== Check Complete ===")
