{
  description = "VLog, a reasoning engine for rules";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.05";
    flake-utils.url = "github:numtide/flake-utils";
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-utils, flake-compat, ... }@inputs:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs {
        inherit system;
      };
      in
      {
        devShell = pkgs.mkShell {
          buildInputs = [
            pkgs.zlib
            pkgs.lz4
            pkgs.sparsehash
          ];
          nativeBuildInputs = [
            pkgs.gcc
            pkgs.gdb
            pkgs.cmake
            pkgs.cmakeCurses
          ];
        };
      });
}
