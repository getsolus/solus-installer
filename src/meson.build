add_global_arguments([
	'-I' + meson.current_build_dir(),
	'-Db_sanitize=address',
	'-Dwerror=true',
], language: 'c')

os_installer_sources = [
    'disk_manager.c',
    'install_info.c',
    'main.c',
    'permissions.c',
    'utils.c',
    'window.c'
]

os_installer_deps = [
    dependency('glib-2.0', version: '>= 2.66'),
	dependency('gio-2.0', version: '>= 2.66'),
    dependency('gtk+-3.0', version: '>= 3.22.0')
]

# Compile our gresource file
os_installer_sources += gnome.compile_resources('os-installer-resources',
    'Installer.gresource.xml',
    dependencies: [ style ],
    c_name: 'installer',
)

# Build the executable
executable(
    'us.getsol.Installer',
    os_installer_sources,
    dependencies: os_installer_deps,
    install: true,
)
