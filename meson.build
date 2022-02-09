project('installer', 'c',
        version: '17',
        meson_version: '>= 0.57.0',
        default_options: [
            'warning_level=2',
            'werror=true',
            'c_std=gnu11',
        ],
)

gnome = import('gnome')

#
# Global stuff
#

prefix = get_option('prefix')
datadir = join_paths(prefix, get_option('datadir'))

subdir('data')
subdir('styles')
subdir('src')
