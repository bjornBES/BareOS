# Kernel Device Model

The device subsystem provides the common kernel object, `device_t`, used to
identify devices, place them in a parent/child tree, and connect them to a bus,
a driver, and a device class. It deliberately does not define the operations
for every kind of hardware. Those belong to the relevant device class.

The implementation is primarily in `src/kernel/dev/`:

- `device.c` allocates and zero-initializes a generic device.
- `device_registry.c` assigns names and IDs, registers devices and children,
  and maintains class membership.
- `device_id.c` supplies per-`init_name` name counters and class-local IDs.

See [device-lifecycle.md](./device-lifecycle.md) for registration and driver
probing, and [../driver/device-classes.md](../driver/device-classes.md) for
typed class interfaces.

## `device_t`

`device_t` is defined in `src/kernel/include/dev/devicet.h`.

- `init_name` is the base name supplied by the device creator.
- `name` is filled in by registration.
- `id` is the global device-registration sequence number.
- `devt` is an opaque `device_id_t`. Its fields encode the class type in
  `major`, a device instance in `device`, and a sub-resource in `minor`.
  Consumers should treat the packed value as an identifier, not as a pointer
  or a way to infer a driver's private state.
- `parent`, `children`, and `siblings` represent topology. Child devices also
  carry their parent pointer.
- `bus` identifies the bus implementation responsible for matching drivers;
  `bus_id` is an opaque bus-specific identifier.
- `driver` points to the selected driver, while `drv_data` is reserved for
  driver-private state.
- `class` identifies the device's functional category. `class_data` can point
  to class-specific state; generic device code does not interpret it.
- `ops` contains generic byte-oriented `read` and `write` operations, an
  unfinished `poll` slot, and a `destroy` callback. Optional operations are
  represented by null function pointers.
- `flags` includes probe readiness, offline, offline-disabled, no-driver, and
  same-name state. `device_unregister()` marks a device offline and sets
  offline-disabled before tearing it down.

Class-specific types commonly embed `device_t` as their first member and
recover the containing object with a typed `container_of` helper, such as
`to_blkdev()` and `to_input_dev()`. This is separate from `class_data`: the
block and input classes use embedding, while timer devices store their
class-specific timer source in `class_data`.

## Names and identifiers

The registry obtains names from `init_name`; for example, independent devices
with the base name `sata` are ordinarily named `sata0`, `sata1`, and so on.
The name counter is keyed by the base name, not by the device class. Root and
child registration share the name-assignment helper. A device marked
`DEV_FLAG_SAME_NAME` is named exactly `init_name`.

The numeric `id` assigned during class insertion is global across registered
devices. The fields in `devt` have separate roles and are assigned by the
registration path; do not assume that `id`, `devt.device`, and a suffix in
`name` are interchangeable.

## Device tree and classes

The parent/child tree describes physical or logical containment, not device
type. For example, a PCI controller can be a parent of a block-class SATA
device, which can in turn be a parent of block-class partition devices.
`DEVICE_PCI` denotes the PCI host device class; it does not mean that every
device attached to PCI has that class.

`dev_class_t` owns a list of devices and `device_added` and `device_removed`
hooks. During generic registration, `class_add_device()` appends the device
to that class list and calls the add hook. During teardown,
`class_remove_device()` calls the remove hook and then removes the device from
the list. The block class uses its add hook to detect partitions for
non-partition block devices.

## Generic I/O and typed operations

The generic `read` and `write` slots represent data transfer through
`device_t`. Class-specific operations live on the corresponding embedded
class object instead. For example, block devices expose sector operations
through `blkdev_t::ops`, and input devices expose event operations through
`input_dev_t::ops`.

There is no per-device `ioctl` hook. Callers should use the typed operations
for the device class rather than add untyped requests to the generic device
object. See [device-classes.md](../driver/device-classes.md) for the interface
rules.

## Current lifecycle boundaries

`device_unregister()` marks a device offline, recursively unregisters its
children, then invokes bus removal, the optional `destroy` operation, and
class removal in that order. The function itself does not directly free the
device. It is not yet a complete or transactional hot-plug framework: see
[device-lifecycle.md](./device-lifecycle.md) for ordering, callback
requirements, and failure cases.
