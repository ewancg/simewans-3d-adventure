{ name, self, pkgs, ... }:
let
  mkZip = (srcName: pkg:
    pkgs.runCommand "${srcName}-zip"
      {
        nativeBuildInputs = [ pkgs.zip ];
      } ''
      mkdir -p $out
      cp -rL ${pkg}/* staging/
      cd staging
      zip -r $out/${srcName}.zip .
    '');
  mkPackage = { file, pkgs, srcName ? null, platform ? null, debug }: pkgs.callPackage file {
    src = self;
    name =
      if (!isNull srcName) then
        "${name}-${srcName}"
      else
        name;
    arch = srcName;
    inherit platform debug;
  };
  mkFullPackage = (srcName: file: pkgs: platform:
    let
      mkPkg = { debug ? false }: mkPackage {
        inherit file pkgs srcName platform debug;
      };
      releasePkg = mkPkg {};
      debugPkg = mkPkg { debug = true; };
    in
    {
      "${srcName}" = releasePkg;
      "${srcName}-debug" = debugPkg;
      "${srcName}-zip" = mkZip "${srcName}" releasePkg;
      "${srcName}-debug-zip" = mkZip "${srcName}" debugPkg;
    });
in
{ default = pkgs.callPackage ./pkgs/default.nix { inherit name; src = self; }; }
//
(with pkgs; mkFullPackage
  "windows-x86_64"
  ./pkgs/default.nix
  pkgsCross.mingwW64
  lib.platforms.windows
) // (with pkgs; mkFullPackage
  "linux-x86_64"
  ./pkgs/default.nix
  pkgsCross.gnu64
  lib.platforms.linux
) // (with pkgs; mkFullPackage
  "macos-aarch64"
  ./pkgs/default.nix
  pkgsCross.aarch64-darwin
  lib.platforms.darwin
)
