{ pkgs, config, ... }:
let
  mkScripts = builtins.mapAttrs (name: body: {
    inherit name;
    pkg = pkgs.writeTextFile {
      inherit name;
      executable = true;
      destination = "/bin/${name}";
      text = ''
        #!${pkgs.oils-for-unix}/bin/osh
        ${body name}
      '';
    };
    # pkg = pkgs.writeShellScriptBin name (body name);
  });
  scripts = mkScripts {
    sccache = name: ''
      [ ! -d "$SCCACHE_DIR" ] && mkdir -p "$SCCACHE_DIR"
      timeout "''${SCCACHE_CONNECTION_TIMEOUT:-10}" "${pkgs.sccache}/bin/sccache" "$@"
      return=$?
      [ $return -eq 124 ] && exec "$@"
      exit $return
    '';
    clean = name: '' # No arguments
        git clean -xdf
      '';
    run = name: '' # $1 binary; $2 CMake config
        [ -z "$PROJECT_ROOT" ] && echo "PROJECT_ROOT not set" && exit 1
        BINARY=
        EXPLICIT_CONFIG=
        USE_DEBUGGER=
        CONFIG="Release"

        REMAINDER=()
        for i in $(seq 1 $#); do
          if [[ "''${!i}" == "--" ]]; then
            REMAINDER=("''${@:$((i+1))}")
            set -- "''${@:1:$((i-1))}"
            break
          fi
        done

        while getopts qdc: OPT; do
          case "$OPT" in
            c) # -c config
              EXPLICIT_CONFIG=true
              CONFIG="$OPTARG"
            ;;
            d) # -d toggles debugger
              USE_DEBUGGER="${pkgs.gdb}/bin/gdb"
              [ -z "''${EXPLICIT_CONFIG}" ] && CONFIG="Debug"
            ;;
            q) # -q queries the binary location
              PRINT_BINARY=true
            ;;
          esac
        done
        shift $(($OPTIND-1))

        BINARY="''${ARGS[1]}"

        BUILD_OUTPUT_PIPE=
        [ "$PRINT_BINARY" = "true" ] && BUILD_OUTPUT_PIPE="2>&1 1>/dev/null"
        # echo ${scripts.build.pkg}/bin/${scripts.build.name} "$CONFIG" $BUILD_OUTPUT_PIPE || exit $?
        eval ${scripts.build.pkg}/bin/${scripts.build.name} "$CONFIG" $BUILD_OUTPUT_PIPE || exit $?

        find "$BUILD_DIR" -type f -executable ''${BINARY:+-name "$BINARY"} -ipath "*$CONFIG*" -print | readarray -t BINARY_MATCHES

        [ ''${#BINARY_MATCHES[@]} -eq 0 ] && {
          echo "No matching binary found."
          exit 0
        }

        BINARY="''${BINARY_MATCHES[0]}"
        # exec $USE_DEBUGGER "$BINARY" ''${ARGS[@]} ''${REMAINDER[@]}
        [ "$PRINT_BINARY" = "true" ] && {
          printf "%s\n" "$BINARY"
          exit 0
        }
        $USE_DEBUGGER "$BINARY" ''${ARGS[@]} ''${REMAINDER[@]} 2> >(sed $'s/.*/\e[31m&\e[0m/' >&2)

        return=$?
        if [ $return -ge 128 ]; then
          signal="$((return - 128))"
          name="$(kill -l $signal 2>/dev/null || echo "unknown")"
          display_process="''${BINARY#"$PWD/"}"
          printf "'%s' killed with signal %s (%s)\n" "$display_process" "$signal" "$name" >&2
        fi
        exit $return
      '';
    get-executable = name: '' # $1 binary; $2 CMake config ; $3 output
      BINARY_NAME="$1" && shift
      CONFIG="$1" && shift
      OUTPUT="$1" && shift
      BINARY="$(${scripts.run.pkg}/bin/run -q -c "$CONFIG" "$BINARY_NAME")"
      [ -L "$OUTPUT" ] && rm "$OUTPUT"
      OUTPUT_DIRNAME="$(dirname "$OUTPUT")"
      [ ! -d "$OUTPUT_DIRNAME" ] && mkdir -p "$OUTPUT_DIRNAME"
      ln "$BINARY" "$OUTPUT"
    '';
    build = name: '' # $1 CMake config; $2 source dir; $3 build dir
        CONFIG="''${1:-Release}" && shift
        SOURCE_DIR="''${1:-$PROJECT_ROOT}" && shift
        BUILD_DIR="''${1:-$PROJECT_ROOT/.build}" && shift

        fast_exit() {
          kill -9 0
          exit 130
        }
        trap fast_exit INT

        run_step() {
          VARNAME=$1 && shift
          STATUS_MSG=$1 && shift
          ERROR_MSG=$1 && shift
          TIMEOUT=''${!VARNAME}

          printf "%s" "$STATUS_MSG"
          output="$(${pkgs.coreutils}/bin/timeout --signal=TERM "''${TIMEOUT:-0}" ${pkgs.expect}/bin/unbuffer "$@" 2>&1)"
          RC=$?
          [ $RC -eq 124 ] && { printf "\n%s\n" "Timed out after ''${TIMEOUT}s per \$${VARNAME}."; return 124; }
          [ $RC -ne 0 ] && { printf "\r%s\n%s\n" "$ERROR_MSG" "$output"; return $RC; }
          printf "\33[2K\r"
        }

        run_step_retry_timeout() {
          while true; do
            run_step "$@"
            RC=$?
            [ $RC -ne 124 ] && return $RC
            printf "Retrying...\n"
          done
        }

        [ "$SKIP_GENERATE" = "true" ] || {
          run_step_retry_timeout GENERATE_TIMEOUT \
            "Generating... ($CONFIG)" \
            "Generate step failed for config '$CONFIG':" \
            ${pkgs.cmake}/bin/cmake $@ -S "$SOURCE_DIR" -B "$BUILD_DIR" \
            -DCMAKE_CONFIGURATION_TYPES="${pkgs.lib.concatStringsSep ";" config.cmake.buildConfigurations}" \
            || exit $?
        }

        run_step_retry_timeout BUILD_TIMEOUT \
          "Building... ($CONFIG)" \
          "Build step failed for config '$CONFIG':" \
          ${pkgs.cmake}/bin/cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel \
          || exit $?
    '';
    run-tests = name: ''
      [ -z "$PROJECT_ROOT" ] && echo "PROJECT_ROOT not set" && exit 1

      TESTS_GENERATOR="''${CMAKE_GENERATOR:-'Ninja Multi-Config'}"

      help () {
        cat <<-'EOF'
      Usage: ${name} [OPTION]... [TEST]...
        Run unit TEST(s) and view their results
        With no TEST, all tests are run.
          -c <...>: Specify CMake configurations separated by spaces
          -A:       All CMake configurations (defaults to Release only)
          -L:       List CTest tests
          -n:       Do not generate (rebuild and run tests)
          -N:       Run tests only (do not generate or build)
          -h:       Show help
      EOF
        exit $1
      }

      build_config () {
        [ "$SKIP_BUILD" = "true" ] || {
          export PRE_GENERATE
          "${scripts.build.pkg}/bin/${scripts.build.name}" "$1" "$PROJECT_ROOT" "$TESTS_BUILD_DIR" -DBUILD_TESTS=ON || return $?
        }
      }

      require_env () {
        [ -z "''${!1}" ] && echo "\$$1 not set" && exit 1
      }

      # [OPTION]...
      SKIP_GENERATE="false"
      SKIP_BUILD="false"
      declare -a TEST_CONFIGS
      while getopts AhnNLc: OPT; do
        case "$OPT" in
          A) # -A: All configs
            TEST_CONFIGS=(${pkgs.lib.concatStringsSep " " config.cmake.buildConfigurations});;
          c) # -c <config>: explicit config
            TEST_CONFIGS+=("$OPTARG");;
          n)
            SKIP_GENERATE="true";;
          N)
            SKIP_BUILD="true";;
          L)
            require_env TESTS_DEFAULT_CONFIG
            require_env TESTS_BUILD_DIR
            build_config "$TESTS_DEFAULT_CONFIG"
            cd "$TESTS_BUILD_DIR"
            ${pkgs.cmake}/bin/ctest -N
            exit 0;;
          h)
            help 0;;
          [?])
            help 1;;
        esac
      done
      shift $(($OPTIND-1))

      # [TEST]...
      declare -a TESTS
      while (( $# )); do
        [[ "$1" == "--" ]] && { shift; REMAINDER=("$@"); break; }
        TESTS+=("$1"); shift
      done

      # $1; CMake config
      test_config () {
        CONFIG=$1
        shift
        build_config "$CONFIG" || return $?
        printf 'Results for config %s:\n' "$CONFIG"
        cd "$TESTS_BUILD_DIR"
        "${pkgs.cmake}/bin/ctest" -j0 -C "$CONFIG" --progress --output-on-failure $@
      }

      for i in ''${TEST_CONFIGS[@]:-"$TESTS_DEFAULT_CONFIG"}; do
        test_config $i ''${TESTS:+$(for i in $TESTS; do printf "%s %s " -R "$i"; done)} $REMAINDER
      done
    '';
    sausage =
      let
        wrappedCommand = "${scripts.run-tests.pkg}/bin/${scripts.run-tests.name}";
        monitorArgs = {
          dirs = pkgs.lib.concatMapStrings (dir: "-w ${dir} ") config.sausage.monitor.dirs;
          exts = pkgs.lib.concatStringsSep "," config.sausage.monitor.extensions;
        };
      in
      name: ''
        [ -z "$PROJECT_ROOT" ] && echo "PROJECT_ROOT not set" && exit 1
        declare -a ARGS
        declare -a REMAINDER
        while (( $# )); do
          [[ "$1" == "--" ]] && { shift; REMAINDER=("$@"); break; }
          [[ "$1" == "-h" ]] && {
            cat <<-'EOF'
        Usage: ${name} [-h] -- [OPTION]... [TEST]...
          Continuously monitor test results as directory contents change
          All arguments not specified are passed to ${scripts.run-tests.name}.
            -h:       Show help
        EOF
            exit 0
          }
          ARGS+=("$1")
          shift
        done

        ${pkgs.watchexec}/bin/watchexec -c reset --timings --no-meta \
        ${monitorArgs.dirs} --exts ${monitorArgs.exts} -- ${wrappedCommand} ''${ARGS[@]} ''${REMAINDER[@]}
      '';
  };
in
{
  inherit scripts; packages = with pkgs; [
  bat
  expect
  watchexec
  sccache
];
}
