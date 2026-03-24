{
  name,
  pkgs,
  scripts,
  ...
}:
let
  main_binary_name = name;
in
rec {
  transientDirectories = {
    build = ".build";
    testsBuild = ".build-tests";
    sccache = ".sccache";
    run = ".run";
  };
  env =
    # let
    #   sccacheBin = "${scripts.scripts.sccache.pkg}/bin/sccache";
    # in
    [
      {
        PROJECT_ROOT = "$(realpath .)";
      }
      {
        # C++
        CMAKE_GENERATOR = "'Ninja Multi-Config'";
        # CMAKE_C_COMPILER_LAUNCHER = "${sccacheBin}";
        # CMAKE_CXX_COMPILER_LAUNCHER = "${sccacheBin}";

        BUILD_DIR = "$PROJECT_ROOT/${transientDirectories.build}";
        TESTS_SRC_DIR = "$PROJECT_ROOT/tests";
        TESTS_BUILD_DIR = "$PROJECT_ROOT/${transientDirectories.testsBuild}";
        TESTS_DEFAULT_CONFIG = "Release";


        ASAN_SYMBOLIZER_PATH = "${pkgs.libllvm}/bin/llvm-symbolizer"; # https://clang.llvm.org/docs/AddressSanitizer.html#symbolizing-the-reports
        ASAN_OPTIONS = "check_initialization_order=1,detect_leaks=1"; # https://clang.llvm.org/docs/AddressSanitizer.html#initialization-order-checking

        TSAN_OPTIONS="enable_adaptive_delay=1,adaptive_delay_mutex_sample_rate=5,adaptive_delay_aggressiveness=150"; # https://clang.llvm.org/docs/ThreadSanitizer.html#enabling-adaptive-delay

        TYSAN_OPTIONS="print_stacktrace=1";

        GENERATE_TIMEOUT = 30;
        BUILD_TIMEOUT = 120;
        SCCACHE_CONNECTION_TIMEOUT = 3;

        EDITOR_RUN_DIR = "$PROJECT_ROOT/${transientDirectories.run}";

        # SCCACHE_DIR = "$PROJECT_ROOT/${transientDirectories.sccache}";
        # SCCACHE_CACHE_SIZE = "${sccache.maxCacheSize}";
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
        ".zed/debug.json" = builtins.toJSON [
          {
            label = "Debug main executable";
            build = {
              command = "${scripts.scripts.get-executable.pkg}/bin/get-executable";
              args = [ "${main_binary_name}" "Debug" "${transientDirectories.run}/debug" ];
            };
            program = "${transientDirectories.run}/debug";
            request = "launch";
            adapter = "CodeLLDB";
          }
        ];

        ".zed/settings.json" =
          let
            clang-tool = name: "${pkgs.clang-tools}/bin/${name}";
          in
          builtins.toJSON {
            file_scan_exclusions = [ "external" ];
            file_scan_inclusions = [ ".env*" ];
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
        ".clangd" = let naming = "readability-identifier-naming"; in ''
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
                - "*-use-anonymous-namespace"
                - "*-use-trailing-return-type"
                - "*-named-parameter"
                - "*-isolate-declaration"
                - "*-macro-usage" # old habits die hard
                - "*-easily-swappable-parameters"
                - "*-pro-type-union-access"
              CheckOptions:
                ${naming}.IgnoreNamespaces: std
                ${naming}.ClassCase: CamelCase
                ${naming}.StructCase: CamelCase
                ${naming}.UnionCase: CamelCase
                ${naming}.EnumCase: CamelCase
                ${naming}.FunctionCase: camelBack
                ${naming}.VariableCase: camelBack
                ${naming}.ParameterCase: camelBack
                ${naming}.ParameterPrefix: t_
                ${naming}.MacroDefinitionCase: UPPER_CASE
                ${naming}.EnumConstantCase: UPPER_CASE
                ${naming}.TypeTemplateParamCase: CamelCase
                ${naming}.ValueTemplateParamCase: CamelCase
                ${naming}.GlobalConstantCase: camelBack
                ${naming}.StaticConstantCase: camelBack
                ${naming}.MemberConstantCase: camelBack
                ${naming}.ConstexprVariableCase: camelBack
                ${naming}.ConstantCase: camelBack
                ${naming}.ConstexprFunctionCase: camelBack
                ${naming}.ConstexprMethodCase: camelBack
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

          ---

          If:
            PathMatch: .*\.h

          CompileFlags:
            Add: [-xc++-header]
        '';
      };
    in
    files
    // {
      # .gitignore "extension" for Nix users
      ".git/info/exclude" = ''
        .direnv
        .env

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
