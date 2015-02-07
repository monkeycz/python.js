{
  "targets": [
    {
      "target_name": "binding",
      "sources": [
        "src/binding.cc",
        "src/utils.cc",
        "src/py_object_wrapper.cc"
      ],
      "conditions": [
        ['OS=="mac"', {
            "xcode_settings": {
              "OTHER_CFLAGS": [
                "-Wno-error=unused-command-line-argument",
                "<!(/usr/bin/python-config --cflags)"
              ],
              "OTHER_LDFLAGS": [
                "<!(/usr/bin/python-config --ldflags)"
              ]
            }
        }, { # not OSX
          "cflags": [
            "<!(python-config --cflags)"
          ],
          "libraries": [
            "<!(python-config --libs)"
          ]
        }]
      ]
    }
  ]
}
