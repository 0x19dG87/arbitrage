{
  "targets": [
      {
          "target_name": "sssp",
          "sources": [ "lib/sssp.cpp" ],
          "include_dirs": ["<!(node -p \"require('node-addon-api').include_dir\")"],
          "cflags!": [ "-fno-exceptions", ],
          "cflags": ["-std=c++20","-O3","-ltbb"],
          "cflags_cc!": [ "-fno-exceptions", ],
          "cflags_cc": ["-std=c++20","-O3","-ltbb"],
          "defines": [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
      }
  ]
}