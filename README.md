# solus-installer
New installer for Solus

[![license](https://img.shields.io/github/license/getsolus/solus-installer.svg)]()

## Motivation

The current `os-installer` project has lived to the natural conclusion of its usefulness. It was written in Python 2 which is now EOL and a port to Python 3 is possible, but goes against our desire to keep our development languages isolated to C and Go at the current time. There are a number of issues with that version of the installer which we would like to also rectify:

1. More reliable partitioning, especially with Windows dual-boot
2. Support for additional filesystems like `bcachefs` and `XFS`
3. Support for software RAID installations
4. Reuse of existing LVM setups
5. Better internationalization

## Goals

* New disk management built around `libblockdev`
  - Support `EXT4` and `XFS` root partitions
  - Support software RAID
    * Support LVM
    * Reuse
    * LUKS Encryption
* Support BIOS and UEFI boot configurations
* Improved internationalization support
  - Input Devices
  - Translations
* GUI + library apparoach
  - Allow for other frontends to the installer library (e.g. CLI, OEM scripting)
* Eventual support for a two-stage OOBE process for OEM installs

## License

Copyright 2021 Solus Project <copyright@getsol.us>
 
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 
http://www.apache.org/licenses/LICENSE-2.0
 
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
