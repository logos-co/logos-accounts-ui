{
  description = "Logos Accounts UI - A Qt UI plugin for Logos Accounts";

  inputs = {
    # Follow the same nixpkgs as logos-liblogos to ensure compatibility
    nixpkgs.follows = "logos-liblogos/nixpkgs";
    logos-cpp-sdk.url = "github:logos-co/logos-cpp-sdk";
    logos-liblogos.url = "github:logos-co/logos-liblogos";
    logos-accounts-module.url = "github:logos-co/logos-accounts-module";
    logos-capability-module.url = "github:logos-co/logos-capability-module";
    nix-bundle-lgx.url = "github:logos-co/nix-bundle-lgx";
    logos-package-manager.url = "github:logos-co/logos-package-manager-module";
  };

  outputs = { self, nixpkgs, logos-cpp-sdk, logos-liblogos, logos-accounts-module, logos-capability-module, nix-bundle-lgx, logos-package-manager }:
    let
      systems = [ "aarch64-darwin" "x86_64-darwin" "aarch64-linux" "x86_64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
        logosSdk = logos-cpp-sdk.packages.${system}.default;
        logosLiblogos = logos-liblogos.packages.${system}.default;
        logosAccountsModule = logos-accounts-module.packages.${system}.default;
        logosAccountsModuleLib = logos-accounts-module.packages.${system}.lib;
        logosCapabilityModule = logos-capability-module.packages.${system}.default;
        lgxBundler = nix-bundle-lgx.bundlers.${system}.default;
        lgpm = logos-package-manager.packages.${system}.cli;
      });
    in
    {
      packages = forAllSystems ({ pkgs, logosSdk, logosLiblogos, logosAccountsModule, logosAccountsModuleLib, logosCapabilityModule, lgxBundler, lgpm }:
        let
          # Common configuration
          common = import ./nix/default.nix {
            inherit pkgs logosSdk logosLiblogos;
          };
          src = ./.;

          # Library package
          lib = import ./nix/lib.nix {
            inherit pkgs common src logosAccountsModule logosSdk;
          };

          logosCapabilityModuleLgx = lgxBundler logosCapabilityModule;
          logosAccountsModuleLgx = lgxBundler logosAccountsModuleLib;

          # App package
          app = import ./nix/app.nix {
            inherit pkgs common src logosLiblogos logosSdk lgpm logosCapabilityModuleLgx logosAccountsModuleLgx;
            logosAccountsUI = lib;
          };
        in
        {
          # Individual outputs
          logos-accounts-ui-lib = lib;
          app = app;
          lib = lib;

          # Default package
          default = lib;
        }
      );

      devShells = forAllSystems ({ pkgs, logosSdk, logosLiblogos, lgpm, ... }: {
        default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja
            pkgs.pkg-config
          ];
          buildInputs = [
            pkgs.qt6.qtbase
            pkgs.qt6.qtremoteobjects
            pkgs.qt6.qtdeclarative
            pkgs.zstd
            pkgs.krb5
            pkgs.abseil-cpp
          ];

          shellHook = ''
            export LOGOS_CPP_SDK_ROOT="${logosSdk}"
            export LOGOS_LIBLOGOS_ROOT="${logosLiblogos}"
            echo "Logos Accounts UI development environment"
            echo "LOGOS_CPP_SDK_ROOT: $LOGOS_CPP_SDK_ROOT"
            echo "LOGOS_LIBLOGOS_ROOT: $LOGOS_LIBLOGOS_ROOT"
          '';
        };
      });
    };
}
