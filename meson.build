project('epic-c', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

cmake = import('cmake')

libcurl = dependency('libcurl')
jsonc = dependency('json-c')
sdl2_dep = dependency('sdl2')
sdl2_image_dep = dependency('sdl2_image')

# Configure the CMake project
# sdl_sp = cmake.subproject('SDL-release-2.28.0')
# message(sdl_sp.target('SDL2'))
# sdl_image_sp = cmake.subproject('SDL_image-release-2.6.3')

# # Fetch the dependency object
# sdl2_dep = sdl_sp.dependency('SDL2')
# sdl2_image_dep = sdl_image_sp.dependency('SDL2_image')

executable('epic-c',
           'epic.c',
           install : true,
           dependencies: [libcurl, jsonc, sdl2_dep, sdl2_image_dep])

