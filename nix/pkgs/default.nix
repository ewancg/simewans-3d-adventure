{ name
, src
, arch ? ""
, platform ? null
, debug ? false
, lib
, stdenv
, cmake
, gcc
, ninja
, sdl3
}:
let buildInputs = [
    sdl3.dev
];
 in stdenv.mkDerivation
{
  inherit name src;

  nativeBuildInputs = [
    gcc
    cmake
    ninja
  ];

  inherit buildInputs;
  passthru.runtimeLibs = buildInputs;

  cmakeFlags = lib.optionals stdenv.hostPlatform.isDarwin [
    "-DSDL_FRAMEWORK=ON"
    "-DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'"
  ];
  env = lib.optionalAttrs stdenv.hostPlatform.isWindows {
    LDFLAGS = "-static";
  };

  meta = {
    description = "${name} ${arch}";
  } // lib.optionalAttrs (!isNull platform) {
    platforms = platform;
  };
} // lib.optionalAttrs debug {
  cmakeBuildType = "Debug";
  dontStrip = true;
}
