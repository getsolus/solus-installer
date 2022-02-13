installer_lib_sources = [
    'disk_manager.c',
    'install_info.c',
    'os.c',
    'permissions.c',
    'utils.c'
]

installer_lib_deps = [
    dependency('glib-2.0', version: '>= 2.66'),
    dependency('gio-2.0', version: '>= 2.66')
]

os_installer_lib = shared_library(
    'solusinstaller',
    installer_lib_sources,
    dependencies: installer_lib_deps,
    install: true
)

link_installer_lib = declare_dependency(
    link_with: os_installer_lib,
    include_directories: [
        include_directories('.')
    ]
)
