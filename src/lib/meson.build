installer_lib_headers = [
    'disk_manager.h',
    'drive.h',
    'installer.h',
    'install_info.h',
    'os.h',
    'partition.h',
    'permissions.h'
]

installer_lib_sources = [
    'disk_manager.c',
    'drive.c',
    'installer.c',
    'install_info.c',
    'os.c',
    'partition.c',
    'permissions.c'
]

installer_lib_deps = [
    dependency('glib-2.0', version: '>= 2.66'),
    dependency('gio-2.0', version: '>= 2.66'),
    dependency('blockdev', version: '>= 2.23')
]

os_installer_lib = shared_library(
    'solusinstaller',
    installer_lib_sources,
    dependencies: installer_lib_deps,
    install: true
)

install_headers(installer_lib_headers, subdir: 'solusinstaller')

link_installer_lib = declare_dependency(
    link_with: os_installer_lib,
    include_directories: [
        include_directories('.')
    ]
)
