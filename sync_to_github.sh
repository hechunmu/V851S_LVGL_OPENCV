#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

REMOTE_NAME="${GITHUB_REMOTE_NAME:-github}"
REMOTE_URL="${GITHUB_REMOTE_URL:-git@github.com:hechunmu/V851S_LVGL_OPENCV.git}"
TARGET_BRANCH="${GITHUB_TARGET_BRANCH:-main}"

echo "[INFO] Run pre-sync cleanup..."
echo "[INFO] Cleanup done"

if ! command -v git >/dev/null 2>&1; then
    echo "[ERROR] Git not found in PATH"
    exit 1
fi

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "[ERROR] Current folder is not a Git repository"
    exit 1
fi

BRANCH=$(git branch --show-current)
[ -z "$BRANCH" ] && BRANCH=master

case "$BRANCH" in
    master|main)
        ;;
    *)
        echo "[ERROR] Current branch is $BRANCH"
        echo "[ERROR] Please switch to master or main first"
        exit 1
        ;;
esac

if git remote get-url "$REMOTE_NAME" >/dev/null 2>&1; then
    CURRENT_URL=$(git remote get-url "$REMOTE_NAME")
    if [ "$CURRENT_URL" != "$REMOTE_URL" ]; then
        echo "[ERROR] Remote $REMOTE_NAME points to: $CURRENT_URL"
        echo "[ERROR] Expected: $REMOTE_URL"
        echo "[INFO] Update it with:"
        echo "  git remote set-url $REMOTE_NAME $REMOTE_URL"
        exit 1
    fi
else
    echo "[INFO] Add remote $REMOTE_NAME -> $REMOTE_URL"
    git remote add "$REMOTE_NAME" "$REMOTE_URL"
fi

if [ "$#" -eq 0 ]; then
    MSG="sync $(date '+%Y-%m-%d_%H-%M-%S')"
else
    MSG="$*"
fi

echo "[INFO] Repo: $PWD"
echo "[INFO] Local branch: $BRANCH"
echo "[INFO] Remote: $REMOTE_NAME"
echo "[INFO] Remote URL: $REMOTE_URL"
echo "[INFO] Target branch: $TARGET_BRANCH"
echo "[INFO] Commit: $MSG"
echo "[INFO] Start sync..."

git add -A

if ! git diff --cached --quiet; then
    git commit -m "$MSG"
else
    echo "[INFO] Nothing to commit"
fi

if git ls-remote --exit-code --heads "$REMOTE_NAME" "$TARGET_BRANCH" >/dev/null 2>&1; then
    git pull --rebase "$REMOTE_NAME" "$TARGET_BRANCH"
else
    echo "[INFO] Remote branch $TARGET_BRANCH does not exist yet, skip pull"
fi

git push -u "$REMOTE_NAME" "$BRANCH:$TARGET_BRANCH"

echo "[OK] Sync completed"