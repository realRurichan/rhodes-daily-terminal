# Rhodes Daily Terminal

一个为明日方舟电子通行证设计的轻量每日终端：番茄钟、每日任务、随机事件、等级/LMD 和电子宠物心情。

## 桌面试玩

```powershell
cmake -S . -B build
cmake --build build --config Release
./build/Release/daily_terminal.exe
```

按键：`W` 上、`S` 下、`Enter` 确认、`Q` 返回/退出。存档默认写入当前目录的 `daily_terminal.sav`。

## 交叉编译

在已经构建过的通行证 Buildroot 目录中：

```sh
source output/host/environment-setup
cmake -S /path/to/rhodes-daily-terminal -B /path/to/rhodes-daily-terminal/build-device \
  -DCMAKE_BUILD_TYPE=Release
cmake --build /path/to/rhodes-daily-terminal/build-device -j$(nproc)
```

将 `daily_terminal` 和 `appconfig.json` 放在同一目录，打成 `.tar.gz` 后放入通行证共享分区的 `apps-inbox`。程序默认读取 `/dev/input/event0`；可用 `EPASS_INPUT_DEVICE` 覆盖。可用 `EPASS_SAVE_PATH` 把存档指定到持久化目录。

## 当前显示后端

当前版本采用 ANSI 终端显示，以便先验证完整玩法和存档。业务逻辑与 `platform.c` 分离；接入设备 DRM/字体渲染时，只需替换 `dt_platform_draw()`，无需改动任务、计时和存档代码。

