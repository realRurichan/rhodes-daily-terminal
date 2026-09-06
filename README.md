# Rhodes Daily Terminal

一个为明日方舟电子通行证设计的轻量每日终端：番茄钟、每日任务、随机事件、等级/LMD 和电子宠物心情。

Windows 用户可以直接双击 `demo/windows-demo.html` 试玩，无需安装任何运行库。

## 桌面试玩

```powershell
cmake -S . -B build
cmake --build build --config Release
./build/Release/daily_terminal.exe
```

按键：`KEY1` 上/上一项、`KEY2` 下/下一项、`KEY3` 确认、`KEY4` 返回。桌面终端演示可直接按数字键 `1`、`2`、`3`、`4`。存档默认写入当前目录的 `daily_terminal.sav`。

## 通行证原生 DRM 版

原生版复用官方 `epass-applications` 的 HAL 和 `epass_game`，获得 DRM RGB565 双缓冲显示、物理尺寸适配和自动扫描的四键输入。先取得官方应用源码，然后在已经构建过的通行证 Buildroot 目录中：

```sh
git clone https://github.com/rhodesepass/epass-applications.git
source output/host/environment-setup
cmake -S /path/to/rhodes-daily-terminal -B /path/to/rhodes-daily-terminal/build-device \
  -DCMAKE_BUILD_TYPE=Release \
  -DEPASS_NATIVE=ON \
  -DEPASS_APPLICATIONS_ROOT=/path/to/epass-applications
cmake --build /path/to/rhodes-daily-terminal/build-device -j$(nproc)
```

将 `daily_terminal` 和 `appconfig.json` 放在同一目录，打成 `.tar.gz` 后放入通行证共享分区的 `apps-inbox`。原生版存档默认写入 `/root/.daily_terminal.sav`，可用 `EPASS_SAVE_PATH` 覆盖。

构建完成后可一键生成并校验安装包：

```sh
sh tools/package_app.sh build-device/daily_terminal
```

产物位于 `packages/rhodes-daily-terminal-v2.tar.gz`，整个压缩包可以直接复制进 `apps-inbox`。

## 当前显示后端

普通桌面构建采用 ANSI 终端显示；带 `EPASS_NATIVE=ON` 的构建使用官方 Sunxi DRM/evdev 后端。两种显示方式共用同一套任务、计时与存档逻辑。
