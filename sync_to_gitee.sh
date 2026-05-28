#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

echo "[INFO] Run pre-sync cleanup..."
echo "[INFO] Cleanup done"

if ! command -v git &>/dev/null; then
    echo "[ERROR] Git not found in PATH"
    exit 1
fi

if ! git rev-parse --is-inside-work-tree &>/dev/null; then
    echo "[ERROR] Current folder is not a Git repository"
    exit 1
fi

if [ -d "$(git rev-parse --git-path rebase-apply)" ] || [ -d "$(git rev-parse --git-path rebase-merge)" ]; then
    echo "[ERROR] Git rebase is in progress"
    echo "[ERROR] Please run git rebase --continue, --abort, or --skip first"
    exit 1
fi

if ! BRANCH=$(git symbolic-ref --quiet --short HEAD 2>/dev/null); then
    echo "[ERROR] HEAD is detached"
    echo "[ERROR] Please switch back to branch master first"
    exit 1
fi

if [ "$BRANCH" != "master" ]; then
    echo "[ERROR] Current branch is $BRANCH"
    echo "[ERROR] Please switch to master first"
    exit 1
fi

if ! git remote get-url origin &>/dev/null; then
    echo "[ERROR] Remote origin is not configured"
    echo "[INFO] Run this once:"
    echo "  git remote add origin https://gitee.com/hechunmu/v851s_-lvgl_-opencv.git"
    exit 1
fi

if [ "$#" -eq 0 ]; then
    MSG="sync $(date '+%Y-%m-%d_%H-%M-%S')"
else
    MSG="$*"
fi

echo "[INFO] Repo: $PWD"
echo "[INFO] Commit: $MSG"
echo "[INFO] Start sync..."

git add -A

if ! git diff --cached --quiet; then
    git commit -m "$MSG"
else
    echo "[INFO] Nothing to commit"
fi

if git ls-remote --exit-code --heads origin master >/dev/null 2>&1; then
    git pull --rebase origin master
else
    echo "[INFO] Remote branch master does not exist yet, skip pull"
fi

git push origin master

echo "[OK] Sync completed"
