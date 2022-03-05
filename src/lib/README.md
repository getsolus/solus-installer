# solusinstaller

Library for the Solus installer.

[![license](https://img.shields.io/github/license/getsolus/solus-installer.svg)]()

## Description

This library powers the backend for the Solus installer. It is installed as a separate library in order to fascilitate installing from multiple different frontends (GUI, CLI, OEM scripting).

It makes use of GLib and GIO for types and utility functions, and [`libblockdev`](https://github.com/storaged-project/libblockdev) for disk and partition management.

See [here](https://github.com/getsolus/solus-installer#goals) for more information about what this library is meant to accomplish.

## Usage

Due to `libblockdev` needing to be initialized before use, developers using this library are recommended to use our built-in function to do just that:

```c
#include <solusinstaller/installer.h>
/* other includes */

int main(void) {
    g_autoptr(GError) err = NULL;
    gboolean success = installer_blockdev_init(&err);

    if (!success) {
        // Handle the error
        return 1;
    }

    return 0;
}
```

This ensures that `libblockdev` is initialized with the plugins required for the installer.

## License

Copyright 2022 Solus Project <copyright@getsol.us>
 
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 
http://www.apache.org/licenses/LICENSE-2.0
 
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
