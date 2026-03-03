## Build instructions
Nix is used for the whole build process, including during development & packaging.

### Development
This repository provides a development environment definition via. `flake.nix`.
It can be automatically loaded by your shell upon entering the directory by using `nix-direnv`.
It can be manually loaded by running `nix develop`.

TODO: integrate touch on scripts

### Release
This repository provides package definitions for each desktop platform via. `flake.nix`.
It also provides package definitions that output `.zip` releases, with the `-zip` suffix.
The default package for your system can be built to `result/` with `nix build`.

To get a complete list of packages, run: 
`nix eval .#packages."$(nix config show system)" --apply 'with builtins; p: concatStringsSep "\n" (attrNames p) + "\n"' --raw 2>/dev/null`
and build your choice with:
`nix build ".#pkgname"`

Most of these can then be built on a NixOS host, excepting the macOS builds which require nix-darwin.
