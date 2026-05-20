#!/bin/bash
set -e

cd "$(dirname "$0")"

echo "=== Building LVGL + Camera ==="
make -j$(nproc)

BINARY=build/bin/main
echo "Binary: $(ls -lh ${BINARY} | awk '{print $5, $9}')"

echo ""
echo "=== Pushing binary to device ==="

adb push ${BINARY} /usr/main
adb shell "chmod +x /usr/main && echo '[OK] Binary installed: /usr/main'"

echo ""
echo "=== Setting up autostart ==="

adb push scripts/S99main /etc/init.d/S99main
adb shell "chmod +x /etc/init.d/S99main"

# Enable the service: procd uses symlinks in /etc/rc.d/
# Fallback: create the symlink manually if 'enable' subcommand is not available
adb shell '
if /etc/init.d/S99main enable 2>/dev/null; then
    echo "[OK] Service enabled via procd (symlink created in /etc/rc.d/)"
else
    ln -sf /etc/init.d/S99main /etc/rc.d/S99main
    echo "[OK] Symlink created: /etc/rc.d/S99main"
fi
'

echo ""
echo "=== Autostart configured ==="
echo "Starting main on device now..."
adb shell "killall main 2>/dev/null; /usr/main &"
