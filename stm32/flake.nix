{
  description = "STM32 firmware build flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };

      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.gcc-arm-embedded
            pkgs.cmake
            pkgs.ninja
            pkgs.gnumake
            pkgs.openocd
          ];

          shellHook = ''
            export CC=arm-none-eabi-gcc
            export CXX=arm-none-eabi-g++
            export OBJCOPY=arm-none-eabi-objcopy
            export SIZE=arm-none-eabi-size
          '';
        };
      }
    );
}
