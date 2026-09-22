# VMLoader

自建虚拟机加载器：payload 被编译为 RISC-V 64 位机器码，由内置的 RV64IM 解释器
逐条解释执行。内存中自始至终不存在任何 x64 shellcode，也没有任何内存区域需要
可执行权限——针对 x64 字节签名的内存扫描面对的是一个空集。

设计要点：

- **指令集隔离**：payload 是 RV64 指令流，x64 shellcode 签名体系对其不适用
- **opcode 置换**：编译期 Fisher-Yates 置换（`JANUS_OPCODE_SEED` 驱动），嵌入脚本
  以同种子重写 payload 的 opcode 字段，静态分发表分析失效
- **纯 RW 内存**：解释执行不需要 X 权限，全程无 RWX、无私有可执行页
- **无 exec 通道**：ECALL 仅 GET_PEB / HOST_CALL / RESOLVE / EXIT 四个，逻辑完全
  以 guest 指令流存在，设计上不存在落地原生 shellcode 的路径
- **哈希化 API 解析**：DLL/API 名经 constexpr sdbm 在编译期折叠为常量，二进制
  零明文字符串
- **密文存储**：payload 以 LCG 密钥流加密（`JANUS_XOR_KEY` 驱动），加载时解密

## 构建

```bash
# payload（Linux/WSL，需 riscv64 工具链 + lld + pyelftools）
cd payload && cmake -B build -DCMAKE_TOOLCHAIN_FILE=../cmake/riscv64.cmake
cmake --build build    # 自动执行 tools/embed_payload.py 生成 payload_blob.cpp

# loader（Windows + MSVC）
cmake -B build && cmake --build build --config Release
.\build\Release\VMLoader.exe    # 演示：弹出 calc.exe
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

见 LICENSE。
