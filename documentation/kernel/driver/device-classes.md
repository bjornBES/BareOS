# Device Classes

A device class groups devices that share an interface (block, input, video, and so on). Classes live in the driver subsystem. The generic device object (`device_t`) is described in [device-model.md](../device/device-model.md).

## dev_class_t

`dev_class_t` is a registry entry, not an interface.

- `this_node` (via `LIST_ENTRY_THIS()`): link in the class registry.
- `devices`: list of every device in this class.
- `name`: human-readable name such as `"block"` or `"timer"`.
- `type`: the `dev_type_t` value for this class.
- `device_added(dev)` is called when a device is added to the class list.
- `device_removed(dev)` is called during removal, before the device is
  unlinked from the class list. `class_register()` currently validates
  `device_added` but does not validate `device_removed`, even though the
  removal path calls it unconditionally.

`dev_class_t` holds no function pointers for operating on devices. There is no per-class "class type" for ops to hang off, and if there were, it would have to be private to one `.c` file.

## dev_type_t

The enum that tags a class. It is defined with the class files, and `device_t` never uses it directly (it reaches it through `class->type`).

- `DEVICE_UNKNOWN`
- `DEVICE_CHAR`: byte-stream devices (keyboards, mice)
- `DEVICE_SERIAL`
- `DEVICE_BLOCK`: disks and partitions, random-access blocks
- `DEVICE_TTY`
- `DEVICE_NET`
- `DEVICE_VIDEO`: framebuffers
- `DEVICE_TIMER`
- `DEVICE_PSEUDO`: `/dev/null`, `/dev/zero`, `/dev/random`
- `DEVICE_VIRTUAL`
- `DEVICE_PCI`: the PCI host device itself (for example the AHCI controller), not any device that merely sits on PCI
- `DEVICE_TYPE_MAX`: count, not a real type

## Per-class device structs

Typed operations live in the per-class device structs, in an `ops` field of each one. This is what replaces ioctl.

- `input_dev_t` ops: `flush`, `event` (type, code, value) and `irq_handler`.
- `blkdev_t` ops: `read_sectors` and `write_sectors`, taking a sector count and sector offset and returning the sector count through an out-parameter.

Other classes (`pci_dev`, `display_dev` and so on) can follow the same
pattern, with their own struct and typed operations. Class-specific structs
commonly embed `device_t` and use `container_of` helpers such as
`to_blkdev()` and `to_input_dev()`. Other users, including timer devices,
store their class-specific object in `device_t::class_data`. The conversion
style is part of each class's implementation; `device_t` does not provide a
universal checked accessor.

## Rules

- Check the class before using a per-class struct. The current
  `container_of` helpers are casts, not runtime class checks.
- Calls go downward through typed interfaces only. For example, partition code calls the block ops of its disk directly. Kernel code never goes through the syscall layer to reach another device.
- Generic read/write on `device_t` is for plain data transfer. Everything else gets a typed op in the class struct.

## Why there is no ioctl

ioctl is one entry point that takes a request number and an untyped pointer. Its problems:

- The call graph is hidden: a syscall calls a driver's ioctl, which can call another driver's ioctl, and nothing in the code shows the chain.
- Argument contracts exist only in documentation and in the driver's own switch statement.

What BareOS does instead:

- Each class gets typed ops in its own device struct, so argument types are checked by the compiler and the set of operations is visible in one place.
- Layers call strictly downward, so call chains can be read from the code.

Costs accepted:

- Every new capability is a new op in a class struct (or a new syscall), so the ABI grows explicitly instead of hiding behind a request number.
- Ported software that calls `ioctl()` directly will not work until a shim exists. If one is added, it should be a thin syscall that decodes the request and dispatches to the typed op, never a per-driver ioctl hook.

POSIX only defines `ioctl` for the STREAMS option. Terminal control is specified through dedicated functions like `tcgetattr` and `tcsetattr`, which BareOS can implement on top of the tty class ops.
