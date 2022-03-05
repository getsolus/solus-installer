installer_lib_headers = [
    'disk_manager.h',
    'installer.h',
    'install_info.h',
    'os.h',
    'permissions.h',
    'strings.h',
    'system_partition.h'
]

installer_lib_sources = [
    'disk_manager.c',
    'installer.c',
    'install_info.c',
    'os.c',
    'permissions.c',
    'strings.c',
    'system_partition.c'
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
