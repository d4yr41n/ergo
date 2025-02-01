{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: let
    pkgs = import nixpkgs { system = "x86_64-linux"; };
  in {
    packages.x86_64-linux.default = pkgs.stdenv.mkDerivation {
      pname = "ergo";
      version = "0.0.3";

      src = ./.;

      nativeBuildInputs = with pkgs; [
        pkg-config
        wayland-scanner
      ];

      outputs = [ "out" ];

			makeFlags = [ "PREFIX=$(out)" ];

      buildInputs = with pkgs; [
        cairo
        pango
        wayland
        wayland-protocols
      ];
    };
  };
}

