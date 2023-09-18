{
  "targets": [
    {
        "target_name": "spout",
        "sources": [],
        "defines": [],
        "cflags": ["-std=c++11", "-Wall", "-pedantic", "-O3"],
        "include_dirs": [ 
          "<!(node -p \"require('node-addon-api').include_dir\")",
        ],
        "libraries": [],
        "dependencies": [],
        "conditions": [
            ['OS=="win"', {
              "sources": [ "node-spout.cpp" ],
              'include_dirs': [
                './node_modules/native-graphics-deps/include',
              ],
              'library_dirs': [
                './lib/x64',
                './node_modules/native-graphics-deps/lib/windows/glew',
                'lib/<(target_arch)',
              ],
              'libraries': [
                '-lSpoutLibrary.lib',
                'glew32.lib',
                'opengl32.lib'
              ],
              'defines' : [
                'WIN32_LEAN_AND_MEAN',
                'VC_EXTRALEAN'
              ],
              "copies": [{
                'destination': './build/<(CONFIGURATION_NAME)/',
                'files': ['./lib/x64/SpoutLibrary.dll']
              }]
            }],
            ['OS=="mac"', {
              'cflags+': ['-fvisibility=hidden'],
              'xcode_settings': {},
            }],
            ['OS=="linux"', {}],
        ],
    }
  ]
}