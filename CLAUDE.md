# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

QEMU is a generic and open source machine & userspace emulator and virtualizer. It can emulate complete machines in software without hardware virtualization support, integrate with hypervisors like KVM and Xen for near-native performance, and provide userspace API virtualization for running binaries across different architectures.

## Common Development Commands

### Build System

QEMU uses Meson as its primary build system with configure as a frontend:

```bash
# Initial configuration (from repository root)
mkdir build
cd build
../configure

# Common configure options:
../configure --target-list=x86_64-softmmu  # Build only specific targets
../configure --enable-debug                # Enable debug symbols
../configure --enable-sanitizers           # Enable ASan/UBSan

# Build
make -j$(nproc)

# Clean build
make clean

# Full clean (removes configuration)
make distclean
```

### Testing

```bash
# Run all tests
make check

# Run specific test suites
make check-unit              # Unit tests
make check-qtest            # QTest device tests  
make check-functional       # Python functional tests
make check-block            # Block layer tests
make check-qapi-schema      # QAPI schema tests

# Run tests for specific target
make check-qtest-x86_64

# Run with verbose output
make V=1 check

# Generate test report
make check-report.junit.xml
```

### Development Tasks

```bash
# Generate tags for code navigation
make ctags      # Generate ctags
make TAGS       # Generate etags
make cscope     # Generate cscope index

# Update Linux VDSO images
make update-linux-vdso

# Run specific QEMU binary (after build)
./build/qemu-system-x86_64 [options]
./build/qemu-img [options]
```

## High-Level Architecture

QEMU is organized into several major subsystems that interact to provide emulation capabilities:

### Core Subsystems

1. **TCG (Tiny Code Generator)** - `/tcg/`, `/accel/tcg/`
   - Dynamic binary translation engine that converts guest code to host code
   - Implements the core emulation loop for software-based virtualization
   - Architecture-specific backends in `/tcg/<arch>/`

2. **Accelerators** - `/accel/`
   - Abstraction layer for different acceleration technologies
   - Implementations: TCG (software), KVM (Linux kernel virtualization), HVF (macOS), WHPX (Windows), Xen
   - Each accelerator provides CPU execution and memory management interfaces

3. **QOM (QEMU Object Model)** - `/qom/`, `/include/qom/`
   - Object-oriented framework for devices and other QEMU components
   - Provides inheritance, properties, and introspection capabilities
   - All devices and many internal structures are QOM objects

4. **Device Emulation** - `/hw/`
   - Organized by device type: `/hw/net/` (network), `/hw/block/` (storage), `/hw/display/` (graphics), etc.
   - Board definitions in `/hw/<arch>/` directories
   - Uses QOM for device modeling and QDev for device tree management

5. **Block Layer** - `/block/`, `/block/`
   - Handles disk images and block device emulation
   - Supports numerous formats: qcow2, raw, vmdk, vdi, etc.
   - Provides features like snapshots, backing files, and live migration

6. **Memory Management** - `/system/memory.c`, `/system/physmem.c`
   - Implements guest physical memory mapping
   - Memory regions and address spaces abstraction
   - DMA and IOMMU support

7. **Migration** - `/migration/`
   - Live migration of running VMs between hosts
   - Savevm/loadvm for snapshots
   - COLO (COarse-grained LOck-stepping) for fault tolerance

8. **Monitor** - `/monitor/`
   - HMP (Human Monitor Protocol) for interactive use
   - QMP (QEMU Machine Protocol) for programmatic control
   - Commands defined throughout codebase with QMP/HMP infrastructure

### Target Architectures

Each supported architecture has its implementation in `/target/<arch>/`:
- CPU definitions and instruction decoding
- Architecture-specific registers and features
- Integration with accelerators (TCG ops generation, KVM/HVF support)

### User Mode Emulation

- **Linux User Mode** - `/linux-user/`
  - Syscall translation and signal handling
  - Per-architecture support in subdirectories

- **BSD User Mode** - `/bsd-user/`
  - FreeBSD, NetBSD, OpenBSD syscall emulation

### Key APIs and Frameworks

1. **QAPI** - `/qapi/`
   - Schema-based API definition language
   - Generates C code for QMP commands and types
   - JSON schemas define interfaces

2. **Coroutines** - `/util/coroutine-*.c`
   - Cooperative threading for block I/O
   - Enables synchronous-style code for async operations

3. **Main Loop** - `/util/main-loop.c`
   - Event-driven architecture using poll/epoll
   - Handles timers, file descriptors, and bottom halves

4. **Error Handling** - `/util/error.c`, `/include/qapi/error.h`
   - Consistent error reporting through Error objects
   - Propagation through call chains

### Build System Architecture

The build uses a two-stage process:
1. `configure` script (shell) performs host detection and feature tests
2. Meson (Python) handles the actual build configuration and ninja file generation

Key build files:
- `meson.build` - Main build definition
- `meson_options.txt` - Build options
- `configs/targets/*.mak` - Per-target configurations
- `default-configs/devices/*.mak` - Device selections

### Code Organization Patterns

- Headers in `/include/` mirror source organization
- Stubs in `/stubs/` provide empty implementations for optional features
- Tests mirror source structure under `/tests/`
- Architecture-specific code uses #ifdef or separate files
- QOM types follow TypeInfo registration pattern
- Extensive use of callbacks and function pointers for polymorphism

## Important Notes

- Always run tests before submitting changes
- Follow QEMU coding style (can be checked with `scripts/checkpatch.pl`)
- Many operations are asynchronous due to coroutine usage
- Device models must handle migration (save/load state)
- Guest-visible behavior changes require compatibility handling
- Performance-critical paths often have architecture-specific optimizations