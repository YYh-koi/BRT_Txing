# Repository Guidelines

## Project Structure & Module Organization

This repository is a FreeRTOS Bluetooth SDK for the AMT630H V100 target. Root IAR project files (`libbrlink_630h.ewp`, `libbrlink_630hv100.ewp`) describe embedded builds. Public headers live in `include/`, including `include/autoconf_bt.h` and Bluedroid-compatible APIs under `include/bluedroid/`. Bluetooth stack code is organized by profile in `stack/` (`att/`, `hci/`, `hid/`, `sdp/`, `ancs/`, etc.). Platform implementation and generated AT command glue live under `sdk/auto_app/` and `sdk/auto_at/`; `transport/`, `h5/`, `bcsp/`, `rfcomm/`, `gatt/`, `audio_hw/`, and `vendor/` hold feature modules. `linux/` contains host-side sources. Build outputs such as `brlink/Exe/`, `brlink/List/`, `brlink/Obj/`, object files, shared libraries, and `settings/` are ignored.

## Build, Test, and Development Commands

- `make -C sdk/auto_app`: builds `libbt_platform.so` from the auto application layer and selected profiles.
- `make -C sdk/auto_app clean`: removes `libbt_platform.so` and object files.
- `make -C sdk`: invokes the SDK subdirectory Makefile. Verify subdirectory paths before relying on it, because target names must match local directories.
- Open `libbrlink_630hv100.ewp` in IAR Embedded Workbench for the firmware-oriented project build.

## Coding Style & Naming Conventions

Use C with existing local style: tabs or four spaces are both present, so match the surrounding file. Keep functions and variables in lower snake case (`brt_bt_task`, `bt_sw_cfg`), constants and configuration switches in upper snake case (`CONFIG_SPP`, `BT_UART_PORT`), and file names descriptive by feature (`internal_hf.c`, `auto_spp.c`). Prefer existing `CONFIG_*` guards and Makefile source lists when adding optional modules. Do not manually edit generated files marked as generated, such as `sdk/auto_app/Makefile`.

## Testing Guidelines

No dedicated automated test framework is currently present. Before submitting changes, run the narrowest relevant build and a clean rebuild when source lists or headers change. For Bluetooth profile work, validate the affected `CONFIG_*` combination and document any manual smoke test, such as SPP connection, A2DP playback, or GATT discovery.

## Commit & Pull Request Guidelines

The visible history uses concise, descriptive commit subjects, for example `Initial commit: brt_aud3_sdk_freertos with standard SPP server support`. Keep subjects imperative or descriptive and mention the affected subsystem when useful (`auto_app: fix SPP reconnect`). Pull requests should summarize the change, list builds run, call out configuration flags touched, and include manual test notes for hardware or profile behavior. Avoid committing generated build outputs, IDE user files, or local backups.

## Security & Configuration Tips

Treat `include/autoconf_bt.h`, `CONFIG_FILE`, chip-selection flags, license settings, and vendor modules as configuration-sensitive. Do not add secrets, private license keys, or machine-specific paths to tracked files.
