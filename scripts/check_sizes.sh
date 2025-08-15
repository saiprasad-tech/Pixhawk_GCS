#!/bin/bash

# Script to check sizes and validate APK meets requirements

set -e

echo "=== Pixhawk GCS Size Verification ==="
echo

# Check asset sizes
echo "Asset file sizes:"
if [ -d "app/src/main/assets/payload" ]; then
    ls -lh app/src/main/assets/payload/*.bin
    echo
    echo "Total asset size: $(du -sh app/src/main/assets/payload | cut -f1)"
else
    echo "❌ Assets directory not found!"
    exit 1
fi

echo

# Check native library sizes per ABI
echo "Native library sizes per ABI:"
jnilibs_total=0
for abi in armeabi-v7a arm64-v8a x86_64; do
    abi_dir="app/src/main/jniLibs/$abi"
    if [ -d "$abi_dir" ]; then
        echo "$abi:"
        ls -lh "$abi_dir/"*.so 2>/dev/null || echo "  No libraries found"
        abi_size=$(du -s "$abi_dir" 2>/dev/null | cut -f1 || echo "0")
        abi_size_mb=$((abi_size / 1024))
        echo "  Total for $abi: ${abi_size_mb} MB"
        jnilibs_total=$((jnilibs_total + abi_size))
    else
        echo "$abi: Directory not found"
    fi
    echo
done

jnilibs_total_mb=$((jnilibs_total / 1024))
echo "Overall jniLibs size: ${jnilibs_total_mb} MB"
echo

# Check repository size (committed files only)
echo "Repository size analysis:"
if [ -d ".git" ]; then
    # Get size of tracked files only
    repo_size_kb=$(git ls-files -z | xargs -0 du -ck | tail -1 | cut -f1)
    repo_size_mb=$((repo_size_kb / 1024))
    echo "Total committed files: ${repo_size_mb} MB"
    
    if [ $repo_size_mb -gt 20 ]; then
        echo "✅ Repository size > 20 MB requirement: PASSED"
    else
        echo "❌ Repository size < 20 MB requirement: FAILED"
        echo "   Current: ${repo_size_mb} MB, Required: >20 MB"
    fi
else
    echo "❌ Git repository not found"
fi

echo

# Check for built APK
echo "APK size verification:"
apk_found=false
apk_size_mb=0

# Common APK output locations
apk_locations=(
    "app/build/outputs/apk/release/app-release.apk"
    "app/build/outputs/apk/release/app-release-unsigned.apk"
    "build/outputs/apk/release/app-release.apk"
)

for apk_path in "${apk_locations[@]}"; do
    if [ -f "$apk_path" ]; then
        apk_found=true
        apk_size_bytes=$(stat -f%z "$apk_path" 2>/dev/null || stat -c%s "$apk_path" 2>/dev/null)
        apk_size_mb=$((apk_size_bytes / 1024 / 1024))
        echo "Found APK: $apk_path"
        echo "APK size: ${apk_size_mb} MB ($(du -h "$apk_path" | cut -f1))"
        break
    fi
done

if [ "$apk_found" = false ]; then
    echo "⚠️  No release APK found. Build the project first:"
    echo "   ./gradlew :app:assembleRelease"
    echo
    echo "Expected APK location: app/build/outputs/apk/release/app-release.apk"
else
    echo
    if [ $apk_size_mb -gt 50 ]; then
        echo "✅ APK size > 50 MB requirement: PASSED"
        echo "   APK size: ${apk_size_mb} MB"
    else
        echo "❌ APK size < 50 MB requirement: FAILED"
        echo "   Current: ${apk_size_mb} MB, Required: >50 MB"
        echo
        echo "This may be due to:"
        echo "   - APK compression reducing file sizes"
        echo "   - Missing build configuration to retain debug symbols"
        echo "   - Gradle resource shrinking being enabled"
        exit 1
    fi
fi

echo

# Summary
echo "=== SUMMARY ==="
if [ -d "app/src/main/assets/payload" ]; then
    asset_size=$(du -s app/src/main/assets/payload | cut -f1)
    asset_size_mb=$((asset_size / 1024))
    echo "✅ Assets: ${asset_size_mb} MB"
fi

echo "✅ Native libs: ${jnilibs_total_mb} MB across 3 ABIs"

if [ -d ".git" ]; then
    echo "Repository: ${repo_size_mb} MB $([ $repo_size_mb -gt 20 ] && echo "✅" || echo "❌")"
fi

if [ "$apk_found" = true ]; then
    echo "APK: ${apk_size_mb} MB $([ $apk_size_mb -gt 50 ] && echo "✅" || echo "❌")"
fi

echo
if [ "$apk_found" = true ] && [ $apk_size_mb -gt 50 ] && [ $repo_size_mb -gt 20 ]; then
    echo "🎉 All size requirements met!"
    exit 0
else
    echo "❌ Some requirements not met. See details above."
    exit 1
fi