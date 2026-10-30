# BareOS Documentation

This folder contains design notes and subsystem documentation. Start with the
topic closest to the area you are working on:

- [ABI contracts](./abi-contracts.md) — interfaces shared across boot,
  kernel, and user-space boundaries.
- [Kernel initialization](./kernel/init.md) — initialization phases and
  initcalls.
- [System calls](./kernel/syscalls.md) — syscall interface and conventions.
- [Device model](./kernel/device/device-model.md) — generic device objects,
  IDs, topology, classes, and generic I/O.
- [Device lifecycle](./kernel/device/device-lifecycle.md) — device
  registration, class insertion, and bus/driver probing.
- [Device classes](./kernel/driver/device-classes.md) — class-specific
  operations and the typed-interface design.

The device-model and device-lifecycle documents describe the implementation
currently in `src/kernel/dev/` and `src/kernel/drivers/base/`; they also call
out unfinished behavior where the code does not yet provide a complete
lifecycle contract.