# JanusLoader

RISC-V VM shellcode 加载器：payload 以 RV64 指令流形式由自建 RV64IM 解释器执行，
内存中自始至终不存在任何 x64 shellcode，也没有任何内存区域需要可执行权限。

针对内存扫描 / 静态分析的对抗点：

- **指令集隔离**：payload 交叉编译为 RV64 机器码，x64 字节签名体系对其不适用
- **opcode 置换**：编译期 Fisher-Yates 置换（`JANUS_OPCODE_SEED` 驱动），嵌入脚本同种子重写 payload 的 opcode 字段，静态分发表分析失效
- **纯 RW 内存**：解释执行不需要 X 权限，全程无 RWX / 无私有可执行页
- **无 exec 通道**：ECALL 仅 GET_PEB / HOST_CALL / RESOLVE / EXIT 四个，逻辑完全以 guest 指令流存在，不落地原生 shellcode
- **哈希化 API 解析**：DLL/API 名 constexpr 编译期折叠为常量，二进制零明文字符串
- **LCG 密钥流加密**：payload 密文存储（`JANUS_XOR_KEY` 驱动），加载时解密

## 构建

```bash
# payload（Linux/WSL，需 riscv64 工具链 + lld + pyelftools）
cd payload && cmake -B build -DCMAKE_TOOLCHAIN_FILE=../cmake/riscv64.cmake
cmake --build build    # 自动执行 tools/embed_payload.py 生成 payload_blob.cpp

# loader（Windows + MSVC）
cmake -B build && cmake --build build --config Release
.\build\Release\JanusLoader.exe    # 演示：弹出 calc.exe
```

## 目录

```
include/janus/   VM 状态、译码宏、ECALL 定义、哈希、战役配置
src/vm/          RV64IM 解释器（取指-译码-分发）
src/syscall/     ECALL 的 Windows 实现（PEB walk、host_call 转发）
src/loader/      解密、重定位、内存分配
src/inject/      inline / fiber / remote_thread 三种执行方式
payload/         RV64 payload 裸机工程（crt0、链接脚本、demo）
tools/           embed_payload.py（拍平 ELF、置换 opcode、加密、生成 blob）
```

## 配置

`include/janus/config.hpp`：`JANUS_OPCODE_SEED` / `JANUS_XOR_KEY` 为每场战役更换项，
修改后需重编 loader 并重跑嵌入脚本，两侧不一致则 payload 无法运行。

## License

BSD-2-Clause，见 LICENSE。
