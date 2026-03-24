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
, glaze # de/serialization of toml for local configs, messagepack for network traffic
, openssl # for glaze
, frozen-containers # contexpr STL-mimicking types
, sdl3 # graphics, input
, shader-slang # cross-platform shader compiler
, miniaudio # audio playback, 3d spatialization
, pkg-config # this and all of the following are for miniaudio
, alsa-lib
, libpulseaudio
, libjack2
}:
let buildInputs = [
  glaze
  frozen-containers
  sdl3.dev
  miniaudio.dev
  alsa-lib.dev
  libpulseaudio.dev
  libjack2.dev
];
 in stdenv.mkDerivation
{
  inherit name src;

  nativeBuildInputs = [
    pkg-config
    gcc
    cmake
    ninja
    shader-slang
    openssl
  ];

  inherit buildInputs;
  passthru.runtimeLibs = buildInputs;

  cmakeFlags = lib.optionals stdenv.hostPlatform.isDarwin [
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
