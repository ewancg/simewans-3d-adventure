{ name
, src
, arch ? ""
, platform ? null
, lib
, stdenv
, cmake
, gcc
, ninja
}:

let buildInputs = [
];
 in stdenv.mkDerivation {
  inherit name src;

  nativeBuildInputs = [
    gcc
    cmake
    ninja
  ];

  inherit buildInputs;
  passthru.runtimeLibs = buildInputs;

  env = lib.optionalAttrs stdenv.hostPlatform.isWindows {
    LDFLAGS = "-static";
  };

  meta = {
    description = "${name} ${arch}";
  } // lib.optionalAttrs (!isNull platform) {
    platforms = platform;
  };
}
