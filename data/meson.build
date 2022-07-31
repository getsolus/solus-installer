desktop_file = 'us.getsol.Installer.desktop'

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils,
    args: [desktop_file]
  )
endif

install_data(
    desktop_file,
    install_dir: join_paths(get_option('datadir'), 'applications')
)

install_data(
    'org.freedesktop.policykit.pkexec.policy',
    install_dir: join_paths(get_option('datadir'), 'polkit-1', 'actions')
)
