# Device Registration and Driver Binding

This document describes the current path from a device object to class
membership and driver probing. The generic device layer is in
`src/kernel/dev/`; bus and driver registration and matching are in
`src/kernel/drivers/base/`.

## Registration paths

The main entry points are declared in `src/kernel/include/dev/device.h`:

- `device_create()` allocates a zero-initialized `device_t`. The caller must
  fill in fields such as `init_name`, `class`, `bus`, and any class-specific
  or driver-specific data before registration.
- `device_register()` is the ordinary registration entry point. Unless
  `DEV_FLAG_NO_DRIVER` is set or a driver is already assigned, it immediately
  returns the result of bus probing. In that path, this function does not
  continue on to assign a name or add the device to its class. The later
  registration path requires a class and `init_name`, then assigns identity
  fields and adds the device to the class list.
- `device_register_child()` registers a child under an already-added parent.
  It requires both devices and a child class, assigns the child name and
  parent pointer, links it into the parent's `children` list, and adds it to
  its class.
- `device_add()` also assigns a name and adds the device to its class, but
  does not run the ordinary `device_register()` probe gate. Current code uses
  it in some bus and driver paths.

The two root-device entry points, `device_register()` and `device_add()`, do
not have identical ID and probing behavior. The early-probe path intentionally
does not assign a name before probing because bus-enumerated devices may not
have a name yet. The driver or bus code is expected to set `init_name` and
call `device_add()` after it identifies the device. `device_register()` does
not add the device to its class after a successful probe; the probing code
must do so. Follow the specific caller's path when debugging a device rather
than assuming all registrations assign every field in the same way.

## Class publication

`class_register()` places a `dev_class_t` in the class registry. It requires a
`device_added` callback and a `device_removed` callback and then initializes the class's device list.
`class_add_device()` inserts each device into that list and calls the class
hook. The class hook is where a class can react to newly added devices; for
example, the block class starts MBR partition detection for whole-disk
devices, then registers detected partitions as children. On removal,
`class_remove_device()` calls `device_removed` before unlinking the device.

Class registration and device registration are distinct operations. A device
sets its `class` pointer before registration, and the class must already be
usable when the device is added.

## Bus and driver probing

`bus_register()` registers a `bus_type_t`; `drivers_register()` adds a
`driver_t` to the global driver list. A driver points to its bus. The
device/driver attach path then:

1. Iterates the drivers associated with the device's bus.
2. Calls the bus driver's `match(dev, drv)` callback when present.
3. If matched, checks `DEV_FLAG_READY_TO_PROBE`.
4. Assigns `dev->driver`, then calls the bus-level `probe(dev)`
   callback when present; otherwise it calls the driver's `probe(dev)`
   callback. A failed probe clears `dev->driver`.

The bus-level match callback decides whether a driver is applicable; probe
performs the driver's setup and may reject or defer the match. Driver
descriptors also contain `remove`, `suspend`, and `resume` callbacks, but
their presence does not mean those transitions are fully wired into generic
device teardown or power management.

The current implementation propagates the driver-iteration result from
`device_attach()`, mapping a non-success result to `KERRNO_ENTRY_NOT_FOUND`;
`bus_probe_device()` returns that result. Consequently, a registration return
value alone is not a reliable summary of binding: inspect the probe result,
logs, and `dev->driver`, especially when the driver iteration ends after
attempting matches.

## Topology example

The intended representation of a storage stack is:

```text
PCI controller (DEVICE_PCI)
└── SATA disk (block class)
    ├── partition (block class)
    └── partition (block class)
```

The bus relationship, device tree, and class are orthogonal:

- The bus describes where/how a device is enumerated and matched to drivers.
- The parent/child links describe device topology.
- The class describes the interface consumers use.

Partition devices use the block class and are registered as children of the
whole-disk device. MBR detection examines the four primary partition entries
and invokes a kernel panic when it encounters a GPT protective partition type
(`0xEE`), because GPT is not supported yet. Partition reads round the byte
offset and length up to sector boundaries, add the partition's starting
sector, and dispatch to the parent block device's sector operation; the
returned sector count is converted back to bytes.

## Teardown and future work

`device_unregister()` first validates that the device and its class are
non-null, rejects an already-offline device, marks it offline, and confirms it
is present in the class list. It then sets `offline_disabled` and performs
teardown in this order:

1. Pop and recursively unregister each child.
2. If the device has a bus, call `bus_remove_device()`.
3. If present, call `device_ops_t::destroy`.
4. Call `class_remove_device()`, which calls the class's
   `device_removed` hook before unlinking the device from the class list.

After teardown begins, callback results are accumulated with bitwise OR, so
later steps are attempted even if an earlier child or callback reports an
error. The function returns the accumulated status. It does not directly
free the device; any storage cleanup must be supplied by the destroy
operation or another owner. There is no rollback, so errors can leave a
partially torn-down, offline device.

The current bus removal path calls the bound driver's `remove` callback and
clears `dev->driver` after that callback returns. The `bus_type_t::remove`
callback is not used by `bus_remove_device()`. When no driver is bound, the
current path invokes `IMPLEMENTED_QUICK_AND_DIRTY()`: it reports
`KERRNO_DONT_DO_THAT` and then triggers a kernel panic. A bound driver must
provide a non-null `remove` callback; the current code invokes it without a
null check.

The `offline_disabled` flag is set by unregistration but currently
unsupported. Suspend/resume callbacks remain declared but are
not wired into this lifecycle. The generic `poll` operation is present in
`device_ops_t` but has no completed contract, and `mmap` is only a commented
design note. Neither should be treated as a supported device capability
based solely on its appearance in the structure.
