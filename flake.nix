{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay.url = "github:oxalica/rust-overlay";

    # Contains runtime configuration for nix shell
    configNix = {
      url = "path:./nix/config.nix";
      flake = false;
    };
    # Contains convenience scripts for nix shell
    scriptsNix = {
      url = "path:./nix/scripts.nix";
      flake = false;
    };
    # Contains reproducible package targets for nix build
    packagesNix = {
      url = "path:./nix/pkgs.nix";
      flake = false;
    };

    self = {
      submodules = true;
    };
  };
  outputs =
    { self, ... }@inputs:
      with inputs;
      let
        name = "simewan-3d-adventure";
      in
      flake-utils.lib.eachDefaultSystem (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [ (import inputs.rust-overlay) ];
          };
          lib = pkgs.lib;
          packages = import packagesNix {
            inherit self name pkgs;
          };
          scripts = import scriptsNix {
            inherit pkgs config;
          };
          config = import configNix {
            inherit self name pkgs system scripts packages;
          };
          stdenv = pkgs.stdenvAdapters.useWildLinker pkgs.gccStdenv;
          buildInputs =
            with pkgs;
            [
              cmake-format
              (
                let
                  version = "3.12.0";
                in
                catch2_3.overrideAttrs (prev: {
                  inherit version;
                  src = fetchFromGitHub {
                    owner = "catchorg";
                    repo = "Catch2";
                    tag = "v${version}";
                    hash = "sha256-M1n2jWiA0hNCNqO3zSXRANiaMVCebn7/VU/4FfcjoA8=";
                  };
                })
              )
            ]
            ++ lib.mapAttrsToList (name: value: scripts.scripts.${name}.pkg) scripts.scripts;
          nativeBuildInputs =
            (with pkgs; [
              gdb
              clang-tools # clangd, clang-format, clang-tidy
              jq
              oils-for-unix
            ])
            ++ scripts.packages;
        in
        {
          devShell = pkgs.mkShell {
            inputsFrom = [ packages.default ];
            inherit stdenv buildInputs nativeBuildInputs;
            shellHook = let source =
              lib.concatStringsSep "\n"
                (
                  lib.concatMap (lib.mapAttrsToList (name: value: "export ${name}=${builtins.toString value}")) config.env
                )
              + "\n"
              + lib.concatStringsSep "\n" (
                lib.mapAttrsToList
                  (file: content: ''
                    mkdir -p "$(dirname '${file}')"
                    cat > '${file}' <<'DOTFILE'
                    ${content}
                    DOTFILE
                  '')
                  config.projectDotfiles
              ); in ''
                ${source}
                cat > .env <<'DOTENV'
                ${source}
                DOTENV
              '';
          };
          inherit packages;
        }
      );
}
