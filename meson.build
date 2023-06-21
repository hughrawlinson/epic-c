project('epic-c', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

libcurl = dependency('libcurl')
jsonc = dependency('json-c')
sdl2_dep = dependency('sdl2')
sdl2_image_dep = dependency('sdl2_image')

executable('epic-c',
           'epic.c',
           install : true,
           dependencies: [libcurl, jsonc, sdl2_dep, sdl2_image_dep])

