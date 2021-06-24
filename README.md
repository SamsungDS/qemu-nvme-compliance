# QEMU NVMe Compliance Patches

This repository contains a number of patches that, when applied to a specific
version of QEMU, add additional functionality to the emulated NVMe device.

These patches may be useful for compliance testing.

## Quick Start

Currently supported version of QEMU is `v6.0.0`. You may be able to apply the
patches to later versions or master.

Download the supported version to the local directory and extract the sources.

    $ wget https://download.qemu.org/qemu-6.0.0.tar.xz
    $ mkdir src
    $ tar -Jxf qemu-6.0.0.tar.xz -C src --strip-components=1

The patches are managed by quilt, so you need to install that. Use the `quilt`
wrapper script to apply the patches. The `quilt` wrapper script only sets up a
couple of required environment variables to make it easy to apply the patches
in the subdirectory, so you can use any quilt subcommand to manually apply
individual patches or fix hunks that fail. If you did not extract to `./src`,
set the `SRCDIR` environment variable to point to the extracted sources.

To apply all patches, do

    $ ./quilt push -a

Now, proceed to configure and compile QEMU as usual.

To revert all changes, do

    $ ./quilt pop -a


## License

Like QEMU, all patches in this repository are licensed under the GNU General
Public License version 2.
