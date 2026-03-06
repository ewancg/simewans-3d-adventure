{
  name,
  pkgs,
  scripts,
  stdenv,
  packages,
  ...
}:
let
  mainBinaryName = name;
in
rec {
  inherit mainBinaryName;

  transientDirectories = {
    build = ".build";
    testsBuild = ".build-tests";
    sccache = ".sccache";
    run = ".run";
  };
  env =
    let
      sccacheBin = "${scripts.scripts.sccache.pkg}/bin/sccache";
    in
    [
      {
        PROJECT_ROOT = "$(realpath .)";
      }
      {
        # C++
        CMAKE_GENERATOR = "'Ninja Multi-Config'";
        CMAKE_C_COMPILER_LAUNCHER = "${sccacheBin}";
        CMAKE_CXX_COMPILER_LAUNCHER = "${sccacheBin}";

        BUILD_DIR = "$PROJECT_ROOT/${transientDirectories.build}";
        TESTS_SRC_DIR = "$PROJECT_ROOT/tests";
        TESTS_BUILD_DIR = "$PROJECT_ROOT/${transientDirectories.testsBuild}";
        TESTS_DEFAULT_CONFIG = "Release";

        ASAN_SYMBOLIZER_PATH = "${pkgs.libllvm}/bin/llvm-symbolizer"; # https://clang.llvm.org/docs/AddressSanitizer.html#symbolizing-the-reports
        ASAN_OPTIONS = "check_initialization_order=1,detect_leaks=1"; # https://clang.llvm.org/docs/AddressSanitizer.html#initialization-order-checking

        # It sometimes gets stuck waiting for sccache
        GENERATE_TIMEOUT = 30;
        BUILD_TIMEOUT = 120;
        SCCACHE_CONNECTION_TIMEOUT = 3;

        EDITOR_RUN_DIR = "$PROJECT_ROOT/${transientDirectories.run}";

        SCCACHE_DIR = "$PROJECT_ROOT/${transientDirectories.sccache}";
        SCCACHE_CACHE_SIZE = "${sccache.maxCacheSize}";
      }
    ];

  cmake = {
    buildConfigurations = [
      "Release"
      "Debug"
      "RelWithDebInfo"
      "MinSizeRel"
    ];
  };

  sccache = {
    maxCacheSize = "1G";
  };

  sausage = {
    monitor = {
      dirs = [
        "$PROJECT_ROOT/src"
        "$PROJECT_ROOT/include"
        "$PROJECT_ROOT/tests"
        "$PROJECT_ROOT/CMakeLists.txt"
      ];
      extensions = [
        "cpp"
        "h"
        "cmake"
        "txt"
      ];
    };
  };

  projectDotfiles =
    let
      files = {
        ".zed/debug.json" = let nativeTarget = "${transientDirectories.run}/debug"; in builtins.toJSON [
          {
            label = "Debug main executable (native)";
            build = let in {
              command = "${scripts.scripts.get-executable.pkg}/bin/get-executable";
              args = [ "${mainBinaryName}" "Debug" nativeTarget ];
            };
            program = nativeTarget;
            request = "launch";
            adapter = "CodeLLDB";
          }
          {
            label = "Debug main executable (Windows via. WSL2)";
            build = let in {
              command = "${scripts.scripts.get-executable-wsl.pkg}/bin/get-executable-wsl";
              args = [ "${mainBinaryName}" "Debug" nativeTarget ];
            };
            program = nativeTarget;
            request = "launch";
            adapter = "CodeLLDB";
          }

        ];
        ".zed/settings.json" =
          let
            clang-tool = name: "${pkgs.clang-tools}/bin/${name}";
          in
          builtins.toJSON {
            lsp.clangd.binary = {
              path = clang-tool "clangd";
              arguments = [
                "--compile-commands-dir=${transientDirectories.build}"
                "--query-driver=${pkgs.gcc}/bin/g++"
              ];
            };
            languages = {
              "CMake" = {
                format_on_save = "on";
                formatter.external = {
                  command = "${pkgs.cmake-format}/bin/cmake-format";
                  arguments = [
                    "-"
                  ];
                };
              };
              "C++" = {
                format_on_save = "on";
                language_servers = [ "clangd" ];
                formatter.external = {
                  command = clang-tool "clang-format";
                  arguments = [
                    "--style=file"
                    "--assume-filename={buffer_path}"
                  ];
                };
              };
            };
          };
        ".clangd" = ''
          Diagnostics:
            ClangTidy:
              Add:
                - "*"
              Remove:
                - google-*
                - llvmlibc-*
                - fuchsia-*
                - altera-*
                - abseil-*
                - boost-*
                - misc-use-anonymous-namespace
                - modernize-use-trailing-return-type
          CompileFlags:
            CompilationDatabase: ${transientDirectories.build}
            Add: [-Wno-unknown-warning-option]
            Remove: [-m*, -f*]

          ---

          If:
            PathMatch: tests/.*
          CompileFlags:
            Add: [ -DUNIT_TESTING, -Wno-unused-value, -Wno-comma ] # unused-value and comma warn about assert(("message", condition))
            CompilationDatabase: ${transientDirectories.testsBuild}
        '';
      };
    in
    files
    // {
      # .gitignore "extension" for Nix users
      ".git/info/exclude" = ''
        .direnv
        .env

        nix/local-config.nix

        # Nix build
        result

        # Editor build links
        .out/

        # Our dev shell's local transient directories
        ${builtins.concatStringsSep "\n" (builtins.attrValues transientDirectories)}

        # Our dev shell generated files
        ${builtins.concatStringsSep "\n" (builtins.attrNames files)}
      '';
    };
}
