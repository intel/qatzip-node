{
  "targets": [
    {
      "target_name": "qzip",
      "sources": [ "src/qzip.cc" ],
      "include_dirs": [ "<!@(node -p \"require('node-addon-api').include\")", "/root/QATzip/QATzip/include"],
      "cflags_cc!": [ "-fno-exceptions", "-fno-rtti" ,"-fstack-protector" ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ],
      "libraries": [ 
        "-lqatzip" ], 
    }
  ]
}
